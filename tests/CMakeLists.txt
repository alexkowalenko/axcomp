#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

include(${PROJECT_SOURCE_DIR}/cmake/StandardProjectSettings.cmake)

add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")

set(INSTALL_GTEST OFF)

mark_as_advanced(
    # BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    BUILD_GTEST BUILD_SHARED_LIBS
    # gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

macro(package_add_test TESTNAME)
    # create an exectuable in which the tests will be stored
    add_executable(${TESTNAME} ${ARGN})
    # link the Google test infrastructure, mocking library, and a default main fuction to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_link_libraries(${TESTNAME} libax axtest gtest gtest_main ${llvm_libs} ${ICU_LIBRARIES})
    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TESTNAME}
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()              

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/extern/googletest/googletest/include)


add_library(axtest STATIC lib_test.cc fake_importer.cc) 

package_add_test(lex.1 lex.1.cc)
package_add_test(lex.2 lex.2.cc)

package_add_test(parse.1 parse.1.cc)
package_add_test(parse.2 parse.2.cc)
package_add_test(parse.3 parse.3.cc)
package_add_test(parse.4 parse.4.cc)
package_add_test(parse.5 parse.5.cc)
package_add_test(parse.6 parse.6.cc)

package_add_test(inspect.1 inspect.1.cc)
package_add_test(inspect.2 inspect.2.cc)
package_add_test(inspect.3 inspect.3.cc)
package_add_test(inspect.4 inspect.4.cc)

package_add_test(definition.1 definition.cc)

package_add_test(type.1 type.1.cc)