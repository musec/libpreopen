option(LIT_VERBOSE "Make lit test tool run in verbose mode" ON)

if (LIT_VERBOSE)
	set(LIT_OPTIONS "-sv")
else()
	set(LIT_OPTIONS "-s")
endif ()

set(TEST_CFLAGS "-Wall -g -I ${CMAKE_SOURCE_DIR}/include")
set(TEST_LDFLAGS "-L ${LIBRARY_BUILD_DIR} -l preopen")
set(TEST_LIBRARY_NAME
	"${CMAKE_SHARED_LIBRARY_PREFIX}preopen${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(TEST_LIBRARY_PATH "${LIBRARY_BUILD_DIR}/${TEST_LIBRARY_NAME}")

#
# Attempt to find FileCheck and lit:
#
if (NOT FILECHECK_EXECUTABLE)
	find_program(FILECHECK_EXECUTABLE FileCheck
		PATHS
			/usr/local/llvm-devel/bin
			/usr/local/llvm60/bin
			/usr/local/llvm50/bin
			/usr/local/llvm40/bin
			/usr/local/bin
			/usr/bin
		DOC "Path to 'lit' (LLVM Integrated Tester) executable"
	)
endif()

if (NOT LIT_EXECUTABLE)
	find_program(LIT_EXECUTABLE lit
		# Look in user's Pip installation as well as system PATH::
		PATHS $ENV{HOME}/.local/bin
		DOC "Path to 'lit' (LLVM Integrated Tester) executable"
	)
endif()



if (NOT LIT_EXECUTABLE)
	 message(WARNING "Unable to find lit in PATH! If installed,"
		" specify -D LIT_EXECUTABLE=/path/to/lit (or adjust PATH)")

elseif(NOT FILECHECK_EXECUTABLE)
	message(WARNING "Unable to find FileCheck in PATH! In installed,"
		" specify -D FILECHECK_EXECUTABLE=/path/to/FileCheck (or adjust PATH)")

else() # All's well: we have both lit and FileCheck
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
		${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg)

	add_custom_target(check
		COMMAND
			${LIT_EXECUTABLE} ${LIT_OPTIONS} ${CMAKE_CURRENT_BINARY_DIR}
			--xunit-xml-output=test-results.xml

		USES_TERMINAL
		BYPRODUCTS test-results.xml
		COMMENT "Running unit tests"
	)

	 add_dependencies(check preopen)
endif()

