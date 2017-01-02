if (${CMAKE_SYSTEM_NAME} MATCHES FreeBSD)
	add_definitions(-D WITH_CAPSICUM)
endif ()

add_library(capwrap SHARED
	cwrap.c 
	interceptAccess.c 
	interceptOpen.c
)
