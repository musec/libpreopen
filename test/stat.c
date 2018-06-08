/*
 * Copyright (c) 2016, 2018 Jonathan Anderson
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
/*
 * RUN: %cc -c %cflags -D TEST_DATA_DIR="\"%p/Inputs\"" %s -o %t.o
 * RUN: %cc %t.o %ldflags -o %t
 * RUN: %p/run-with-preload %lib %t > %t.out
 * RUN: %filecheck %s -input-file %t.out
 */

#include <sys/stat.h>

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "libpreopen.h"

#define TEST_DIR(name) \
	TEST_DATA_DIR name


int main(int argc, char *argv[])
{	
	struct stat sb;
	int result;

	// CHECK: building map in [[MAP:0x[0-9a-f]+]]
	struct po_map *map = po_map_create(4);
	printf("building map in %p\n", map);
	
	// CHECK: foo: [[FOO:[0-9]+]]
	int foo = openat(AT_FDCWD, TEST_DIR("/foo"), O_RDONLY);
	printf("foo: %d\n", foo);
	assert(foo != -1);
	po_add(map, "foo", foo);

	// CHECK: wibble: [[WIBBLE:[0-9]+]]
	int wibble = po_preopen(map, TEST_DIR("/baz/wibble"), O_DIRECTORY);
	printf("wibble: %d\n", wibble);
	assert(wibble != -1);

	// CHECK-DAG: name: 'foo', fd: [[FOO]]
	// CHECK-DAG: name: '{{.*}}/Inputs/baz/wibble', fd: [[WIBBLE]]
	po_map_foreach(map, po_print_entry);

	po_set_libc_map(map);

	// CHECK: entered capability mode
	cap_enter();
	printf("entered capability mode to disallow the real stat(2)\n");

	// CHECK-NOT: error in stat('foo')
	if (stat("foo", &sb) < 0) {
		err(-1, "error in stat('foo')");
	} else if (S_ISDIR(sb.st_mode)) {
		printf("'foo' is a directory\n");
	}

	// CHECK: Opening baz/wibble/bye.txt...
	printf("Opening baz/wibble/bye.txt...\n");
	result = stat(TEST_DIR("/baz/wibble") "/bye.txt", &sb);

	// CHECK-NOT: bye.txt: -1
	printf("bye.txt: %d\n", result);

	return 0;
}
