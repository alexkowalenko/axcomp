#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

include(${PROJECT_SOURCE_DIR}/cmake/StandardProjectSettings.cmake)

# enable doxygen
include(../cmake/Doxygen.cmake)

set(ENABLE_DOXYGEN  ON)

enable_doxygen()

set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
