#
# AX compiler
#
# Copyright © 2023 Alex Kowalenko
#

set (CMAKE_CXX_FLAGS "-DLLVM_ENABLE_STATS=1")

# src dir
add_library(libax STATIC 
  ast.cc
  error.cc 
  printer.cc
  parser.cc
  inspector.cc 
  lexerUTF8.cc 
  location.cc
  parser.cc
  token.cc
  type.cc
  typetable.cc
  builtin.cc
  astvisitor.cc
  inspector.cc 
  codegen.cc
  defprinter.cc
  defparser.cc
  importer.cc
)

include_directories(src)

target_include_directories(libax PUBLIC "${utfcpp_SOURCE_DIR}/source")
target_link_libraries(libax PUBLIC fmt)