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
struct po_dir{
	int dirfd;
	char*dirname;
	int flags;

};

//Holds dirfd of a matched path  and path relative to that dirfd
struct po_matched_path{
	char * relative_path;
	int dirfd;
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

//Opens a file path
struct po_map* po_preopen(struct po_map*, char* file,int mode);
struct po_map* po_map_create(int );
struct po_matched_path po_find(struct po_map* map, const char * path, int mode);
// returns pointer to the Map structure
struct po_map* getMap();
char* split_path_file(char *relative_path,int length);

//check if path is  a file or a directory
int po_isdir(char *path);

/* Opens a directory and store both the directoryfd and
   the directory path in the opened_dir_struct structure
*/

struct po_dir * open_directory(char* relative_path,struct po_dir *);


// increases the capacity of map by allocating more memory
struct po_map* increaseMapCapacity();

/*
  *Finds how many characters in a string is in another
  *string begining from the first character
  *@param A a directory path in the array containing opened directories
  *@param B a path to be opened
  *Makes sure that none of the parameters are NULL
*/
int findMatchingChars(char *A,char *B);

/*
 Returns the dirfd of the opened path with highest matched char number to the path to be opened
* or zero if no match is found
*/
int  getMostMatchedPath(int matches[],int arraylength,struct po_map *map);

//add an opened path to the pointer to opened_dir_struct field of the Map struct
struct po_map* add_Opened_dirpath_map(struct po_map *, struct po_dir ods);

/*
 * Uses other function to return the matched path if any or opened the pathed to be matched 
*/

struct po_matched_path compareMatched(struct po_map* map,int num,char* character,int mode);
