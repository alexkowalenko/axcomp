#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

#Boehm Garbage collector
message("Using Boehm Garbage collector")
set(BOEHM_INCLUDE_DIRS /usr/local/opt/bdw-gc/include)
set(BOEHM_LIBRARY_DIRS /usr/local/opt/bdw-gc/lib)
set(BOEHM_LIBRARIES gc gccpp)
include_directories(${BOEHM_INCLUDE_DIRS})
link_directories(${BOEHM_LIBRARY_DIRS})

add_library(Ax io.cc math.cc strings.cc)

install(FILES Out.def DESTINATION lib)
install(FILES Strings.def DESTINATION lib)
install(TARGETS Ax DESTINATION lib)