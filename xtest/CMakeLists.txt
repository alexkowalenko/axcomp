#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

find_package(PythonInterp REQUIRED)

add_test(NAME xtests 
         COMMAND ${PYTHON_EXECUTABLE} ./tester.py
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})