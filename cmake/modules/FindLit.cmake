#
# Copyright (c) 2018 Jonathan Anderson
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
#

#
# Find lit, the LLVM Integrated Tester.
#
# This will define:
#   LIT_FOUND              whether we found a `lit` executable
#   LIT_EXECUTABLE         path to `lit` executable
#   LIT_COMMAND            command to run `lit` (including flags)
#

option(LIT_VERBOSE "Make lit test tool run in verbose mode" ON)

if (LIT_VERBOSE)
	set(LIT_OPTIONS "-sv")
else()
	set(LIT_OPTIONS "-s")
endif ()


find_program(LIT_EXECUTABLE lit
	# Look in user's Pip installation as well as system PATHs:
	PATHS
		$ENV{HOME}/.local/bin
		$ENV{PATH}
		/usr/local/llvm-devel/bin
		/usr/local/llvm60/bin
		/usr/local/llvm50/bin
		/usr/local/llvm40/bin

	DOC "Path to 'lit' (LLVM Integrated Tester) executable"
)

set(LIT_COMMAND "${LIT_EXECUTABLE};${LIT_OPTIONS}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(lit DEFAULT_MSG LIT_EXECUTABLE LIT_COMMAND)
mark_as_advanced(LIT_EXECUTABLE)
