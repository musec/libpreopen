/*
 * Copyright (c) 2016 Jonathan Anderson and Stanley Uche Godfrey
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
 * RUN: %p/run-with-preload %lib %t > %t.out
 * RUN: %filecheck %s -input-file %t.out
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "libpreopen.h"

#define TEST_DIR(name) \
	TEST_DATA_DIR name


int main(int argc, char *argv[])
{
	struct po_map *map = po_map_create(4);

	int foo = openat(AT_FDCWD, TEST_DIR("/foo"), O_RDONLY);
	po_add(map, "foo", foo);

	int wibble = po_preopen(map, TEST_DIR("/baz/wibble"), O_DIRECTORY);
	assert(wibble != -1);

	po_set_libc_map(map);

	// CHECK: Opening foo/bar/hi.txt...
	printf("Opening foo/bar/hi.txt...\n");
	int fd = open("foo/bar/hi.txt", O_RDONLY);

	// CHECK-NOT: hi.txt: -1
	printf("hi.txt: %d\n", fd);

	// CHECK: Opening baz/wibble/bye.txt...
	printf("Opening baz/wibble/bye.txt...\n");
	fd = access(TEST_DIR("/baz/wibble") "/bye.txt", O_RDONLY);

	// CHECK-NOT: bye.txt: -1
	printf("bye.txt: %d\n", fd);

	return 0;
}
