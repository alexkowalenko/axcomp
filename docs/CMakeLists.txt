#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)

doxygen_add_docs(docs
    ${PROJECT_SOURCE_DIR}/src
    "${CMAKE_CURRENT_SOURCE_DIR}/mainpage.md"
    WORKING_DIRECTORY
    "${PROJECT_SOURCE_DIR}/src"
)
