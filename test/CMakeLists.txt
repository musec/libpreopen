if (NOT LIT_EXECUTABLE)
	find_program(LIT_EXECUTABLE lit
		# Look in user's Pip installation as well as system PATH::
		PATHS $ENV{HOME}/.local/bin
		DOC "Path to 'lit' (LLVM Integrated Tester) executable"
	)
endif()

if (LIT_EXECUTABLE)
	add_custom_target(check
		COMMAND
			${LIT_EXECUTABLE} ${LIT_OPTIONS} ${CMAKE_CURRENT_SOURCE_DIR}
			--param=build_dir=${CMAKE_BINARY_DIR}
			--param=source_dir=${CMAKE_SOURCE_DIR}
			--param=output_dir=${CMAKE_CURRENT_BINARY_DIR}
			--xunit-xml-output=test-results.xml

		BYPRODUCTS test-results.xml
		COMMENT "Running unit tests"
	)

	 add_dependencies(check preopen)
else()
	 message(WARNING "Unable to find lit in PATH! If installed,"
		" specify -D LIT_EXECUTABLE=/path/to/lit")
endif()

