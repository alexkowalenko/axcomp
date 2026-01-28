//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <utility>
#include <vector>

#include "ast_stmt.hh"

namespace ax {

//////////////////////
// Declaration objects

using RecVar = std::pair<ASTIdentifier, ASTIdentifier>;

class ASTProc_ : public ASTBase_ {
  public:
    ASTIdentifier       name;
    ASTType             return_type{nullptr};
    RecVar              receiver{nullptr, nullptr};
    std::vector<VarDec> params;
};
using ASTProc = std::shared_ptr<ASTProc_>;

class ASTProcedureForward_ : public ASTProc_,
                             public std::enable_shared_from_this<ASTProcedureForward_> {
  public:
    ~ASTProcedureForward_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };
};
using ASTProcedureForward = std::shared_ptr<ASTProcedureForward_>;

class ASTProcedure_ : public ASTProc_, public std::enable_shared_from_this<ASTProcedure_> {
  public:
    ~ASTProcedure_() override = default;

    void accept(ASTVisitor *v) override { v->visit(ASTProcedure_::shared_from_this()); };

    ASTDeclaration            decs;
    std::vector<ASTProc>      procedures;
    std::vector<ASTStatement> stats;

    std::vector<std::pair<std::string, Attrs>> free_variables;
};
using ASTProcedure = std::shared_ptr<ASTProcedure_>;

/**
 * @brief "VAR" (IDENT ":" type ";")*
 *
 */
class ASTVar_ : public ASTBase_, public std::enable_shared_from_this<ASTVar_> {
  public:
    ~ASTVar_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::vector<VarDec> vars;
};
using ASTVar = std::shared_ptr<ASTVar_>;

/**
 * @brief "TYPE" (IDENT "=" type ";")*
 *
 */
class ASTTypeDec_ : public ASTBase_, public std::enable_shared_from_this<ASTTypeDec_> {
  public:
    ~ASTTypeDec_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::vector<VarDec> types;
};
using ASTTypeDec = std::shared_ptr<ASTTypeDec_>;

struct ConstDec {
    ASTIdentifier ident;
    ASTExpr       value;
    ASTType       type;
};

/**
 * @brief "CONST" (IDENT "=" INTEGER ";")*
 *
 */
class ASTConst_ : public ASTBase_, public std::enable_shared_from_this<ASTConst_> {
  public:
    ~ASTConst_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::vector<ConstDec> consts;
};
using ASTConst = std::shared_ptr<ASTConst_>;

class ASTDeclaration_ : public ASTBase_, public std::enable_shared_from_this<ASTDeclaration_> {
  public:
    ~ASTDeclaration_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    ASTTypeDec type;
    ASTConst   cnst;
    ASTVar     var;
};
using ASTDeclaration = std::shared_ptr<ASTDeclaration_>;

/**
 * @brief "IMPORT" Import {"," Import} ";".
 *
 * Import = = [ident ":="] ident.
 *
 */
class ASTImport_ : public ASTBase_, public std::enable_shared_from_this<ASTImport_> {
  public:
    ~ASTImport_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    using Pair = std::pair<ASTIdentifier,  // Module
                           ASTIdentifier>; // Alias

    std::vector<Pair> imports;
};
using ASTImport = std::shared_ptr<ASTImport_>;

} // namespace ax
