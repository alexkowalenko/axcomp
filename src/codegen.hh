//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>

#include "astvisitor.hh"

namespace ax {

class CodeGenerator : ASTVisitor {
  public:
    CodeGenerator(){};

    void generate(std::shared_ptr<ASTModule> ast) {
        visit_ASTModule(ast.get());
    };

    void visit_ASTModule(ASTModule *ast);
    void visit_ASTExpr(ASTExpr *ast);
    void visit_ASTInteger(ASTInteger *ast);

  private:
    void init();
    void generate_objectcode();
};

} // namespace ax