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
#include <stdarg.h>
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

	assert(map->refcount > 0);
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

bool
po_dir_print(const char *dirname, int dirfd, cap_rights_t rights)
{
	printf(" - dirname: '%s', dirfd: %d, rights: <rights>\n",
	       dirname, dirfd);
	return (true);
}

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

	map->refcount = 1;
	map->capacity = capacity;
	map->length = 0;

	po_map_assertvalid(map);

	return (map);
}

void
po_map_release(struct po_map *map)
{
	if (map == NULL) {
		return;
	}

	po_map_assertvalid(map);

	map->refcount -= 1;

	if (map->refcount == 0) {
		free(map->entries);
		free(map);
	}
}

size_t
po_map_foreach(const struct po_map *map, po_dir_callback cb)
{
	struct po_dir *dir;
	size_t n;

	po_map_assertvalid(map);

	for (n = 0; n < map->length; n++) {
		dir = map->entries + n;

		if (!cb(dir->dirname, dir->dirfd, dir->rights)) {
			break;
		}
	}

	return (n);
}

struct po_map*
po_map_get()
{
	if (global_map == NULL) {
		global_map = po_map_create(4);
	}

	if (global_map != NULL) {
		po_map_assertvalid(global_map);
	}

	global_map->refcount += 1;

	return (global_map);
}

void
po_map_set(struct po_map *map)
{
	po_map_assertvalid(map);

	map->refcount += 1;

	if (global_map != NULL) {
		po_map_release(global_map);
	}

	global_map = map;
}

struct po_map*
po_add(struct po_map *map, const char *path, int fd)
{
	struct po_dir *d;

	po_map_assertvalid(map);

	if (path == NULL || fd < 0) {
		return (NULL);
	}

	if (map->length == map->capacity) {
		map = po_map_enlarge(map);
		if (map == NULL) {
			return (NULL);
		}
	}

	d = map->entries + map->length;
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
po_preopen(struct po_map *map, const char *path, int flags, ...)
{
	va_list args;
	int fd, mode;

	va_start(args, flags);
	mode = va_arg(args, int);

	po_map_assertvalid(map);

	if (path == NULL) {
		return (-1);
	}

	fd = openat(AT_FDCWD, path, flags, mode);
	if (fd == -1) {
		return (-1);
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

	if (relative_path == NULL) {
		return (NULL);
	}

	filename = strrchr(relative_path, slash);
	dirName = strndup(relative_path,
			strlen(relative_path) - strlen(filename) + 1);

	return dirName;
}

struct po_relpath
po_find(struct po_map* map, const char *path, cap_rights_t *rights)
{
	const char *relpath ;
	struct po_relpath match = { .relative_path = NULL, .dirfd = -1 };
	size_t bestlen = 0;
	int best = -1;

	po_map_assertvalid(map);

	if (path == NULL) {
		return (match);
	}

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

	relpath = path + bestlen;

	if (*relpath == '/') {
		relpath++;
	}

	if (*relpath == '\0') {
		relpath = ".";
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
	struct po_packed_entry *entry;
	struct po_packed_map *packed;
	char *trailer;
	size_t chars;   /* total characters to be copied into trailer data */
	size_t size;
	int fd, i, offset;

	po_map_assertvalid(map);

	fd = shm_open(SHM_ANON, O_CREAT | O_RDWR, 0600);
	if (fd == -1){
		po_errormessage("failed to shm_open SHM for packed map");
		return (-1);
	}

	chars = 0;
	for(i = 0; i < map->length; i++) {
		chars += strlen(map->entries[i].dirname) + 1;
	}

	size = sizeof(struct po_packed_map)
		+ map->length * sizeof(struct po_packed_entry)
		+ chars;

	if (ftruncate(fd, size) != 0) {
		po_errormessage("failed to truncate shared memory segment");
		close(fd);
		return (-1);
	}

	packed = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (packed == MAP_FAILED) {
		po_errormessage("shm_open");
		close(fd);
		return (-1);
	}

	packed->count = map->length;
	packed->trailer_len = chars;
	trailer = ((char*) packed) + size - chars;
	offset = 0;

	for(i=0; i < map->length; i++){
		entry = packed->entries + i;

		entry->fd = map->entries[i].dirfd;
		entry->offset = offset;
		entry->len = strlen(map->entries[i].dirname);
		strlcpy(trailer + offset, map->entries[i].dirname,
			chars - offset);

		offset += entry->len;
	}

	return fd;
}


struct po_map*
po_unpack(int fd)
{
	struct stat sb;
	struct po_dir *entry;
	struct po_map *map;
	struct po_packed_map *packed;
	char *trailer;
	int i;

	if(fstat(fd, &sb) < 0) {
		po_errormessage("failed to fstat() shared memory segment");
		return (NULL);
	}

	packed = mmap(0, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (packed == MAP_FAILED) {
		po_errormessage("mmap");
		return (NULL);
	}

	trailer = ((char*) packed->entries)
		+ packed->count * sizeof(struct po_packed_entry);
	assert(trailer - ((char*) packed) < sb.st_size);

	map = malloc(sizeof(struct po_map));
	if (map == NULL) {
		munmap(packed, sb.st_size);
		return (NULL);
	}

	map->entries = calloc(packed->count, sizeof(struct po_dir));
	if (map->entries == NULL) {
		munmap(packed, sb.st_size);
		free(map);
		return (NULL);
	}

	map->refcount = 1;
	map->capacity = packed->count;
	map->length = packed->count;
	for(i = 0; i < map->length; i++) {
		entry = map->entries + i;

		entry->dirfd = packed->entries[i].fd;
		entry->dirname = strndup(trailer + packed->entries[i].offset,
			packed->entries[i].len);
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

	if (k >= map->length) {
		return (NULL);
	}

	return map->entries[k].dirname;
}

int
po_map_fd(struct po_map *map,int k)
{
	po_map_assertvalid(map);

	if (k >= map->length) {
		return (-1);
	}

	return map->entries[k].dirfd;
}

/* Internal (service) functions: */

static struct po_map*
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
