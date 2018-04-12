if (${CMAKE_SYSTEM_NAME} MATCHES FreeBSD)
	add_definitions(-D WITH_CAPSICUM)
endif ()

set(LIBRARY_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

add_library(preopen SHARED
	libpreopen.c
	po_err.c
	po_libc_wrappers.c
	po_map.c
	po_pack.c
)
install(TARGETS preopen DESTINATION lib)
