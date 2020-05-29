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
