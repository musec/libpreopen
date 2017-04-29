
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

#ifdef WITH_CAPSICUM
#include <sys/capsicum.h>
#else
#define cap_rights_t void
#endif

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
/**
*The structure holds the offset and the length of a string element
*of the trailer string pointer
*/
struct po_offset{
	int offset;//offset of the string element in trailer string  
	int len;// length of the string element  in the trailer string
	int fd;// the directory the path is relative to
};
/**
*The structure holds counter to the trailer string,
*The length of total trailer string,
*An po_offset type with a pointer to the trailer string
*/
struct po_shmstruct{
	int count;//counter to the relative paths in trailer string  
	int trailer_len;// length of the  trailer string
	int capacity; // the capacity of  *data
	struct po_offset *data; //pointer to po_offset struct and a reserved memory for the trailer string
};

/**
 * Create a @ref po_map of at least the specified capacity.
 */
struct po_map* po_map_create(int capacity);

/**
 * Free a @ref po_map and all of its owned memory.
 */
void po_map_free(struct po_map *);

/**
 * Retrieve (and possibly create) the default map.
 *
 * This can fail if there is no existing map and memory allocation fails.
 */
struct po_map* po_map_get(void);

/**
 * Set the default map, taking ownership of its memory allocation(s).
 *
 * If there is an existing default map, it will be freed before it is replaced.
 * It is permissible to pass in a NULL map in order to clear the current
 * default map.
 */
void po_map_set(struct po_map*);

/**
 * Add an already-opened directory to a @ref po_map.
 *
 * @param   map     the map to add the path->fd mapping to
 * @param   path    the path that will map to this directory
 *                  (which may or may not be the path used to open it)
 * @param   fd      the directory descriptor (must be a directory!)
 */
struct po_map* po_add(struct po_map *map, const char *path, int fd);

/**
 * Pre-open a path and store it in a @ref po_map for later use.
 *
 * @returns the file descriptor of the opened directory or -1 if
 *          @b path is not a directory or cannot be opened or if
 *          the @ref po_map cannot store the directory (e.g., resizing fails)
 */
int po_preopen(struct po_map *, const char *path);

/**
 * Find a directory whose path is a prefix of @b path and (on platforms that
 * support Capsicum) that has the rights required by @b rights.
 *
 * @param   map     the map to look for a directory in
 * @param   path    the path we want to find a pre-opened prefix for
 * @param   rights  if non-NULL on a platform with Capsicum support,
 *                  the rights any directory descriptor must have to
 *                  qualify as a match
 *
 * @returns a @ref po_relpath containing the descriptor of the best-match
 *          directory in the map (or -1 if none was found) and the remaining
 *          path, relative to the file (or undefined if no match found)
 */
struct po_relpath po_find(struct po_map *map, const char *path,
	cap_rights_t *rights);
/**
*Prints and error message when an error occurs
*@param  msg the error message to be printed alongside system error message
*/
void po_errormessage(const char *msg);
/**
*creates a shared memory block which points to po_shmstruct 
*returns a fd of the shared memory created.
*@param map  the map to map into the shared memory block
*/
int po_create_shmdata(struct po_map *map);
/**
*create a po_map* from the fd returned by the po_create_shmdata function
*by accessing the shared memory block created by po_create_shmdata function
*@param fd     the file descriptor returned by po_create_shmdata function
*/
struct po_map* po_reverse_create_shmdata(int fd);
/**
*Returns the number of elements in the pointer to the map struct
*@param map the map struct pointer which its lenght will be returned
*/
int  get_map_length(struct po_map *map);
/**
*Returns the directory name at index  in the pointer to the map struct
*@param map the map struct pointer which contains the directory name to be returned
*@param k   index at which to look for the directory name to be returned
*/
char *  get_map_dirname(struct po_map *map,int k);
#endif /* !LIBPO_H */
