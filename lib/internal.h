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

#ifndef LIBPO_INTERNAL_H
#define LIBPO_INTERNAL_H

#ifdef WITH_CAPSICUM
#include <sys/capsicum.h>
#endif

#include <stdbool.h>

/**
 * A directory that has been pre-opened.
 */
struct po_dir{
	/** The path that maps to this directory */
	const char *dirname;

	/** The directory's descriptor */
	int dirfd;

#ifdef WITH_CAPSICUM
	/** Capability rights associated with the directory. */
	cap_rights_t rights;
#endif
};

/* Contains array of  opened_dir_struct
*The capacity of the array
*The number of elements currently in the array
*/
struct po_map{
	struct po_dir * opened_files;
	size_t capacity;//The size of the Map pointer
	size_t length;// Number of elements in the Map pointer
};

/**
 * Enlarge a @ref po_map's capacity.
 *
 * This results in new memory being allocated and existing entries being copied.
 * If the allocation fails, the function will return NULL but the original
 * map will remain valid.
 */
struct po_map* increaseMapCapacity();

/** Is this path a directory? */
bool po_isdir(const char *path);

/**
 * Is a directory a prefix of a given path?
 *
 * @param   dir     a directory path, e.g., `/foo/bar`
 * @param   dirlen  the length of @b dir
 * @param   path    a path that may have @b dir as a prefix,
 *                  e.g., `/foo/bar/baz`
 */
bool po_isprefix(const char *dir, size_t dirlen, const char *path);

/**
 * Open a directory and store its details in a @ref po_dir structure.
 */
struct po_dir* open_directory(const char *relative_path, struct po_dir *);

#endif /* LIBPO_INTERNAL_H */
