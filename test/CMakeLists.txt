find_package(Lit)
find_package(FileCheck)

set(TEST_CFLAGS "-Wall -g -I ${CMAKE_SOURCE_DIR}/include")
set(TEST_LDFLAGS "-L ${LIBRARY_BUILD_DIR} -l preopen")
set(TEST_LIBRARY_NAME
	"${CMAKE_SHARED_LIBRARY_PREFIX}preopen${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(TEST_LIBRARY_PATH "${LIBRARY_BUILD_DIR}/${TEST_LIBRARY_NAME}")


if (NOT LIT_EXECUTABLE)
	message(WARNING "Unable to find lit in PATH! Cannot run tests.")

elseif(NOT FILECHECK_EXECUTABLE)
	message(WARNING "Unable to find FileCheck in PATH! Cannot run tests.")

else() # All's well: we have both lit and FileCheck
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
		${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg)

	add_custom_target(check
		COMMAND
			${LIT_COMMAND} ${CMAKE_CURRENT_BINARY_DIR}
				--xunit-xml-output=test-results.xml

		USES_TERMINAL
		BYPRODUCTS test-results.xml
		COMMENT "Running unit tests"
	)

	add_dependencies(check preopen)
endif()
