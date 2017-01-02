/*-
 * Copyright (c) 2016 Stanley Uche Godfrey
 * Copyright (c) 2016 Jonathan Anderson
 * All rights reserved.
 *
 * This software was developed at Memorial University under the
 * NSERC Discovery program (RGPIN-2015-06048).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
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

#include <sys/param.h>
#include <sys/stat.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libpo.h"
#include "internal.h"


static struct po_map *global_map;

struct po_map*
po_map_create(int capacity)
{
	struct po_map *map;
	
	map = malloc(sizeof(struct po_map));
	if (map == NULL) {
		return (NULL);
	}

	map->opened_files = calloc(sizeof(struct po_dir), capacity);
	if (map->opened_files == NULL) {
		free(map);
		return (NULL);
	}

	map->capacity = capacity;
	map->length = 0;

	return (map);
}

void
po_map_free(struct po_map *map)
{
	if (map == NULL) {
		return;
	}

	free(map->opened_files);
	free(map);
}

struct po_map*
po_map_get()
{
	if (global_map == NULL) {
		global_map = po_map_create(4);
	}
	return (global_map);
}

void
po_map_set(struct po_map *map)
{
	if (global_map != NULL) {
		po_map_free(global_map);
	}

	global_map = map;
}

struct po_map*
po_add(struct po_map *map, const char *path, int fd)
{
	struct po_dir *d = map->opened_files + map->length;
	map->length++;

	d->dirname = path;
	d->dirfd = fd;

	return (map);
}

//split file from path
char* split_path_file(char *relative_path,int length) {
	const char slash='/';
   	char *filename;
	char *dirName;
	filename= strrchr(relative_path, slash);
	dirName=strndup(relative_path,length- strlen(filename));
	return dirName;
}
// increases the capacity of map by allocating more memory
struct po_map* increaseMapCapacity(struct po_map *map) {
	int i;struct po_dir *new_opened_files;
	new_opened_files=(struct po_dir*)malloc((2*map->capacity)*sizeof(struct po_dir));
	assert(new_opened_files!=NULL);
	map->capacity=2*map->capacity;

		for(i=0;i<map->length;i++){
			new_opened_files[i]=map->opened_files[i];

		}
		free(map->opened_files);
		map->opened_files=new_opened_files;
		return map;

}

bool
po_isdir(char *path)
{
	struct stat statbuf;

	if(stat(path,&statbuf)!=0){
		return 0;
	}
			
	return (S_ISDIR (statbuf.st_mode));
}
/* Opens a directory and store both the directoryfd and
   the directory path in a structure
*/

struct po_dir * open_directory(char* file_path,struct po_dir *dos){
	int dir_fd,k; char * dirname;
	DIR *dir;
	k=po_isdir(file_path);

	if(k==0){
		return NULL;
	}
	else{
		dirname=file_path;
	}
	dir=opendir(dirname);

	if(dir !=NULL){
		dir_fd=dirfd(dir);
		dos->dirfd=dir_fd;
		dos->dirname=dirname;

	}
	else{
		return NULL;
	}

	return dos;

}

//Opens a file path
struct po_map* po_preopen(struct po_map *map, char* file,int mode){
	int k;
	struct po_dir ods;
	struct po_dir * odsp;
	if(map->length!=0){
		k=map->capacity-map->length;
		if(k<2){
				map=increaseMapCapacity(map);
			}
	}



	odsp=open_directory( file,&ods);
	map = po_add(map, ods.dirname, ods.dirfd);
	return map;
}


bool po_isprefix(const char *dir, size_t dirlen, const char *path)
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

struct po_relpath po_find(struct po_map* map, const char *path){
	struct po_relpath match;
	size_t bestlen = 0;
	int best = -1;

	for(size_t i = 0; i < map->length; i++){
		const char *dirname = map->opened_files[i].dirname;
		size_t len = strnlen(dirname, MAXPATHLEN);

		if (po_isprefix(dirname, len, path) && len > bestlen) {
			best = i;
			bestlen = len;
		}
	}

	match.relative_path = path + bestlen;
	match.dirfd = map->opened_files[best].dirfd;

	return match;
}
