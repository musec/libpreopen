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
 * RUN: %cc %cflags -I %p/../lib %ldflags %s -o %t
 * RUN: %t > %t.out
 * RUN: %filecheck %s -input-file %t.out
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "internal.h"


void check_prefix(const char *dir, const char *path);


int main(int argc, char *argv[])
{
	// CHECK: '/foo' IS a prefix of '/foo'
	check_prefix("/foo", "/foo");

	// CHECK: '/foo' IS a prefix of '/foo/'
	check_prefix("/foo", "/foo/");

	// CHECK: '/foo' IS a prefix of '/foo/bar'
	check_prefix("/foo", "/foo/bar");

	// CHECK: '/foo' IS a prefix of '/foo/bar/baz/wibble'
	check_prefix("/foo", "/foo/bar/baz/wibble");

	// CHECK: '/foo' IS NOT a prefix of 'foo'
	check_prefix("/foo", "foo");

	// CHECK: '/foo' IS NOT a prefix of '/bar'
	check_prefix("/foo", "/bar");

	// CHECK: '/foo' IS NOT a prefix of '/foobar'
	check_prefix("/foo", "/foobar");

	return 0;
}


void check_prefix(const char *dir, const char *path)
{
	const bool success = po_isprefix(dir, strlen(dir), path);

	printf("'%s' %s a prefix of '%s'\n",
		dir, success ? "IS" : "IS NOT", path);
}
