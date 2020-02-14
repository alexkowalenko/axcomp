cmake_minimum_required(VERSION 3.16)

# Change compiler here
set(AX_BASE_CLANG /usr/local/opt/llvm)
set(CMAKE_CXX_COMPILER ${AX_BASE_CLANG}/bin/clang++)

project (axcomp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wshadow -Wpedantic)

add_executable(ax cmd/ax.cc)


# install
set(CMAKE_INSTALL_PREFIX /Users/alex/Development/compilers/axcomp)
install(TARGETS ax DESTINATION bin)
