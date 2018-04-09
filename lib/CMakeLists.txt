if (${CMAKE_SYSTEM_NAME} MATCHES FreeBSD)
	add_definitions(-D WITH_CAPSICUM)
endif ()

set(LIBRARY_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

add_library(preopen SHARED
	libpreopen.c
	 wrappers.c
)
install(TARGETS preopen DESTINATION lib)
