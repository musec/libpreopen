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
 * @file  po_pack.c
 * @brief Code for [in]packing po_map into/from dense shared memory segments
 */

#include <sys/mman.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "internal.h"

/**
 * An entry in a po_packed_map.
 *
 * @internal
 */
struct po_packed_entry {
	/** Integer file descriptor */
	int fd;

	/** Offset of the entry's name within the po_packed_map's string table */
	int offset;

	/** Length of the entry's name (not including any null terminator) */
	int len;
};

/**
 * Packed-in-a-buffer representation of a po_map.
 *
 * An object of this type will be immediately followed in memory by a string
 * table of length `tablelen`.
 *
 * @internal
 */
struct po_packed_map {
	/** The number of po_packed_entry values in the packed map */
	int count;

	/**
	 * Length of the name string table that follows this po_packed_map in the
	 * shared memory segment.
	 */
	int tablelen;

	/** The actual packed entries */
	struct po_packed_entry entries[0];
};

int
po_pack(struct po_map *map)
{
	struct po_packed_entry *entry;
	struct po_packed_map *packed;
	char *strtab;
	size_t chars;   /* total characters to be copied into string table */
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
		chars += strlen(map->entries[i].name) + 1;
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
	packed->tablelen = chars;
	strtab = ((char*) packed) + size - chars;
	offset = 0;

	for(i=0; i < map->length; i++){
		entry = packed->entries + i;

		entry->fd = map->entries[i].fd;
		entry->offset = offset;
		entry->len = strlen(map->entries[i].name);
		strlcpy(strtab + offset, map->entries[i].name,
			chars - offset);

		offset += entry->len;
	}

	return fd;
}

struct po_map*
po_unpack(int fd)
{
	struct stat sb;
	struct po_map_entry *entry;
	struct po_map *map;
	struct po_packed_map *packed;
	char *strtab;
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

	strtab = ((char*) packed->entries)
		+ packed->count * sizeof(struct po_packed_entry);
	assert(strtab - ((char*) packed) <= sb.st_size);

	map = malloc(sizeof(struct po_map));
	if (map == NULL) {
		munmap(packed, sb.st_size);
		return (NULL);
	}

	map->entries = calloc(packed->count, sizeof(struct po_map_entry));
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

		entry->fd = packed->entries[i].fd;
		entry->name = strndup(strtab + packed->entries[i].offset,
			packed->entries[i].len);
	}

	po_map_assertvalid(map);

	return map;
}
