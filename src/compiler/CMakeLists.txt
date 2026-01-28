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
        types/type_base.cc
        types/type_simple.cc
        types/type_procedure.cc
        types/type_array.cc
        types/type_record.cc
        types/type_pointer.cc
        types/type_set.cc
        typetable.cc)

target_include_directories(ax.compiler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${utfcpp_SOURCE_DIR}/source" )
