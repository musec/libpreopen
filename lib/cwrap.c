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

	return (map);
}

void
po_map_free(struct po_map *map)
{
	if (map == NULL) {
		return;
	}

	free(map->entries);
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
	struct po_dir *d;

	if (map->length == map->capacity) {
		map = po_map_enlarge(map);
		if (map == NULL) {
			return (NULL);
		}
	}
	
	d = map->entries + map->length;
	map->length++;

	d->dirname = path;
	d->dirfd = fd;

#ifdef WITH_CAPSICUM
	if (cap_rights_get(fd, &d->rights) != 0) {
		return (NULL);
	}
#endif

	return (map);
}

int
po_preopen(struct po_map *map, const char *path)
{
	struct stat statbuf;
	int fd;

	if((stat(path, &statbuf) != 0) || !S_ISDIR (statbuf.st_mode)) {
		return (-1);
	}

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		return (-1);
	}

	if (po_add(map, path, fd) == NULL) {
		return (-1);
	}

	return (fd);
}

struct po_relpath
po_find(struct po_map* map, const char *path, cap_rights_t *rights)
{
	struct po_relpath match;
	size_t bestlen = 0;
	int best = -1;

	for(size_t i = 0; i < map->length; i++){
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

		best = i;
		bestlen = len;
	}

	const char *relpath = path + bestlen;
	if (*relpath == '/') {
		relpath++;
	}

	match.relative_path = relpath;
	match.dirfd = map->entries[best].dirfd;

	return match;
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
