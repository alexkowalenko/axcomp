# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE
      Debug #RelWithDebInfo
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui, ccmake
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

find_program(CCACHE ccache)
if(CCACHE)
  message("using ccache")
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
  message("ccache not found cannot use")
endif()

# Generate compile_commands.json to make it easier to work with clang based
# tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_IPO
       "Enable Iterprocedural Optimization, aka Link Time Optimization (LTO)"
       OFF)

if(ENABLE_IPO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT result OUTPUT output)
  if(result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(SEND_ERROR "IPO is not supported: ${output}")
  endif()
endif()

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

# LLVM
set(CMAKE_PREFIX_PATH  ${AX_BASE_CLANG}/lib/cmake)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs native asmparser passes
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

include(cmake/CPM.cmake)
CPMAddPackage("gh:nemtrif/utfcpp#v3.2.1")
CPMAddPackage("gh:ivmai/bdwgc#v8.2.4")
