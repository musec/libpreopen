/*-
 * Copyright (c) 2016 Stanley Uche Godfrey
 * Copyright (c) 2016, 2018 Jonathan Anderson
 * All rights reserved.
 *
 * This software was developed at Memorial University under the
 * NSERC Discovery program (RGPIN-2015-06048).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "internal.h"
#include "libpreopen.h"


static char error_buffer[1024];

static struct po_map *global_map;

/**
 * Check that a @ref po_map is valid (assert out if it's not).
 */
#ifdef NDEBUG
#define po_map_assertvalid(...)
#else
static void
po_map_assertvalid(const struct po_map *map)
{
	const struct po_dir *dir;
	size_t i;

	assert(map->length <= map->capacity);
	assert(map->entries != NULL || map->capacity == 0);

	for (i = 0; i < map->length; i++) {
		dir = map->entries + i;

		assert(dir->dirname != NULL);
		assert(dir->dirfd >= 0);
	}
}
#endif

/**
 * Enlarge a @ref po_map's capacity.
 *
 * This results in new memory being allocated and existing entries being copied.
 * If the allocation fails, the function will return NULL but the original
 * map will remain valid.
 */
static struct po_map* po_map_enlarge(struct po_map *map);

struct po_map*
po_map_create(int capacity)
{
	struct po_map *map;

	map = malloc(sizeof(struct po_map));
	if (map == NULL) {
		return (NULL);
	}

	map->entries = calloc(sizeof(struct po_dir), capacity);
	if (map->entries == NULL) {
		free(map);
		return (NULL);
	}

	map->capacity = capacity;
	map->length = 0;

	po_map_assertvalid(map);

	return (map);
}

void
po_map_free(struct po_map *map)
{
	//free strdup call
	if (map == NULL) {
		return;
	}

	po_map_assertvalid(map);

	free(map->entries);
	free(map);
}

struct po_map*
po_map_get()
{
	if (global_map == NULL) {
		global_map = po_map_create(4);
	}

	po_map_assertvalid(global_map);

	return (global_map);
}

void
po_map_set(struct po_map *map)
{
	po_map_assertvalid(map);

	if (global_map != NULL) {
		po_map_free(global_map);
	}

	global_map = map;
}

struct po_map*
po_add(struct po_map *map, const char *path, int fd)
{
	struct po_dir *d;

	po_map_assertvalid(map);

	if (map->length == map->capacity) {
		map = po_map_enlarge(map);
		if (map == NULL) {
			return (NULL);
		}
	}

	d = map->entries + map->length*sizeof(struct po_dir);
	map->length++;

	d->dirname = strdup(path); 
	d->dirfd = fd;

#ifdef WITH_CAPSICUM
	if (cap_rights_get(fd, &d->rights) != 0) {
		return (NULL);
	}
#endif

	po_map_assertvalid(map);

	return (map);
}

int
po_preopen(struct po_map *map, const char *path)
{
	int fd=0,  is_reg_path;
	struct stat statbuff;

	po_map_assertvalid(map);

	fstatat(AT_FDCWD,path,&statbuff,AT_SYMLINK_NOFOLLOW);
	is_reg_path = S_ISREG(statbuff.st_mode);
	if(is_reg_path == 0) {
		fd = openat(AT_FDCWD, path, O_DIRECTORY);
		if (fd == -1) {
			return (-1);
		}
	} else if(is_reg_path != 0) {
		char* temp_path = po_split_file_fromPath(path);
		fd = openat(AT_FDCWD, temp_path, O_DIRECTORY);
		if (fd == -1) {
			return (-1);
		}
	} else{
		po_errormessage("Specify path type\n");
	}

	if (po_add(map, path, fd) == NULL) {
		return (-1);
	}

	po_map_assertvalid(map);

	return (fd);
}

char*
po_split_file_fromPath(const char *relative_path)
{
	const char slash='/';
	char *filename;
	char *dirName;

	filename = strrchr(relative_path, slash);
	dirName = strndup(relative_path,
			strlen(relative_path) - strlen(filename) + 1);

	return dirName;
}

struct po_relpath
po_find(struct po_map* map, const char *path, cap_rights_t *rights)
{
	const char *relpath ;
	struct po_relpath match;
	size_t bestlen = 0;
	int best = -1;

	po_map_assertvalid(map);

	for(size_t i = 0; i < map->length; i++) {
		const struct po_dir *d = map->entries + i;
		const char *dirname = d->dirname;
		size_t len = strnlen(dirname, MAXPATHLEN);

		if ((len <= bestlen) || !po_isprefix(dirname, len, path)) {
			continue;
		}

#ifdef WITH_CAPSICUM
		if (rights && !cap_rights_contains(&d->rights, rights)) {
			continue;
		}
#endif

		best = d->dirfd;
		bestlen = len;
	}

	relpath = path + strlen(po_split_file_fromPath(path));

	if (*relpath == '/') {
		relpath++;
	}

	match.relative_path = relpath;
	match.dirfd = best;

	return match;
}

void
po_errormessage(const char *msg)
{

	snprintf(error_buffer, sizeof(error_buffer), "%s: error %d",
		msg, errno);
}

const char*
po_last_error()
{

	return (error_buffer);
}

int
po_pack(struct po_map *map)
{
	char* trailerstring;
	struct po_packed_map* data_array;
	int fd, i, r, trailer_len, offset;

	po_map_assertvalid(map);

	trailer_len=0;
	for(i=0;i<map->length;i++) {
		trailer_len+=strlen(map->entries[i].dirname);
	}

	const size_t shardmemory_blocksize=sizeof(struct po_packed_map)
		+(map->length)*sizeof(struct po_packed_entry)+(trailer_len)*sizeof(char);

	fd = shm_open(SHM_ANON, O_CREAT |O_RDWR, 0666);
	if (fd == -1){
		po_errormessage("shm_open");
		return (-1);
	}

	r = ftruncate(fd,shardmemory_blocksize);
	void *ptr = mmap(0,shardmemory_blocksize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED) {
		po_errormessage("shm_open");
		return (-1);
	} else{
		data_array=(struct po_packed_map*)ptr;
	}

	data_array->entries=(struct po_packed_entry*)data_array+sizeof(struct po_packed_map);
	trailerstring =(char*)data_array->entries+(map->length)*sizeof(struct po_packed_entry);
	assert(trailerstring !=NULL);
	for(i=0;i<map->length;i++){
		strcat(trailerstring,map->entries[i].dirname);
	}

	data_array->count=map->length;
	data_array->trailer_len=trailer_len;
	offset=0;
	for(i=0;i<map->length;i++){
		data_array->entries[i].offset=offset;
		data_array->entries[i].len=strlen(map->entries[i].dirname);
		data_array->entries[i].fd=map->entries[i].dirfd;
		offset+=data_array->entries[i].len;
	}

	return fd;
}


struct po_map*
po_unpack(int fd)
{
	struct po_map *map;
	struct stat fdStat;
	struct po_packed_map* data_array=NULL;
	char *trailerstring, *tempstr;
	int i;

	if(fstat(fd,&fdStat) < 0) {
		po_errormessage("fdStat");
		return (NULL);
	}

	void *ptr = mmap(0,fdStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED) {
		po_errormessage("mmap");
		return (NULL);
	} else {
		data_array=(struct po_packed_map*)ptr;
	}

	data_array->entries=(struct po_packed_entry*)data_array+sizeof(struct po_packed_map);
	trailerstring =(char*)data_array->entries+data_array->count*sizeof(struct po_packed_entry);
	assert(data_array->entries !=NULL);
	assert(trailerstring !=NULL);

	map = malloc(sizeof(struct po_map));
	if (map == NULL) {
		return (NULL);
	}

	map->entries = calloc(sizeof(struct po_dir),data_array->count);
	if (map->entries == NULL) {
		free(map);
		return (NULL);
	}

	map->length =data_array->count;
	for(i=0; i < map->length; i++) {
		map->entries[i].dirfd=data_array->entries[i].fd;
		tempstr=trailerstring+data_array->entries[i].offset;
		map->entries[i].dirname=strndup(tempstr,data_array->entries[i].len);
	}

	po_map_assertvalid(map);

	return map;
}


int
po_map_length(struct po_map* map)
{
	po_map_assertvalid(map);

	return (int)map->length;
}

const char*
po_map_name(struct po_map *map, int k)
{
	po_map_assertvalid(map);

	return map->entries[k].dirname;
}

int
po_map_fd(struct po_map *map,int k)
{
	po_map_assertvalid(map);

	return map->entries[k].dirfd;
}

/* Internal (service) functions: */

struct po_map*
po_map_enlarge(struct po_map *map)
{
	struct po_dir *enlarged;
	enlarged = calloc(sizeof(struct po_dir), 2 * map->capacity);
	if (enlarged == NULL) {
		return (NULL);
	}
	memcpy(enlarged, map->entries, map->length * sizeof(*enlarged));
	free(map->entries);
	map->entries = enlarged;
	map->capacity = 2 * map->capacity;
	return map;
}

bool
po_isprefix(const char *dir, size_t dirlen, const char *path)
{
	size_t i;
	assert(dir != NULL);
	assert(path != NULL);
	for (i = 0; i < dirlen; i++)
	{
		if (path[i] != dir[i])
			return false;
	}
	return path[i] == '/' || path[i] == '\0';
}
