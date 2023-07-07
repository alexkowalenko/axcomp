#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

include_directories(../src)

include_directories(Ax PUBLIC "${gc_SOURCE_DIR}/include")

add_library(Ax STATIC io.cc math.cc strings.cc)

target_link_libraries(Ax gc)

install(FILES Out.def DESTINATION lib)
install(FILES Math.def DESTINATION lib)
install(FILES Strings.def DESTINATION lib)
install(TARGETS Ax DESTINATION lib)