if (${CMAKE_SYSTEM_NAME} MATCHES FreeBSD)
	add_definitions(-D WITH_CAPSICUM)
endif ()

add_library(po SHARED
	libpo.c 
	interceptAccess.c 
	interceptOpen.c
)
