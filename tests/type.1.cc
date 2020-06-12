//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"
#include <memory>

#include "type.hh"
#include "typetable.hh"

using namespace ax;

TEST(Type, Basic) {

    TypeTable types;
    types.initialise();

    EXPECT_EQ(TypeTable::IntType->get_name(), "INTEGER");
    EXPECT_EQ(TypeTable::BoolType->get_name(), "BOOLEAN");
    EXPECT_EQ(TypeTable::CharType->get_name(), "CHAR");
    EXPECT_EQ(TypeTable::RealType->get_name(), "REAL");
    EXPECT_EQ(TypeTable::StrType->get_name(), "STRING");
}

TEST(Type, Structured) {

    TypeTable types;
    types.initialise();

    auto arrayType = std::make_shared<ArrayType>(TypeTable::IntType);
    EXPECT_EQ(arrayType->get_name(), "INTEGER[]");
    arrayType = std::make_shared<ArrayType>(TypeTable::RealType);
    EXPECT_EQ(arrayType->get_name(), "REAL[]");
    arrayType = std::make_shared<ArrayType>(TypeTable::CharType);
    EXPECT_EQ(arrayType->get_name(), "CHAR[]");
}

TEST(Type, Find) {

    TypeTable types;
    types.initialise();

    EXPECT_EQ(types.find("INTEGER"), TypeTable::IntType);
    EXPECT_EQ(types.find("CHAR"), TypeTable::CharType);
    EXPECT_EQ(types.find("BOOLEAN"), TypeTable::BoolType);
    EXPECT_EQ(types.find("REAL"), TypeTable::RealType);
    EXPECT_EQ(types.find("STRING"), TypeTable::StrType);
}

TEST(Type, Resolve) {

    TypeTable types;
    types.initialise();

    EXPECT_EQ(types.resolve("INTEGER"), TypeTable::IntType);
    EXPECT_EQ(types.resolve("CHAR"), TypeTable::CharType);
    EXPECT_EQ(types.resolve("BOOLEAN"), TypeTable::BoolType);
    EXPECT_EQ(types.resolve("REAL"), TypeTable::RealType);
    EXPECT_EQ(types.resolve("STRING"), TypeTable::StrType);

    EXPECT_EQ(types.resolve("SHORTINT"), TypeTable::IntType);
    EXPECT_EQ(types.resolve("LONGINT"), TypeTable::IntType);
    EXPECT_EQ(types.resolve("HUGEINT"), TypeTable::IntType);
    EXPECT_EQ(types.resolve("LONGREAL"), TypeTable::RealType);

    EXPECT_EQ(types.resolve("CHAR[]"), TypeTable::StrType);
}

TEST(Type, ResolveArrays) {

    TypeTable types;
    types.initialise();

    EXPECT_EQ(types.resolve("INTEGER"), TypeTable::IntType);

    auto type = std::make_shared<TypeAlias>("I", TypeTable::IntType);
    types.put("I", type);
    EXPECT_EQ(types.resolve("I"), TypeTable::IntType);

    // Resolve ARRAY OF I to ARRAY OF INTEGER
    auto t1 = std::make_shared<ArrayType>(TypeTable::IntType);
    // EXPECT_EQ(types.resolve("I[]"), t1);
}

TEST(Type, Records) {
    TypeTable types;
    types.initialise();

    auto rec_type = std::make_shared<ax::RecordType>();
    rec_type->insert("a", TypeTable::IntType);
    rec_type->insert("b", TypeTable::IntType);

    EXPECT_EQ(rec_type->get_index("a"), 0);
    EXPECT_EQ(rec_type->get_index("b"), 1);
    EXPECT_EQ(rec_type->get_index("x"), -1);

    auto rec1 = std::make_shared<ax::RecordType>();
    rec1->insert("x", TypeTable::IntType);
    rec1->insert("y", TypeTable::IntType);

    auto rec2 = std::make_shared<ax::RecordType>();
    rec2->set_baseType(rec1);
    rec2->insert("z", TypeTable::IntType);

    EXPECT_EQ(rec2->get_index("x"), 0);
    EXPECT_EQ(rec2->get_index("y"), 1);
    EXPECT_EQ(rec2->get_index("z"), 2);

    EXPECT_EQ(rec2->is_base(rec1), true);
    EXPECT_EQ(rec1->is_base(rec2), false);

    EXPECT_EQ(rec1->equiv(rec1), true);
    EXPECT_EQ(rec2->equiv(rec2), true);
    EXPECT_EQ(rec2->equiv(rec1), false);
    EXPECT_EQ(rec1->equiv(rec2), false);

    EXPECT_EQ(std::string(*rec_type), "{a,b}");

    // Pointers to RECORD
    TypePtr pr1 = std::make_shared<PointerType>(rec1);
    TypePtr pr2 = std::make_shared<PointerType>(rec2);

    EXPECT_EQ(pr1->equiv(pr1), true);
    EXPECT_EQ(pr2->equiv(pr2), true);
    EXPECT_EQ(pr1->equiv(pr2), false);
    EXPECT_EQ(pr2->equiv(pr1), true);
}

TEST(Type, String1) {
    TypeTable types;
    types.initialise();

    auto str1 = std::make_shared<ax::SimpleType>("STRING1", TypeId::str1);

    EXPECT_EQ(TypeTable::CharType->equiv(str1), true);
    EXPECT_EQ(TypeTable::StrType->equiv(str1), true);
}
