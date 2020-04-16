#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

include_directories(${PROJECT_SOURCE_DIR}/extern/fmt/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

# Allow short runs during automated testing to see if something new breaks
set(FUZZ_RUNTIME
    10
    CACHE STRING "Number of seconds to run fuzz tests during ctest run"
)# Default of 10 seconds


macro(package_add_test TESTNAME)
    # create an exectuable in which the tests will be stored
    add_executable(${TESTNAME} ${ARGN})

    target_link_libraries(${TESTNAME} PRIVATE project_options project_warnings -fsanitize=fuzzer,undefined,address,leak 
                                      libax ${llvm_libs})
    target_compile_options(${TESTNAME} PRIVATE -fsanitize=fuzzer,undefined,address,leak)
endmacro()

# LLVM
set(CMAKE_PREFIX_PATH /usr/local/opt/llvm)
find_package(LLVM REQUIRED CONFIG)
llvm_map_components_to_libnames(llvm_libs native)                  

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# The tests
package_add_test(fuzz_lex fuzz_lex.cpp)
add_test(NAME fuzz_lex COMMAND fuzz_lex -max_total_time=${FUZZ_RUNTIME} corpus)

package_add_test(fuzz_parse fuzz_parse.cpp)
add_test(NAME fuzz_parse COMMAND fuzz_parse -max_total_time=${FUZZ_RUNTIME} -only_ascii=1 corpus.parse)

execute_process(COMMAND "mkdir" -pv corpus corpus.parse
                WORKING_DIRECTORY fuzz_test)