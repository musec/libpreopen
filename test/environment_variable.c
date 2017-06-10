
/*
 * Copyright (c) 2016 Stanley Uche Godfrey
  All rights reserved.
 *This software was developed at Memorial University under the
 * NSERC Discovery program (RGPIN-2015-06048).
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
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

/* RUN: %cc -D PARENT %cflags -D LIBPRELOAD="\"%dir\"lib" -D TEST_DATA_DIR="\"%p/Inputs\"" %s %ldflags -o %t.parent
 * RUN: %cc -D CHILD %cflags -D TEST_DATA_DIR="\"%p/Inputs\"" %s %ldflags -o %t.child
 * RUN: %t.parent %t.child > %t.out
 * RUN: %filecheck %s -input-file %t.out
 */
#include <assert.h>
#include <sys/capsicum.h>
#include <sys/mman.h>
#include<stdlib.h>
#include<stdio.h>
#include"libpreopen.h"

#ifdef PARENT
#define TEST_DIR(name) \
	TEST_DATA_DIR name
char **environ;
int main(int arg, char *argv[]){
	int k,i,j;
	char buffer [20];
  	struct po_map *map = po_map_get();
	int foo = openat(AT_FDCWD, TEST_DIR("/foo"), O_RDONLY);
	po_add(map, "foo", foo);
	int wibble = po_preopen(map, TEST_DIR("/baz/wibble"));
	assert(wibble != -1);
	i= po_map_length( map);
	char env_data[j];
	k=po_create_shmdata(map);
	i=snprintf (buffer, sizeof(buffer),"%d",k);
	if(fcntl(k,  F_GETFD)!=0){
		fcntl(k,F_SETFD,0);
	}
	printf("buffer %s\n", buffer);
	setenv("LIB_PO_MAP",buffer,1);
	execve(argv[1],argv+1,environ);
	perror("execve");	

	return 0;
}

#else
#define TEST_DIR(name) \
	TEST_DATA_DIR name

#include <sys/stat.h>

int main(int arg, char *args[]){
	struct stat st; int i,j;
	struct po_map *map;
	char shm_string[20];
	int shmfd;
	shmfd=atoi(getenv("LIB_PO_MAP"));
	// CHECK: Opening foo/bar/hi.txt...
	printf("Opening foo/bar/hi.txt...\n");
	map=po_unpack_shm(shmfd);
	po_map_set(map);
	printf("map dir fd before passing eviro variable\n");
	for(i=0;i<po_map_length(map);i++){
		printf("%d\n",po_map_fd(map,i));
	}
	cap_enter();
	j=stat(TEST_DIR("/baz/wibble") "/bye.txt", &st);
	if(j==0){
		 printf("%4d\n", st.st_nlink);
	}
	else{
		perror("stat");
			
	}
	j=stat("foo/bar/hi.txt", &st);
	if(j==0){
		printf("%4d\n", st.st_nlink);
	}
	else{
		perror("stat");
	}
		

	// CHECK: Opening baz/wibble/bye.txt...
	printf("Opening baz/wibble/bye.txt...\n");
	return 0;
}

#endif
