#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

set(PYTHON_EXECUTABLE python3)
find_package(PythonInterp 3.7 REQUIRED)

set(ENV{AXLIB_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/lib)

macro(xtest_add_test TESTNAME)
    add_test(NAME xtest.${TESTNAME}
            COMMAND ${PYTHON_EXECUTABLE} ./tester.py -t ${TESTNAME}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()       

# Parse tests
xtest_add_test(parse.oo2c)
xtest_add_test(parse.ETHZ)

xtest_add_test(basic)
xtest_add_test(oberon0)
xtest_add_test(oberon-2)
xtest_add_test(import)
xtest_add_test(demo)
xtest_add_test(extra)
xtest_add_test(oo2c)

xtest_add_test(lang/const)
xtest_add_test(lang/type)
xtest_add_test(lang/var)

xtest_add_test(lang/record)

xtest_add_test(lang/bool)
xtest_add_test(lang/expr)
xtest_add_test(lang/conditional)

xtest_add_test(lib/maths)

xtest_add_test(lang/return)

