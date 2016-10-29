/*
 * cwrap.c
 *
 *  Created on: Oct 17, 2016
 *      Author: ujay
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


static Map* map;
//split file from path
char* split_path_file(char *relative_path) {
    const char slash='/';
   	char *filename;
	char *dirName;
	filename= strrchr(relative_path, slash);
	dirName=strndup(relative_path,strlen(relative_path)- strlen(filename));
	return dirName;
}
//check the capacity of map
int checkCapacity(Map*map){
	if(map->length<=map->capacity)
		return 0;
	else
		return -1;
}
// increases the capacity of map
Map* increaseMapCapacity(Map *map){
	int i;
	Map *new_map=(Map*)malloc((2*map->capacity)*sizeof(Map));
	assert(new_map!=NULL);
	new_map->opened_files=(opened_dir_struct*)malloc((2*map->capacity)*sizeof(new_map->opened_files));
	assert(new_map->opened_files!=NULL);
	new_map->capacity=2*map->capacity;
	new_map->length=0;

		for(i=0;i<map->length;i++){
			new_map[i]=map[i];
			new_map->length++;
		}
		free(map->opened_files);
		free(map);
		return new_map;

}
//check if file path is a file or a directory
int pathCheck(char *path){
	int i;
	struct stat statbuf;

		if(stat(path,&statbuf)==0){
			if (S_ISREG (statbuf.st_mode)){
				i=0;
			}
			 if (S_ISDIR (statbuf.st_mode)) {
			        i=-1;
			    }

		}
		return i;
}
/* Opens a directory and store both the directoryfd and
   the directory path in a structure
*/

void open_directory(char* file_path,opened_dir_struct *ods){
	int dir_fd,k; char * dirname;
	DIR *dir;
	k=pathCheck(file_path);

	if(k==0){
		dirname=split_path_file(file_path);
	}
	else{
		dirname=file_path;
	}
	dir=opendir(dirname);

	if(dir !=NULL){
		ods->dirfd=dirfd(dir);
		ods->dirname=dirname;
		ods->flags=0;

	}
	else
     		printf("Error opening %s \n ",ods->dirname);

}

Map *initializeMap(int capacity){
	int i;
	Map *map=(Map*)malloc(capacity*sizeof(Map));

	map->opened_files=(opened_dir_struct*)malloc(capacity*sizeof(opened_dir_struct));
	assert(map->opened_files!=NULL);
	map->capacity=capacity;
	map->length=0;
	return map;

}
Map* getMap(){
	if(map==0){
		map=initializeMap(4);
	}
	return map;
}
Map* preopen(char* file,int mode,int flag){
	int i,j,k;
	char* dirname;
	k=checkCapacity(map);
	if(k<0){
		map=increaseMapCapacity(map);
	}

	open_directory( file,map[map->length].opened_files);

	map->length++;

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
matched_path  getMostMatchedPath(int matches[],char *newPath,int mode, int flag){
	int highestnumber=0,i,highest_num_index;

	matched_path  matchedPath;
	for(i=0;i<map->length;i++){
		if(matches[i]>highestnumber){
			highestnumber=matches[i];
			highest_num_index=i;

		}

	}
	if(matches[highest_num_index]<=1){
		map=preopen(newPath,mode,flag);
		matchedPath.dirfd=map[map->length-1].opened_files->dirfd;
		matchedPath.relative_path=newPath+strlen(map[map->length-1].opened_files->dirname)+1;
	}
	else{
		matchedPath.dirfd=map[highest_num_index].opened_files->dirfd;
		matchedPath.relative_path=newPath+(highestnumber+1);
	}
	return matchedPath;

}
/*
 Search for the best matching path in an array that matches a path argument
 Open and store the directoryfd and directory path in the array if no matching
 path is found in the array
*/

matched_path map_path(Map  *map,char* a_filepath){
	int i;
	matched_path mp;
	int mode=O_RDWR;
	int flag=0;
	int matched_num[map->length];

	for(i=0;i<map->length;i++){

		matched_num[i]=findMatchingChars(a_filepath,map->opened_files->dirname);
	}
	mp=getMostMatchedPath(matched_num,a_filepath,mode,flag);


	return mp;
}

