//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <set>
#include <vector>

#include "ast.hh"
#include "lexer.hh"
#include "symboltable.hh"
#include "typetable.hh"

namespace ax {
class Parser {
  public:
    explicit Parser(Lexer &l, std::shared_ptr<SymbolTable<TypePtr>> s)
        : lexer(l), symbols(std::move(s)){};

    std::shared_ptr<ASTModule> parse();

    void setup_builtins();

  private:
    std::shared_ptr<ASTModule>      parse_module();
    std::shared_ptr<ASTDeclaration> parse_declaration();
    std::shared_ptr<ASTConst>       parse_const();
    std::shared_ptr<ASTVar>         parse_var();
    std::shared_ptr<ASTProcedure>   parse_procedure();
    void parse_parameters(std::vector<VarDec> &params);

    std::shared_ptr<ASTStatement> parse_statement();
    void
    parse_statement_block(std::vector<std::shared_ptr<ASTStatement>> &stats,
                          std::set<TokenType> end_tokens);

    std::shared_ptr<ASTAssignment> parse_assignment();
    std::shared_ptr<ASTReturn>     parse_return();
    std::shared_ptr<ASTExit>       parse_exit();
    std::shared_ptr<ASTCall>       parse_call();
    std::shared_ptr<ASTIf>         parse_if();
    std::shared_ptr<ASTFor>        parse_for();
    std::shared_ptr<ASTWhile>      parse_while();
    std::shared_ptr<ASTRepeat>     parse_repeat();
    std::shared_ptr<ASTLoop>       parse_loop();
    std::shared_ptr<ASTBlock>      parse_block();
    std::shared_ptr<ASTExpr>       parse_expr();
    std::shared_ptr<ASTSimpleExpr> parse_simpleexpr();
    std::shared_ptr<ASTTerm>       parse_term();
    std::shared_ptr<ASTFactor>     parse_factor();
    std::shared_ptr<ASTDesignator> parse_designator();
    std::shared_ptr<ASTType>       parse_type();
    std::shared_ptr<ASTArray>      parse_array();
    std::shared_ptr<ASTIdentifier> parse_identifier();
    std::shared_ptr<ASTInteger>    parse_integer();
    std::shared_ptr<ASTBool>       parse_boolean();

    Token get_token(TokenType t);

    Lexer &                               lexer;
    std::shared_ptr<SymbolTable<TypePtr>> symbols;
};

extern std::vector<std::pair<std::string, std::shared_ptr<ProcedureType>>>
    builtins;

} // namespace ax