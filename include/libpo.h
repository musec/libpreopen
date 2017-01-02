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

#ifndef LIBPO_H
#define LIBPO_H

/**
 * A mapping from paths to pre-opened directories.
 *
 * This type is opaque to clients, but can be thought of as containing
 * a set (with no particular ordering guarantees) of path->dirfd mappings.
 */
struct po_map;


/**
 * A filesystem path, relative to a directory descriptor.
 */
struct po_relpath {
	/** The directory the path is relative to */
	int dirfd;

	/** The path, relative to the directory represented by @ref dirfd */
	const char *relative_path;
};


//Opens a file path
struct po_map* po_preopen(struct po_map*, char* file,int mode);
struct po_map* po_map_create(int );
struct po_relpath po_find(struct po_map *map, const char *path);
// returns pointer to the Map structure
struct po_map* getMap();
char* split_path_file(char *relative_path,int length);

/*
 * Uses other function to return the matched path if any or opened the pathed to be matched 
*/

struct po_relpath compareMatched(struct po_map* map,int num,char* character,int mode);

#endif /* !LIBPO_H */
