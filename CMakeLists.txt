#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

cmake_minimum_required(VERSION 3.16)

# Change compiler here
set(AX_BASE_CLANG /usr/local/Cellar/llvm/10.0.0)
set(CMAKE_CXX_COMPILER ${AX_BASE_CLANG}/bin/clang++)

project (axcomp VERSION 0.1
                LANGUAGES CXX)
include(cmake/StandardProjectSettings.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
set(ENABLE_SANITIZER_ADDRESS ON)
set(ENABLE_SANITIZER_MEMORY OFF) # Not on MacOS
set(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR ON)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# enable doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# allow for static analysis options
set(ENABLE_CLANG_TIDY OFF)
set(ENABLE_CPPCHECK OFF)
include(cmake/StaticAnalyzers.cmake)


option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable Test Builds" ON)

# Precompiled Headers
option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if (ENABLE_PCH)
  # This sets a global PCH parameter, each project will build its own PCH, which
  # is a good idea if any #define's change
  # 
  target_precompile_headers(project_options INTERFACE <vector> <string> <map> <utility>)
endif()

# Testing
option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
  message("Enabling testing")
  enable_testing()
  include(GoogleTest)
  add_subdirectory(tests)
  add_subdirectory(xtest)
endif()

# Fuzz Testing
option(ENABLE_FUZZING "Build the Fuzz Tests" ON)
if(ENABLE_FUZZING)
  message( "Enabling fuzz testing, using fuzzing sanitizer.")
  add_subdirectory(fuzz_test)
endif()

# External libraries
message("Using CLI11")
add_subdirectory(extern/CLI11)

# LLVM
set(CMAKE_PREFIX_PATH /usr/local/opt/llvm)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs native asmparser
  # For future cross compiler
  # AllTargetsInfos AllTargetsAsmParsers AllTargetsAsmPrinters AllTargetsCodeGens
  # also try 'all'
)                 

set(LLVM_ENABLE_ASSERTIONS ON)
set(LLVM_ENABLE_EH ON)
set(LLVM_ENABLE_RTTI ON)

message(STATUS "Using LLVM libs: ${llvm_libs}")

# Doxygen docs
add_subdirectory(docs)

# src dir
file(GLOB source_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cc)
add_library(libax STATIC ${source_files})
include_directories(src)
include_directories(extern/CLI11/include)

# ax compiler executable
add_executable(ax cmd/ax.cc)
target_link_libraries(ax PRIVATE project_options project_warnings libax ${llvm_libs})

# ax runtime library
add_subdirectory(lib)

# install
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
install(TARGETS ax DESTINATION bin)
