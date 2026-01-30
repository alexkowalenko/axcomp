//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <utility>
#include <vector>

#include "ast_basic.hh"

namespace ax {

/**
 * @brief "(" ident { [","] ident } ")"
 *
 */
class ASTEnumeration_ : public ASTBase_, public std::enable_shared_from_this<ASTEnumeration_> {
  public:
    ~ASTEnumeration_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::vector<ASTIdentifier> values;
};
using ASTEnumeration = std::shared_ptr<ASTEnumeration_>;

/**
 * @brief Qualident | arrayType | recordType | pointerType
 *
 */
class ASTType_ : public ASTBase_, public std::enable_shared_from_this<ASTType_> {
  public:
    ~ASTType_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::variant<ASTQualident, ASTArray, ASTRecord, ASTPointerType, ASTEnumeration> type;
};
using ASTType = std::shared_ptr<ASTType_>;

/**
 * @brief  "ARRAY" [ expr ( , expr ) ] "OF" type
 *
 */
class ASTArray_ : public ASTBase_, public std::enable_shared_from_this<ASTArray_> {
  public:
    ~ASTArray_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::vector<ASTInteger> dimensions;
    ASTType                 type;
};
using ASTArray = std::shared_ptr<ASTArray_>;

/**
 * @brief "RECORD" "(" qualident ")" fieldList ( ";" fieldList )* "END"
 *
 */

using VarDec = std::pair<ASTIdentifier, ASTType>;

class ASTRecord_ : public ASTBase_, public std::enable_shared_from_this<ASTRecord_> {
  public:
    ~ASTRecord_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    ASTQualident        base = nullptr;
    std::vector<VarDec> fields;
};
using ASTRecord = std::shared_ptr<ASTRecord_>;

/**
 * @brief "POINTER" "TO" type
 *
 */
class ASTPointerType_ : public ASTBase_, public std::enable_shared_from_this<ASTPointerType_> {
  public:
    ~ASTPointerType_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    ASTType reference;
};
using ASTPointerType = std::shared_ptr<ASTPointerType_>;

} // namespace ax
