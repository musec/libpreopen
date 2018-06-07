/*
 * Copyright (c) 2016 Stanley Uche Godfrey
 * Copyright (c) 2017-2018 Jonathan Anderson
 * All rights reserved.
 *
 * This software was developed at Memorial University under the
 * NSERC Discovery program (RGPIN-2015-06048).
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
 *
 * UNSUPPORTED: linux
 *
 * RUN: %cc -D PARENT %cflags -D TEST_DATA_DIR="\"%p/Inputs\"" %s %ldflags -o %t.parent
 * RUN: %cc -D CHILD %cflags -D TEST_DATA_DIR="\"%p/Inputs\"" %s %ldflags -o %t.child
 * RUN: %t.parent %t.child > %t.out
 * RUN: %filecheck %s -input-file %t.out
 */

#include <sys/capsicum.h>
#include <sys/mman.h>

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "libpreopen.h"

#define TEST_DIR(name) \
	TEST_DATA_DIR name

#ifdef PARENT

char **environ;

int main(int argc, char *argv[]){
	char buffer[20];
	struct po_map *map;
	int len, shmfd;
	int foo, wibble;

	// CHECK: {{.*}}.parent
	printf("----------------------------------------"
		"----------------------------------------\n");
	printf("%s\n", argv[0]);
	printf("----------------------------------------"
		"----------------------------------------\n");

	if (argc < 2) {
		errx(-1, "Usage: %s <binary to exec as child>", argv[0]);
	}

	map = po_map_create(4);

	// CHECK: saving [[FOO:[0-9]+]] as "foo"
	foo = openat(AT_FDCWD, TEST_DIR("/foo"), O_RDONLY);
	assert(foo >= 0);
	printf("saving %d as \"foo\"\n", foo);
	po_add(map, "foo", foo);

	// CHECK: pre-opened [[WIBBLE:[0-9]+]] as "[[WIBBLE_PATH:.*]]"
	wibble = po_preopen(map, TEST_DIR("/baz/wibble"), O_DIRECTORY);
	assert(wibble >= 0);
	printf("pre-opened %d as \"%s\"\n", wibble, TEST_DIR("/baz/wibble"));

	// CHECK: packed map into SHM [[SHMFD:[0-9]+]]
	shmfd = po_pack(map);
	printf("packed map into SHM %d\n", shmfd);

	// CHECK: unpacked SHM into map at [[COPY:0x.*]]
	struct po_map *unpacked_copy = po_unpack(shmfd);
	printf("unpacked SHM into map at %p\n", unpacked_copy);

	// CHECK: contents of copy at [[COPY]]:
	// CHECK-DAG: name: 'foo', fd: [[FOO]]
	// CHECK-DAG: name: '[[WIBBLE_PATH]]', fd: [[WIBBLE]]
	printf("contents of copy at %p:\n", unpacked_copy);
	po_map_foreach(unpacked_copy, po_print_entry);

	// clear close-on-exec flag: we want this to be propagated!
	fcntl(shmfd, F_SETFD, 0);

	// CHECK: set LIB_PO_MAP=[[SHMFD]]
	snprintf(buffer, sizeof(buffer),"%d", shmfd);
	setenv("LIB_PO_MAP", buffer, 1);
	printf("set LIB_PO_MAP=%s\n", getenv("LIB_PO_MAP"));

	fflush(stdout);

	execve(argv[1], argv+1, environ);
	err(-1, "failed to execute '%s'", argv[1]);

	return 0;
}

#else

#include <sys/stat.h>

int main(int argc, char *argv[])
{
	char buffer[1024];
	struct stat st;
	struct po_map *map;
	char *end, *env;
	int fd, i, shmfd;

	// CHECK: {{.*}}.child
	printf("----------------------------------------"
		"----------------------------------------\n");
	printf("%s\n", argv[0]);
	printf("----------------------------------------"
		"----------------------------------------\n");

	cap_enter();
	// Attempt to unwrap po_map from a shared memory segment specified by
	// SHARED_MEMORYFD
	env = getenv("LIB_PO_MAP");
	if (env == NULL || *env == '\0') {
		return (NULL);
	}

	// We expect this environment variable to be an integer and nothing but
	// an integer.
	// CHECK: got shmfd: [[SHMFD]]
	shmfd = strtol(env, &end, 10);
	if (*end != '\0') {
		err(-1, "failed to extract SHM FD from envvar '%s'", env);
	}
	printf("got shmfd: %d\n", shmfd);

	// CHECK: unpacked map: [[MAP:0x[0-9a-f]+]]
	map = po_unpack(shmfd);
	if (map == NULL) {
		err(-1, "failed to unpack map");
	}
	printf("unpacked map: %p\n", map);

	// CHECK: contents of [[MAP]]:
	// CHECK-DAG: name: 'foo', fd: [[FOO]]
	// CHECK-DAG: name: '[[WIBBLE_PATH]]', fd: [[WIBBLE]]
	printf("contents of %p:\n", map);
	po_map_foreach(map, po_print_entry);

	return 0;
}

#endif
