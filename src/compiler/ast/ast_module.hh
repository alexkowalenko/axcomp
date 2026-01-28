//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <string>
#include <vector>

#include "ast_decl.hh"

namespace ax {

class ASTModule_ : public ASTBase_, public std::enable_shared_from_this<ASTModule_> {
  public:
    ~ASTModule_() override = default;
    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::string               name;
    ASTImport                 import;
    ASTDeclaration            decs;
    std::vector<ASTProc>      procedures;
    std::vector<ASTStatement> stats;
};
using ASTModule = std::shared_ptr<ASTModule_>;

} // namespace ax
