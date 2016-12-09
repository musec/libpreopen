/*
 * interceptOPen.c
 *
 *  Created on: Oct 15, 2016
 *      Author: ujay
 */
#define _GNU_SOURCE
#define _GNU_SOURCE
#include<dlfcn.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<unistd.h>
#include"../include/cwrapHeader.h"


int (*real_openat)(int fd1,const char* pathname1, int flags1)=NULL;
int open(const char *pathname,int mode){
	struct Map *map=getMap();

	 struct matched_path matchedPath=map_path(map,pathname,mode);
	real_openat=dlsym(RTLD_NEXT,"openat");
	return real_openat(matchedPath.dirfd,matchedPath.relative_path,mode);

}

