//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "printer.hh"

#include <algorithm>

#include <fmt/core.h>

#include "astmod.hh"

namespace ax {

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

void ASTPrinter::visit_ASTModule(ASTModule *ast) {
    os << fmt::format("MODULE {};\n", ast->name);
    ast->decs->accept(this);
    std::for_each(ast->procedures.begin(), ast->procedures.end(),
                  [this](auto const &proc) { proc->accept(this); });
    os << fmt::format("BEGIN\n");
    std::for_each(ast->stats.begin(), ast->stats.end(), [this](auto const &x) {
        x->accept(this);
        os << ";\n";
    });
    os << fmt::format("END {}.\n", ast->name);
}

void ASTPrinter::visit_ASTDeclaration(ASTDeclaration *ast) {
    if (ast->cnst) {
        ast->cnst->accept(this);
    }
    if (ast->var) {
        ast->var->accept(this);
    }
}

void ASTPrinter::visit_ASTConst(ASTConst *ast) {
    if (!ast->consts.empty()) {
        os << "CONST\n";
        std::for_each(ast->consts.begin(), ast->consts.end(),
                      [this](auto const &c) {
                          c.ident->accept(this);
                          os << " = ";
                          c.value->accept(this);
                          os << ";\n";
                      });
    }
}

void ASTPrinter::visit_ASTVar(ASTVar *ast) {
    if (!ast->vars.empty()) {
        os << "VAR\n";
        std::for_each(ast->vars.begin(), ast->vars.end(),
                      [this](auto const &v) {
                          v.first->accept(this);
                          os << fmt::format(": {};\n", v.second);
                      });
    }
}

void ASTPrinter::visit_ASTProcedure(ASTProcedure *ast) {
    os << fmt::format("PROCEDURE {}", ast->name);
    if (!ast->params.empty() || !ast->return_type.empty()) {
        os << "(";
        std::for_each(ast->params.begin(), ast->params.end(),
                      [this, ast](auto const &p) {
                          p.first->accept(this);
                          os << fmt::format(" : {}", p.second);
                          if (p != *(ast->params.end() - 1)) {
                              os << "; ";
                          }
                      });
        os << ")";
    }
    if (!ast->return_type.empty()) {
        os << fmt::format(": {}", ast->return_type);
    }
    os << ";\n";
    ast->decs->accept(this);
    os << fmt::format("BEGIN\n");
    std::for_each(ast->stats.begin(), ast->stats.end(), [this](auto const &x) {
        x->accept(this);
        os << ";\n";
    });
    os << fmt::format("END {}.\n", ast->name);
}

void ASTPrinter::visit_ASTAssignment(ASTAssignment *ast) {
    ast->ident->accept(this);
    os << " := ";
    ast->expr->accept(this);
}

void ASTPrinter::visit_ASTReturn(ASTReturn *ast) {
    os << "RETURN ";
    if (ast->expr) {
        ast->expr->accept(this);
    }
}

void ASTPrinter::visit_ASTCall(ASTCall *ast) {
    ast->name->accept(this);
    os << "(";
    std::for_each(ast->args.begin(), ast->args.end(),
                  [this, ast](auto const &a) {
                      a->accept(this);
                      if (a != *(ast->args.end() - 1)) {
                          os << ", ";
                      }
                  });
    os << ")";
};

void ASTPrinter::visit_ASTExpr(ASTExpr *ast) {
    if (ast->first_sign) {
        os << string(ast->first_sign.value());
    }
    ast->term->accept(this);
    std::for_each(ast->rest.begin(), ast->rest.end(), [this](auto t) {
        os << string(t.first);
        t.second->accept(this);
    });
}

void ASTPrinter::visit_ASTTerm(ASTTerm *ast) {
    ast->factor->accept(this);
    std::for_each(ast->rest.begin(), ast->rest.end(), [this](auto t) {
        if (t.first == TokenType::div || t.first == TokenType::mod) {
            os << fmt::format(" {} ", string(t.first));
        } else {
            os << string(t.first);
        }
        t.second->accept(this);
    });
}

void ASTPrinter::visit_ASTFactor(ASTFactor *ast) {
    std::visit(overloaded{[this](auto arg) { arg->accept(this); },
                          [this](std::shared_ptr<ASTExpr> const &arg) {
                              this->os << " (";
                              arg->accept(this);
                              this->os << ") ";
                          }},
               ast->factor);
}

void ASTPrinter::visit_ASTInteger(ASTInteger *ast) {
    os << fmt::format("{}", ast->value);
}

void ASTPrinter::visit_ASTIdentifier(ASTIdentifier *ast) {
    os << fmt::format("{}", ast->value);
};

} // namespace ax