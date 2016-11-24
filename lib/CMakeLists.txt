cmake_minimum_required (VERSION 2.6)
project (CAPSH)
#Bring the headers, such as cwrapHeader.h into the project
include_directories(${include})
 
#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "*.c")
add_library(capwrap SHARED ${SOURCES})

#Set the location for library installation -- i.e., /usr/lib in this case
# not really necessary in this example. Use "sudo make install" to apply
#install(TARGETS capwrap DESTINATION /usr/lib)
