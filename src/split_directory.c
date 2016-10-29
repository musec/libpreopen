/*
 * split_directory.c
 *
 *  Created on: Oct 15, 2016
 *      Author: ujay
 */
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include"cwrapHeader.h"

char *directoryArray[40];
char *fileArray[40];
char *path;
char tempstr[140];
int i=0;
void split_path(char * path_args){
	path=path_args;
	while(path[i]){
		printf("%c\n",path[i]);
		i++;
	}

}
char* split_path_file(char *relative_path) {
    const char slash='/';
   	char *filename;
	char *dirName;
	filename= strrchr(relative_path, slash);
	dirName=strndup(relative_path,strlen(relative_path)- strlen(filename));


     printf("%s\n",dirName);
     printf("%s\n",filename);
	return dirName;
}

int main(){
	char * a_path ="/usr/james/gogo/real/pc.c";
	printf("%ld\n",strlen(a_path));
	split_path_file(a_path);


	return 0;
}



