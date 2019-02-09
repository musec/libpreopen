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

/**
 * @file  libpreopen.c
 * Implementation of high-level libpreopen functions.
 *
 * The functions defined in this source file are the highest-level API calls
 * that client code will mostly use (plus po_map_create and po_map_release).
 * po_isprefix is also defined here because it doesn't fit anywhere else.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "internal.h"


struct po_map*
po_add(struct po_map *map, const char *path, int fd)
{
	struct po_map_entry *entry;

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

	entry = map->entries + map->length;
	map->length++;

	entry->name = strdup(path);
	entry->fd = fd;

#ifdef WITH_CAPSICUM
	if (cap_rights_get(fd, &entry->rights) != 0) {
		return (NULL);
	}
#endif

	po_map_assertvalid(map);

	return (map);
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
		const struct po_map_entry *entry = map->entries + i;
		const char *name = entry->name;
		size_t len = strnlen(name, MAXPATHLEN);

		if ((len <= bestlen) || !po_isprefix(name, len, path)) {
			continue;
		}

#ifdef WITH_CAPSICUM
		if (rights && !cap_rights_contains(&entry->rights, rights)) {
			continue;
		}
#endif

		best = entry->fd;
		bestlen = len;
	}

	relpath = path + bestlen;

	while (*relpath == '/') {
		relpath++;
	}

	if (*relpath == '\0') {
		relpath = ".";
	}

	match.relative_path = relpath;
	match.dirfd = best;

	return match;
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

bool
po_print_entry(const char *name, int fd, cap_rights_t rights)
{
	printf(" - name: '%s', fd: %d, rights: <rights>\n",
	       name, fd);
	return (true);
}
