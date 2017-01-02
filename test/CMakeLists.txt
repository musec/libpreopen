if (NOT LLVM_LIT)
	find_program(LLVM_LIT llvm-lit)
endif()

if (NOT LLVM_LIT)
	message(FATAL_ERROR "unable to find llvm-lit: change PATH "
		" or specify -D LLVM_LIT=/path/to/lit")
endif()

set(SHOW_TEST_STATS true CACHE BOOL "Show statistics after running tests")
if (SHOW_TEST_STATS)
  message(STATUS "Will show statistics after each test run")
  set(LIT_OPTIONS "-sv")
else ()
  message(STATUS "Will run successful tests quietly")
  set(LIT_OPTIONS "-qv")
endif ()


add_custom_target(check
	COMMAND
		${LLVM_LIT} ${LIT_OPTIONS} ${CMAKE_CURRENT_SOURCE_DIR}
		--param=build_dir=${CMAKE_BINARY_DIR}
		--param=source_dir=${CMAKE_SOURCE_DIR}
		--param=output_dir=${CMAKE_CURRENT_BINARY_DIR}
		--xunit-xml-output=test-results.xml

	BYPRODUCTS test-results.xml
	COMMENT "Running unit tests"
)

add_dependencies(check po)
