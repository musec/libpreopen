/*-
 * Copyright (c) 2016 Stanley Uche Godfrey
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

#include<stdlib.h>
#include<string.h>


// Holds opened directory fd and path
struct opened_dir_struct{
	int dirfd;
	char*dirname;
	int flags;

};

//Holds dirfd of a matched path  and path relative to that dirfd
struct matched_path{
	char * relative_path;
	int dirfd;
};

/* Contains array of  opened_dir_struct
*The capacity of the array
*The number of elements currently in the array
*/
struct Map{
	struct opened_dir_struct * opened_files;
	size_t capacity;
	size_t length;
};

struct Map* preopen(char* file,int mode);
struct Map* initializeMap(int );
struct matched_path map_path(struct Map* map, const char * path, int mode);
struct Map* getMap();
char* split_path_file(char *relative_path);
int pathCheck(char *path);
struct opened_dir_struct * open_directory(char* relative_path,struct opened_dir_struct *);
int checkCapacity();
struct Map* increaseMapCapacity();
int findMatchingChars(char *A,char *B);
int  getMostMatchedPath(int matches[]);
struct Map* add_Opened_dirpath_map(struct opened_dir_struct ods);
struct matched_path compareMatched(struct Map* map,int num,char* character,int mode);
