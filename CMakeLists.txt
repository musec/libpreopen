cmake_minimum_required(VERSION 3.0.2)
include(GNUInstallDirs)

project(preopen)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

# Always build with all warnings.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes")

include_directories(include)

add_subdirectory(doc)
add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(test)

if (${CMAKE_SYSTEM_NAME} MATCHES FreeBSD)
	set(PKG_CONFIG_DEST "libdata/pkgconfig")
else ()
	set(PKG_CONFIG_DEST "lib/pkgconfig")
endif ()

configure_file("libpreopen.pc.in" "${CMAKE_BINARY_DIR}/libpreopen.pc" @ONLY)
install(FILES "${CMAKE_BINARY_DIR}/libpreopen.pc" DESTINATION "${PKG_CONFIG_DEST}")
