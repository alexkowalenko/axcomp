#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

cmake_minimum_required(VERSION 3.22)

# Change compiler here
set(AX_BASE_CLANG /opt/homebrew/Cellar/llvm/17.0.0)
set(CMAKE_CXX_COMPILER ${AX_BASE_CLANG}/bin/clang++)
message(STATUS "C++ compiler = ${CMAKE_CXX_COMPILER}")

# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

project (axcomp VERSION 0.2
                LANGUAGES CXX C)
include(cmake/StandardProjectSettings.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_23)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
set(ENABLE_SANITIZER_ADDRESS OFF)
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
if(ENABLE_TESTING)
  message("Enabling testing")
  enable_testing()
  
  CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v1.13.0
    VERSION 1.13.0
    OPTIONS
      "INSTALL_GTEST OFF"
      "BUILD_GMOCK OFF"
      "gtest_force_shared_crt ON"
      "gtest_disable_pthreads ON" 
      "gtest_hide_internal_symbols ON"
  )

  add_subdirectory(tests)
  add_subdirectory(xtest)
endif()

# Fuzz Testing
option(ENABLE_FUZZING "Build the Fuzz Tests" ON)
if(ENABLE_FUZZING)
  message( "Enabling fuzz testing, using fuzzing sanitizer.")
  add_subdirectory(fuzz_test)
endif()

# Doxygen docs
add_subdirectory(docs)

add_subdirectory(src)
add_subdirectory(cmd)

# ax runtime library
add_subdirectory(runtime)

# install
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
