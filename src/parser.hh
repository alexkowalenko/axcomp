//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <set>
#include <vector>

#include "ast.hh"
#include "astvisitor.hh"
#include "lexer.hh"
#include "symboltable.hh"
#include "typetable.hh"

namespace ax {

class Parser {
  public:
    Parser(Lexer &l, SymbolFrameTable &s, TypeTable &t, ErrorManager &e)
        : lexer{l}, symbols{s}, types{t}, errors{e} {};

    ASTModulePtr parse();

    void setup_builtins();

  protected:
    ASTModulePtr      parse_module();
    ASTImportPtr      parse_import();
    ASTDeclarationPtr parse_declaration();
    ASTConstPtr       parse_const();
    ASTTypeDecPtr     parse_typedec();
    void              parse_identList(std::vector<ASTIdentifierPtr> &list);
    ASTVarPtr         parse_var();
    ASTProcedurePtr   parse_procedure();
    void              parse_parameters(std::vector<VarDec> &params);

    ASTStatementPtr parse_statement();
    void            parse_statement_block(std::vector<ASTStatementPtr> &stats,
                                          const std::set<TokenType> &   end_tokens);

    ASTAssignmentPtr parse_assignment(ASTDesignatorPtr d);
    ASTReturnPtr     parse_return();
    ASTExitPtr       parse_exit();
    ASTCallPtr       parse_call(ASTDesignatorPtr d);
    ASTIfPtr         parse_if();
    ASTForPtr        parse_for();
    ASTWhilePtr      parse_while();
    ASTRepeatPtr     parse_repeat();
    ASTLoopPtr       parse_loop();
    ASTBlockPtr      parse_block();
    ASTExprPtr       parse_expr();
    ASTSimpleExprPtr parse_simpleexpr();
    ASTTermPtr       parse_term();
    ASTFactorPtr     parse_factor();
    ASTDesignatorPtr parse_designator();
    ASTTypePtr       parse_type();
    ASTArrayPtr      parse_array();
    ASTRecordPtr     parse_record();
    ASTQualidentPtr  parse_qualident();
    ASTIdentifierPtr parse_identifier();
    ASTIntegerPtr    parse_integer();
    ASTBoolPtr       parse_boolean();

    Token get_token(TokenType const &t);

    void set_attrs(ASTIdentifierPtr const &ident);

    Lexer &           lexer;
    SymbolFrameTable &symbols;
    TypeTable &       types;
    ErrorManager &    errors;
};

extern std::vector<std::pair<std::string, std::shared_ptr<ProcedureType>>> builtins;

template <class T> inline std::shared_ptr<T> makeAST(Lexer &lexer) {
    auto ast = make<T>();
    ast->set_location(lexer.get_location());
    return ast;
}

} // namespace ax