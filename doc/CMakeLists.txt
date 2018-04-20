find_package(Doxygen)
find_package(Git)

option(INTERNAL_DOCS "Document internal implementation details" OFF)

# Attempt to generate a version like v1.0+ab35sdf-dirty
if (GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --always --dirty --tags
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE CURRENT_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
else()
	set(CURRENT_VERSION "unknown")
endif()

if (DOXYGEN_FOUND)
	# Oh, CMake... why is this so difficult?
	if (INTERNAL_DOCS)
		set(DOXYGEN_INTERNAL_DOCS "YES")
		set(DOXYGEN_SECTIONS "internal")
	else ()
		set(DOXYGEN_INTERNAL_DOCS "NO")
		set(DOXYGEN_SECTIONS "")
	endif()

	configure_file(Doxyfile.in Doxyfile)
	set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html
		DEPENDS
			preopen
			${DOXYFILE}
			DoxygenLayout.xml
			${CMAKE_SOURCE_DIR}/README.md

		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
	)

	add_custom_target(doc ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html)
endif ()
