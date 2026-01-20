#
# AX compiler
#
# Copyright © 2023 Alex Kowalenko
#

add_library(ax.compiler)
add_library(ax::compiler ALIAS ax.compiler)

target_sources(ax.compiler PRIVATE
        ast.cc
        astvisitor.cc
        builtin.cc
        codegen.cc
        defparser.cc
        defprinter.cc
        error.cc
        importer.cc
        inspector.cc
        lexer.cc
        location.cc
        parser.cc
        printer.cc
        token.cc
        type.cc
        typetable.cc)

target_include_directories(ax.compiler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${utfcpp_SOURCE_DIR}/source" )
