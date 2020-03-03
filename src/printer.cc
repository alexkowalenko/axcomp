//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "printer.hh"

#include <fmt/core.h>

#include "astmod.hh"

namespace ax {

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

void ASTPrinter::visit_ASTModule(ASTModule *ast) {
    os << fmt::format("MODULE {};\n", ast->name);
    visit_ASTDeclaration(ast->decs.get());
    for (auto const &proc : ast->procedures) {
        proc->accept(this);
    }
    os << fmt::format("BEGIN\n");
    for (auto const &x : ast->stats) {
        x->accept(this);
        os << ";\n";
    }
    os << fmt::format("END {}.\n", ast->name);
}

void ASTPrinter::visit_ASTDeclaration(ASTDeclaration *ast) {
    if (ast->cnst) {
        visit_ASTConst(ast->cnst.get());
    }
    if (ast->var) {
        visit_ASTVar(ast->var.get());
    }
}

void ASTPrinter::visit_ASTConst(ASTConst *ast) {
    if (!ast->consts.empty()) {
        os << "CONST\n";
        for (auto const &c : ast->consts) {
            visit_ASTIdentifier(c.first.get());
            os << " = ";
            c.second->accept(this);
            os << ";\n";
        }
    }
}

void ASTPrinter::visit_ASTVar(ASTVar *ast) {
    if (!ast->vars.empty()) {
        os << "VAR\n";
        for (auto const &c : ast->vars) {
            visit_ASTIdentifier(c.first.get());
            os << fmt::format(": {};\n", c.second);
        }
    }
}

void ASTPrinter::visit_ASTProcedure(ASTProcedure *ast) {
    os << fmt::format("PROCEDURE {}", ast->name);
    if (!ast->return_type.empty()) {
        os << fmt::format("(): {}", ast->return_type);
    }
    os << ";\n";
    ast->decs->accept(this);
    os << fmt::format("BEGIN\n");
    for (auto const &x : ast->stats) {
        x->accept(this);
        os << ";\n";
    }
    os << fmt::format("END {}.\n", ast->name);
}

void ASTPrinter::visit_ASTAssignment(ASTAssignment *ast) {
    visit_ASTIdentifier(ast->ident.get());
    os << " := ";
    visit_ASTExpr(ast->expr.get());
}

void ASTPrinter::visit_ASTReturn(ASTReturn *ast) {
    os << "RETURN ";
    if (ast->expr) {
        visit_ASTExpr(ast->expr.get());
    }
}

void ASTPrinter::visit_ASTCall(ASTCall *ast) {
    ast->name->accept(this);
    os << "()";
};

void ASTPrinter::visit_ASTExpr(ASTExpr *ast) {
    if (ast->first_sign) {
        os << string(ast->first_sign.value());
    }
    visit_ASTTerm(ast->term.get());
    for (auto t : ast->rest) {
        os << string(t.sign);
        visit_ASTTerm(t.term.get());
    }
}

void ASTPrinter::visit_ASTTerm(ASTTerm *ast) {
    visit_ASTFactor(ast->factor.get());
    for (auto t : ast->rest) {
        if (t.sign == TokenType::div || t.sign == TokenType::mod) {
            os << fmt::format(" {} ", string(t.sign));
        } else {
            os << string(t.sign);
        }
        visit_ASTFactor(t.factor.get());
    }
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