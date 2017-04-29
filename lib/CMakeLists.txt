if (${CMAKE_SYSTEM_NAME} MATCHES FreeBSD)
	add_definitions(-D WITH_CAPSICUM)
endif ()

add_library(preopen SHARED
	libpreopen.c
	interceptaccess.c
	interceptopen.c
	interceptstat.c
)

install(TARGETS preopen DESTINATION lib)
