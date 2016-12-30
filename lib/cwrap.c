
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


#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include"cwrapHeader.h"
#include <limits.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<assert.h>


static struct po_map* map;

struct po_map *initializeMap(int capacity){
	
	struct po_map *map=(struct po_map*)malloc(sizeof(struct po_map));

	map->opened_files=(struct po_opened_dir_struct*)malloc(capacity*sizeof(struct po_opened_dir_struct));
	assert(map->opened_files!=NULL);
	map->capacity=capacity;
	map->length=0;
	return map;

}

struct po_map* getMap(){
	if(map==0){
		map=initializeMap(4);
	}
	return map;
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
struct po_map* increaseMapCapacity(){
	int i;struct po_opened_dir_struct *new_opened_files;
	new_opened_files=(struct po_opened_dir_struct*)malloc((2*map->capacity)*sizeof(struct po_opened_dir_struct));
	assert(new_opened_files!=NULL);
	map->capacity=2*map->capacity;

		for(i=0;i<map->length;i++){
			new_opened_files[i]=map->opened_files[i];

		}
		free(map->opened_files);
		map->opened_files=new_opened_files;
		return map;

}

int po_isdir(char *path){
	struct stat statbuf;

	if(stat(path,&statbuf)!=0){
		return 0;
	}
			
	return (S_ISDIR (statbuf.st_mode));
}
/* Opens a directory and store both the directoryfd and
   the directory path in a structure
*/

struct po_opened_dir_struct * open_directory(char* file_path,struct po_opened_dir_struct *dos){
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

//add an opened path the pointer to opened_dir_struct field of the Map struct
struct po_map* add_Opened_dirpath_map(struct po_opened_dir_struct ods){
	map->opened_files[map->length]=ods;
	map->length++;
	return map;
}
//Opens a file path
struct po_map* preopen(char* file,int mode){
	int k;
	struct po_opened_dir_struct ods;
	struct po_opened_dir_struct * odsp;
	if(map->length!=0){
		k=map->capacity-map->length;
		if(k<2){
				map=increaseMapCapacity();
			}
	}



	odsp=open_directory( file,&ods);
	map=add_Opened_dirpath_map(*odsp);
	return map;
}

/*
  Finds how many characters in a string is in another
  string begining from the first character
*/
int findMatchingChars(char *A,char *B){
	//argu *A is path to be matched
	//argu *B is a preopened dir path in the map pointer
	int lenA,lenB,matchedNum=0,i;
	assert(A!=NULL && B!=NULL);
	lenA=strlen(A);
	lenB=strlen(B);
	for(i=0;i<lenB;i++){
		if(A[i]==B[i]){
			matchedNum++;
		}
		else{
			break;
		}
	}
	return matchedNum;
}

/*
 Returns the dirfd of the opened path with highest matched number
 and relative path to the dirfd
*/
int  getMostMatchedPath(int matches[],int length,struct po_map *map){
	int highestnumber=0,i;
	length=map->length;
	for(i=0;i<length;i++){
		if(matches[i]>highestnumber){
			highestnumber=matches[i];

		}

	}

	return highestnumber;

}
/*compares matched path and see if the matched path is already opened
 * if not it opens the matched path else it returns the matched path dirfd
 * and relative path.
 */
struct po_matched_path compareMatched(struct po_map* map,int best_matched_num,char *newPath,int mode){
	char * temp_dir,*t_dir;
	int i,status;
	struct po_matched_path  matchedPath ={0};
	if(best_matched_num==0){
		map=preopen(newPath,mode);
		matchedPath.dirfd=map->opened_files[map->length-1].dirfd;
		matchedPath.relative_path=newPath+strlen(map->opened_files[map->length-1].dirname);
	}
	else{

		t_dir=newPath+best_matched_num;
		temp_dir=strndup(newPath,strlen(newPath)- strlen(t_dir));

		for(i=0;i<map->length;i++){
			status=strcmp(temp_dir,map->opened_files[i].dirname);
			if(status==0){
				matchedPath.dirfd=map->opened_files[i].dirfd;
				matchedPath.relative_path=t_dir;
				break;
			}
		}
		if(status !=0){
			map=preopen(newPath,mode);

		}

	}
	return matchedPath;
}
/*
 * Uses other function to return matched path
*/

struct po_matched_path map_path(struct po_map* map,const char* a_filepath,int mode){
	int i, length=map->length; char * filename;
	int best_matched_num;
	struct po_matched_path matchedPath={0};
	int matched_num[length];
	filename=(char*)a_filepath;
	if(length==0){
				map=preopen(filename,mode);
			}
	else{
		for(i=0;i<length;i++){

				matched_num[i]=findMatchingChars(filename,map->opened_files[i].dirname);
			}
		best_matched_num=getMostMatchedPath(matched_num,length,map);
		matchedPath=compareMatched(map,best_matched_num,filename, mode);
	}



	return matchedPath;
}

