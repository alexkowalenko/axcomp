#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

cmake_minimum_required(VERSION 3.16)

# Change compiler here
set(AX_BASE_CLANG /usr/local/opt/llvm)
set(CMAKE_CXX_COMPILER ${AX_BASE_CLANG}/bin/clang++)

project (axcomp VERSION 0.1
                LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests)
    add_subdirectory(xtest)
endif()

add_subdirectory(extern/fmt EXCLUDE_FROM_ALL)
add_subdirectory(extern/CLI11)

# LLVM
set(CMAKE_PREFIX_PATH /usr/local/opt/llvm)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs AllTargetsInfos AllTargetsAsmParsers AllTargetsAsmPrinters AllTargetsCodeGens)
message(STATUS "Using LLVM libs: ${llvm_libs}")

# Docs only available if this is the main app
find_package(Doxygen)
if(Doxygen_FOUND)
    add_subdirectory(docs)
else()
    message(STATUS "Doxygen not found, not building docs")
endif()

add_compile_options(-Wall)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# src dir
file(GLOB source_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cc)
add_library(libax STATIC ${source_files})
include_directories(src)
include_directories(extern/fmt/include)
include_directories(extern/CLI11/include)

# ax executable
add_executable(ax cmd/ax.cc)
target_link_libraries(ax libax fmt ${llvm_libs})

# install
set(CMAKE_INSTALL_PREFIX /Users/alex/Development/compilers/axcomp)
install(TARGETS ax DESTINATION bin)
