/*
 * Copyright (c) 2016 Jonathan Anderson
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

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libpreopen.h"
#define TEST_DIR(name) \
	"/" TEST_DATA_DIR name


static void find(const char *absolute, struct po_map *map);

int main(int argc, char *argv[])
{
	printf("-------------------------------------------------------\n");
	printf("Creating map: ");

	// CHECK: Creating map: [[MAP:.*]]
	struct po_map *map = po_map_create(4);
	printf("0x%p\n", map);

	printf("-------------------------------------------------------\n");

	// CHECK: foo: [[FOO:.*]]
	int foo = open(TEST_DIR("/foo"), O_RDONLY | O_DIRECTORY);
	printf("foo: %d\n", foo);
	assert(foo != -1);

	// CHECK: po_add("/foo", [[FOO]]) returned: [[MAP]]
	map = po_add(map, "/foo", foo);
	printf("po_add(\"/foo\", %d) returned: 0x%p\n", foo, map);

	// CHECK: - name: '/foo', fd: [[FOO]]
	po_map_foreach(map, po_print_entry);

	// CHECK: -----
	printf("-------------------------------------------------------\n");

	// CHECK: wibble: [[WIBBLE:.*]]
	int wibble = po_preopen(map, TEST_DIR("/baz/wibble"), O_DIRECTORY);
	printf("wibble: %d\n", wibble);
	assert(wibble != -1);

	// CHECK-DAG: - name: '/foo', fd: [[FOO]]
	// CHECK-DAG: - name: '{{.*}}/Inputs/baz/wibble', fd: [[WIBBLE]]
	po_map_foreach(map, po_print_entry);

	// CHECK: -----
	printf("-------------------------------------------------------\n");

	// Re-adding a file descriptor at a different path (kind of like a
	// nullfs mount) ought to work:

	// CHECK: po_add("/wibble", [[WIBBLE]]) returned: [[MAP]]
	map = po_add(map, "/wibble", wibble);
	printf("po_add(\"/wibble\", %d) returned: 0x%p\n", wibble, map);

	// CHECK-DAG: - name: '/foo', fd: [[FOO]]
	// CHECK-DAG: - name: '{{.*}}/Inputs/baz/wibble', fd: [[WIBBLE]]
	// CHECK-DAG: - name: '/wibble', fd: [[WIBBLE]]
	po_map_foreach(map, po_print_entry);

	// CHECK: -----
	printf("-------------------------------------------------------\n");

	// CHECK: /foo -> [[FOO]]:
	find("/foo", map);

	// CHECK: /foo/bar/baz -> [[FOO]]:bar/baz
	find("/foo/bar/baz", map);

	// CHECK: /wibble/foo -> [[WIBBLE]]:foo
	find("/wibble/foo", map);

	// CHECK: /bar/wibble/foo -> -1:
	find("/bar/wibble/foo", map);

	printf("-------------------------------------------------------\n");

	return 0;
}


static void
find(const char *absolute, struct po_map *map)
{
	struct po_relpath rel = po_find(map, absolute, NULL);
	printf("%s -> %d:%s\n", absolute, rel.dirfd, rel.relative_path);
}
