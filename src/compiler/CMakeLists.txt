#
# AX compiler
#
# Copyright © 2023 Alex Kowalenko
#

set (CMAKE_CXX_FLAGS "-DLLVM_ENABLE_STATS=1")

set(LIBAX_SOURCES
  ast.cc
  astvisitor.cc
  builtin.cc
  codegen.cc
  defparser.cc
  defprinter.cc
  error.cc
  importer.cc
  inspector.cc
  lexerUTF8.cc
  location.cc
  parser.cc
  printer.cc
  token.cc
  type.cc
  typetable.cc
)

add_library(libax STATIC ${LIBAX_SOURCES})

target_include_directories(libax
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${utfcpp_SOURCE_DIR}/source"
)
