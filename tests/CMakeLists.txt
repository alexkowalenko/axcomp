#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

include(GoogleTest)

set_target_properties(gtest PROPERTIES FOLDER _deps)
set_target_properties(gtest_main PROPERTIES FOLDER _deps)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

macro(package_add_test TESTNAME)
    # create an executable in which the tests will be stored
    add_executable(${TESTNAME} ${ARGN})

    # link the Google test infrastructure, mocking library, and a default main fuction to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_link_libraries(${TESTNAME} PRIVATE project_options project_warnings 
                            ax::compiler
                            ax::test
                            gtest gtest_main
                            ${llvm_libs}
                            ${ICU_LIBRARIES})
    
    # gtest_discover_tests replaces gtest_add_tests,
    gtest_discover_tests(${TESTNAME}
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
        DISCOVERY_TIMEOUT 120 # in seconds
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()              

include_directories(${PROJECT_SOURCE_DIR}/src/compiler)
include_directories("${utfcpp_SOURCE_DIR}/source")
include_directories("${gtest_SOURCE_DIR}/include")
include_directories("${fmt_SOURCE_DIR}/include")

add_library(ax.test)
add_library(ax::test ALIAS ax.test)

target_sources(ax.test PRIVATE
            lib_test.cc
            fake_importer.cc)

package_add_test(lex lex.1.cc)
package_add_test(parse parse.cc)
package_add_test(inspect inspect.cc)
package_add_test(definition definition.cc)
package_add_test(type type.1.cc)
package_add_test(importer importer_test.cc)
