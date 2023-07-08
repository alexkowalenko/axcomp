//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "ast.hh"
#include "astvisitor.hh"
#include "lexer.hh"
#include "symboltable.hh"
#include "typetable.hh"

namespace ax {

class Parser {
  public:
    Parser(LexerInterface &l, SymbolFrameTable &s, TypeTable &t, ErrorManager &e)
        : lexer{l}, symbols{s}, types{t}, errors{e} {};

    ASTModule parse();

  protected:
    ASTModule      parse_module();
    ASTImport      parse_import();
    ASTDeclaration parse_declaration();
    ASTConst       parse_const();
    ASTTypeDec     parse_typedec();
    void           parse_identList(std::vector<ASTIdentifier> &list);
    ASTVar         parse_var();

    void                parse_proc(ASTProc_ &proc);
    ASTProcedure        parse_procedure();
    ASTProcedureForward parse_procedureForward();
    void                parse_parameters(std::vector<VarDec> &params);
    RecVar              parse_receiver();

    ASTStatement parse_statement();
    void         parse_statement_block(std::vector<ASTStatement> &stats,
                                       const std::set<TokenType> &end_tokens);

    ASTAssignment parse_assignment(ASTDesignator d);
    ASTReturn     parse_return();
    ASTExit       parse_exit();
    ASTCall       parse_call(ASTDesignator d);
    ASTIf         parse_if();

    ASTCase parse_case();
    void    parse_caseElements(std::vector<ASTCaseElement> &elements);
    std::variant<ASTSimpleExpr, ASTRange> parse_caseLabel();

    ASTFor         parse_for();
    ASTWhile       parse_while();
    ASTRepeat      parse_repeat();
    ASTLoop        parse_loop();
    ASTBlock       parse_block();
    ASTExpr        parse_expr();
    ASTSimpleExpr  parse_simpleexpr();
    ASTTerm        parse_term();
    ASTFactor      parse_factor();
    ASTDesignator  parse_designator();
    ASTType        parse_type();
    ASTArray       parse_array();
    ASTRecord      parse_record();
    ASTPointerType parse_pointer();
    ASTQualident   parse_qualident();
    ASTIdentifier  parse_identifier();
    ASTInteger     parse_integer();
    ASTSet         parse_set();
    ASTReal        parse_real();
    ASTCharPtr     parse_char();
    ASTString      parse_string();
    ASTBool        parse_boolean();
    ASTNil         parse_nil();

    Token get_token(TokenType const &t);

    void set_attrs(ASTIdentifier const &ident);

    LexerInterface   &lexer;
    SymbolFrameTable &symbols;
    TypeTable        &types;
    ErrorManager     &errors;
};

template <class T> inline std::shared_ptr<T> makeAST(LexerInterface &lexer) {
    auto ast = make<T>();
    ast->set_location(lexer.get_location());
    return ast;
}

} // namespace ax