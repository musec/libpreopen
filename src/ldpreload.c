/*
 * ldpreload.c
 *
 *  Created on: Oct 15, 2016
 *      Author: ujay
 */

#include<stdio.h>
#include<fcntl.h>
#include"cwrapHeader.h"
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<dirent.h>
int main(){

	int fd,fd1;
	fd =open("test.txt",O_RDONLY);
	if(fd<0){
		printf("open() returned NULL\n");
		return 1;
	}
	else{
		printf("open() succeedded\n");
	}
	fd1=access("test.txt",R_OK);
	if(fd1<0){
		printf("access() returned NULL\n");
		return 1;
	}
	else{
		printf("access() succeedded\n");
	}


	return 0;
}



