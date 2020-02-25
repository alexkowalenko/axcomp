#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

set(PYTHON_EXECUTABLE python3)
find_package(PythonInterp 3.7 REQUIRED)

add_test(NAME xtests 
         COMMAND ${PYTHON_EXECUTABLE} ./tester.py
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})