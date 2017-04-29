

/*
 * Copyright (c) 2016 Jonathan Anderson & Stanley Uche Godfrey
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
/*
 * RUN: %cc -c %cflags -D TEST_DATA_DIR="\"%p/Inputs\"" %s -o %t.o
 * RUN: %cc %t.o %ldflags -o %t
 * RUN: %t > %t.out
 * RUN: %filecheck %s -input-file %t.out
 */

#include <sys/param.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "libpreopen.h"
#include<ctype.h>
#include <sys/capsicum.h>
#include<err.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>


#define TEST_DIR(name) \
	"/" TEST_DATA_DIR name

static int
send_file_descriptor(
  int socket, /* Socket through which the file descriptor is passed */
  int fd_to_send); /* File descriptor to be passed, could be another socket */

static int
recv_file_descriptor(
  int socket); /* Socket from which the file descriptor is read */

static void print_map_content( struct po_map *map);

int main(int argc, char *argv[])
{	pid_t child[2]; int k;
	int sockets[2];
	 if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
     		 perror("opening stream socket pair");
      		exit(1);
   	}
	//Creating first child process
	for(k=0;k<2;k++){
		child[k]=fork();
	}
	
	if(child[0]==0){
		
		close(sockets[0]);
		int shmfd,map_length,i;
		
		// CHECK: map: [[MAP:.*]]
		struct po_map *map = po_map_create(4);
		printf("map: 0x%tx\n", map);

		// CHECK: foo: [[FOO:.*]]
		int foo = open(TEST_DIR("/foo"), O_RDONLY | O_DIRECTORY);
		printf("foo: %d\n", foo);
		assert(foo != -1);
		// CHECK: map after foo: [[MAP]]
		map = po_add(map, "/foo", foo);
		printf("map after foo: 0x%tx\n", map);

		// CHECK: wibble: [[WIBBLE:.*]]
		int wibble = po_preopen(map, TEST_DIR("/baz/wibble"));
		printf("wibble: %d\n", wibble);
		assert(wibble != -1);

		// CHECK: map after wibble: [[MAP]]
		map = po_add(map, "/wibble", wibble);
		printf("map after wibble: 0x%tx\n", map);
	
		shmfd=po_create_shmdata(map);
		
		i= send_file_descriptor(sockets[1], shmfd);
		close(sockets[1]);
	}
	
	if(child[1]==0){
		close(sockets[1]);
		struct po_map* child_map=NULL;
		int len,client_shmfd;
		
		client_shmfd=recv_file_descriptor(sockets[0]);
		if(client_shmfd<0){
			exit(1);
		}
		child_map=po_reverse_create_shmdata(client_shmfd);
		print_map_content(child_map);
		close(sockets[0]);
		
	}	
	for(k=0;k<2;k++){
		wait(NULL);
	}


	return 0;
}

static int
send_file_descriptor(
  int socket, /* Socket through which the file descriptor is passed */
  int fd_to_send) /* File descriptor to be passed, could be another socket */
{
 struct msghdr message;
 struct iovec iov[1];
 struct cmsghdr *control_message = NULL;
 char ctrl_buf[CMSG_SPACE(sizeof(int))];
 char data[1];

 memset(&message, 0, sizeof(struct msghdr));
 memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));

 /* We are passing at least one byte of data so that recvmsg() will not return 0 */
 data[0] = ' ';
 iov[0].iov_base = data;
 iov[0].iov_len = sizeof(data);

 message.msg_name = NULL;
 message.msg_namelen = 0;
 message.msg_iov = iov;
 message.msg_iovlen = 1;
 message.msg_controllen =  CMSG_SPACE(sizeof(int));
 message.msg_control = ctrl_buf;

 control_message = CMSG_FIRSTHDR(&message);
 control_message->cmsg_level = SOL_SOCKET;
 control_message->cmsg_type = SCM_RIGHTS;
 control_message->cmsg_len = CMSG_LEN(sizeof(int));

 *((int *) CMSG_DATA(control_message)) = fd_to_send;

 return sendmsg(socket, &message, 0);
}

// A method for receiving file descriptor
static int
recv_file_descriptor(
  int socket) /* Socket from which the file descriptor is read */
{
 int sent_fd;
 struct msghdr message;
 struct iovec iov[1];
 struct cmsghdr *control_message = NULL;
 char ctrl_buf[CMSG_SPACE(sizeof(int))];
 char data[1];
 int res;

 memset(&message, 0, sizeof(struct msghdr));
 memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));

 /* For the dummy data */
 iov[0].iov_base = data;
 iov[0].iov_len = sizeof(data);

 message.msg_name = NULL;
 message.msg_namelen = 0;
 message.msg_control = ctrl_buf;
 message.msg_controllen = CMSG_SPACE(sizeof(int));
 message.msg_iov = iov;
 message.msg_iovlen = 1;

 if((res = recvmsg(socket, &message, 0)) <= 0)
  return res;

 /* Iterate through header to find if there is a file descriptor */
 for(control_message = CMSG_FIRSTHDR(&message);
     control_message != NULL;
     control_message = CMSG_NXTHDR(&message,
                                   control_message))
 {
  if( (control_message->cmsg_level == SOL_SOCKET) &&
      (control_message->cmsg_type == SCM_RIGHTS) )
  {
   return *((int *) CMSG_DATA(control_message));
  }
 }

 return -1;
}
static void
print_map_content( struct po_map *map)
{
	int i,map_length;
	char * path;
	map_length=get_map_length(map);
	printf("Map length is %i \n",map_length);
	for(i=0;i<map_length;i++){
		path=get_map_dirname(map,i);
		printf(" %d:-> %s \n", i,path);
	}
	
}

