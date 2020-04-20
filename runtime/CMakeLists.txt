#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

add_library(Ax io.cc math.cc strings.cc )

install(FILES Out.def DESTINATION lib)
install(TARGETS Ax DESTINATION lib)