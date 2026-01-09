include(cmake/CPM.cmake)

# LLVM
set(CMAKE_PREFIX_PATH  ${AX_BASE_CLANG}/lib/cmake)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs native passes
        # For future cross compiler
        # AllTargetsInfos AllTargetsAsmParsers AllTargetsAsmPrinters AllTargetsCodeGens
        # also try 'all'
)

set(LLVM_ENABLE_ASSERTIONS ON)
set(LLVM_ENABLE_EH ON)
set(LLVM_ENABLE_RTTI ON)

message(STATUS "Using LLVM libs: ${llvm_libs}")

# Unicode library
message("Using icu4c")
set(ICU_INCLUDE_DIRS /opt/homebrew/opt/icu4c/include)
set(ICU_LIBRARY_DIRS /opt/homebrew/opt/icu4c/lib)
set(ICU_LIBRARIES icuuc)
include_directories(${ICU_INCLUDE_DIRS})
link_directories(${ICU_LIBRARY_DIRS})

CPMAddPackage(
        NAME utfcpp
        GITHUB_REPOSITORY alexkowalenko/utfcpp
        GIT_TAG master
        OPTIONS
        "UTFCPP_BUILD_TESTS OFF"
        "UTFCPP_BUILD_DOC OFF"
        "UTFCPP_INSTALL OFF"
)

CPMAddPackage("gh:ivmai/bdwgc#v8.2.10")

CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.17.0
        VERSION 1.17.0
        OPTIONS
        "INSTALL_GTEST OFF"
        "BUILD_GMOCK OFF"
        "gtest_force_shared_crt ON"
        "gtest_disable_pthreads ON"
        "gtest_hide_internal_symbols ON"
)

CPMAddPackage(
        NAME argparse
        GITHUB_REPOSITORY p-ranav/argparse
        GIT_TAG v3.2)
