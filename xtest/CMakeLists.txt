#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

find_package(Python3)

set(ENV{AXLIB_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/lib)

macro(xtest_add_test TESTNAME)
    add_test(NAME xtest.${TESTNAME}
            COMMAND ${Python3_EXECUTABLE} ./tester.py -t ${TESTNAME}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()       

# Parse tests
xtest_add_test(parse.oo2c)
xtest_add_test(parse.ETHZ)

xtest_add_test(demo)
xtest_add_test(extra)

xtest_add_test(lang/const)
xtest_add_test(lang/type)
xtest_add_test(lang/var)

# xtest_add_test(lang/record)
xtest_add_test(lang/array)
# xtest_add_test(lang/openarray)
xtest_add_test(lang/set)

xtest_add_test(lang/literals)
xtest_add_test(lang/expr)
xtest_add_test(lang/bool)
xtest_add_test(lang/char)
# xtest_add_test(lang/string)
xtest_add_test(lang/numbers)
xtest_add_test(lang/assignment)
xtest_add_test(lang/conditional)
# xtest_add_test(lang/pointers)

# xtest_add_test(lang/procedure)
xtest_add_test(lang/forward)
# xtest_add_test(lang/closures)
# xtest_add_test(lang/procedure-bound)

xtest_add_test(lang/if)
xtest_add_test(lang/while)
xtest_add_test(lang/repeat)
xtest_add_test(lang/loop)
xtest_add_test(lang/for)
xtest_add_test(lang/case)
xtest_add_test(lang/return)

xtest_add_test(lang/import)

xtest_add_test(lib/assert)
xtest_add_test(lib/inc)
xtest_add_test(lib/char)
xtest_add_test(lib/halt)
xtest_add_test(lib/maths)
xtest_add_test(lib/set)
xtest_add_test(lib/size)
xtest_add_test(lib/write)
