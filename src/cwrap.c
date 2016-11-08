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

Map *initializeMap(int capacity){
	int i;
	Map *map=(Map*)malloc(sizeof(Map));

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
int checkCapacity(){
	if(map->length>=map->capacity)
		return -1;
	else
		return 0;
}
// increases the capacity of map
Map* increaseMapCapacity(){
	int i;
	Map *new_map=(Map*)malloc(sizeof(Map));
	assert(new_map!=NULL);
	new_map->opened_files=(opened_dir_struct*)malloc((2*map->capacity)*sizeof(new_map->opened_files));
	assert(new_map->opened_files!=NULL);
	new_map->capacity=2*map->capacity;
	new_map->length=0;

		for(i=0;i<map->length;i++){
			new_map->opened_files[i]=map->opened_files[i];
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

opened_dir_struct * open_directory(char* file_path,opened_dir_struct *dos){
	int dir_fd,k,j; char * dirname;
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
		dir_fd=dirfd(dir);
		dos->dirfd=dir_fd;
		dos->dirname=dirname;

	}
	else{
		perror("");
	}

	return dos;

}


Map* add_Opened_dirpath_map(Map *map,opened_dir_struct ods){
	int current=map->length,i;
	map->opened_files[map->length]=ods;
	map->length++;
	return map;
}
Map* preopen(Map* map,char* file,int mode){
	int k;
	char* dirname;
	opened_dir_struct ods;
	opened_dir_struct * odsp;
	k=checkCapacity();
	if(k<0){
		map=increaseMapCapacity();
	}

	odsp=open_directory( file,&ods);
	map=add_Opened_dirpath_map(map,*odsp);
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
int  getMostMatchedPath(int matches[]){
	int highestnumber=0,i,highest_num_index=0;

	for(i=0;i<map->length;i++){
		if(matches[i]>highestnumber){
			highestnumber=matches[i];
			highest_num_index=i;

		}

	}

	return matches[highest_num_index];

}
/*compares matched path and see if the matched path is already opened
 * if not it opens the matched path else it returns the matched path dirfd
 * and relative path.
 */
matched_path compareMatched(Map* map,int best_matched_num,char *newPath,int mode){
	char * temp_dir,*t_dir,*filename;
	//const char* slash ="/";
	int i,status;
	matched_path  matchedPath;
	if(best_matched_num==0){
		map=preopen(map,newPath,mode);
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
					map=preopen(map,temp_dir,mode);
					matchedPath.dirfd=map->opened_files[map->length-1].dirfd;
					matchedPath.relative_path=t_dir;
				}

	}
	return matchedPath;
}
/*
 * Uses other function to return matched path
*/

matched_path map_path(Map* map,const char* a_filepath,int mode){
	int i; char * filename,*t_dir;
	int best_matched_num;
	matched_path matchedPath;
	int matched_num[map->length];
	filename=(char*)a_filepath;
	if(map->length==0){
				map=preopen(map,filename, O_RDONLY);
				t_dir=filename+strlen(map->opened_files[map->length-1].dirname);
				matchedPath.relative_path=t_dir;
				matchedPath.dirfd=map->opened_files[map->length-1].dirfd;

			}
	else{
		for(i=0;i<map->length;i++){

				matched_num[i]=findMatchingChars(filename,map->opened_files[i].dirname);
			}
		best_matched_num=getMostMatchedPath(matched_num);
		matchedPath=compareMatched(map,best_matched_num,filename, mode);
	}



	return matchedPath;
}

