//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <string>
#include <variant>
#include <vector>

#include "ast_base.hh"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wold-style-cast"
#include <utf8.h>
#pragma clang diagnostic pop

namespace ax {

///////////////////////////////////////////////////////////////////////////////
// Basic Objects

class ASTNil_ : public ASTBase_, public std::enable_shared_from_this<ASTNil_> {
  public:
    ~ASTNil_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };
};
using ASTNil = std::shared_ptr<ASTNil_>;

class ASTInteger_ : public ASTBase_, public std::enable_shared_from_this<ASTInteger_> {
  public:
    ~ASTInteger_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    Int  value{0};
    bool hex{false};
};
using ASTInteger = std::shared_ptr<ASTInteger_>;

class ASTReal_ : public ASTBase_, public std::enable_shared_from_this<ASTReal_> {
  public:
    ~ASTReal_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    Real        value{0.0};
    std::string style;
};
using ASTReal = std::shared_ptr<ASTReal_>;

class ASTBool_ : public ASTBase_, public std::enable_shared_from_this<ASTBool_> {
  public:
    ~ASTBool_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    Bool value{false};
};
using ASTBool = std::shared_ptr<ASTBool_>;

/**
 * @brief digit {hexDigit} "X" | "’" char "’"
 *
 */
class ASTChar_ : public ASTBase_, public std::enable_shared_from_this<ASTChar_> {
  public:
    ~ASTChar_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::string str() const {
        std::string s;
        utf8::append(static_cast<char32_t>(value), s);
        return s;
    }

    Char value = 0;
    bool hex{false};
};
using ASTChar = std::shared_ptr<ASTChar_>;

class ASTString_ : public ASTBase_, public std::enable_shared_from_this<ASTString_> {
  public:
    ~ASTString_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::string str() const { return delim + value + delim; }

    std::string value;
    char        delim{'"'};
};
using ASTString = std::shared_ptr<ASTString_>;

/**
 * @brief "{"" [ element {, element}] "}""
 *
 * element = expr [".." expr]
 *
 */
class ASTSet_ : public ASTBase_, public std::enable_shared_from_this<ASTSet_> {
  public:
    ~ASTSet_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::vector<std::variant<ASTSimpleExpr, ASTRange>> values;
};
using ASTSet = std::shared_ptr<ASTSet_>;

class ASTIdentifier_ : public ASTBase_, public std::enable_shared_from_this<ASTIdentifier_> {
  public:
    ASTIdentifier_() = default;
    explicit ASTIdentifier_(std::string n) : value(std::move(n)) {};
    ~ASTIdentifier_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    [[nodiscard]] bool is(Attr attr) const { return attrs.contains(attr); }
    void               set(Attr attr) { attrs.set(attr); }

    explicit virtual operator std::string() { return value; };

    std::string value;
    Attrs       attrs;
};
using ASTIdentifier = std::shared_ptr<ASTIdentifier_>;

/**
 * @brief Qualident = [ident "."] ident.
 *
 */
class ASTQualident_ : public ASTBase_, public std::enable_shared_from_this<ASTQualident_> {
  public:
    ASTQualident_() = default;
    explicit ASTQualident_(std::string &n) { id = make<ASTIdentifier_>(n); };
    ~ASTQualident_() override = default;

    ASTQualident_(ASTQualident_ const &o) = default;
    ASTQualident_ &operator=(ASTQualident_ const &other) = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::string                     qual;
    std::shared_ptr<ASTIdentifier_> id = nullptr;

    static std::string make_coded_id(std::string const &q, std::string const &i) {
        return q + "_" + i;
    }
    std::string make_coded_id() const {
        return qual.empty() ? id->value : make_coded_id(qual, id->value);
    }

    explicit operator std::string() const {
        return qual.empty() ? id->value : qual + "." + id->value;
    };
};
using ASTQualident = std::shared_ptr<ASTQualident_>;

} // namespace ax
