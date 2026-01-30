//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type.hh"

#include <map>

#include "type_pointer.hh"
#include "type_record.hh"

#include "../typetable.hh"

namespace ax {

namespace {

const std::map<TypeId, std::string> mapping{
    {TypeId::VOID, "void"},           {TypeId::ANY, "any"},         {TypeId::INTEGER, "integer"},
    {TypeId::REAL, "real"},           {TypeId::BOOLEAN, "boolean"},
    {TypeId::ENUMERATION, "enumeration"},
    {TypeId::CHAR, "chr"},
    {TypeId::PROCEDURE, "procedure"}, {TypeId::ARRAY, "array"},     {TypeId::STRING, "string"},
    {TypeId::RECORD, "record"},       {TypeId::ALIAS, "alias"},     {TypeId::POINTER, "pointer"},
    {TypeId::MODULE, "module"},
};

} // namespace

std::string const &string(TypeId const t) {
    return mapping.at(t);
}

bool Type_::equiv(Type const &t) {
    if (t->id == TypeId::STR1) {
        // String char strings are the same as STRING or CHAR
        return id == TypeId::STRING || id == TypeId::CHAR;
    }
    if (id != t->id) {
        return false;
    }
    if (id == TypeId::ENUMERATION) {
        return get_name() == t->get_name();
    }
    if (id == TypeId::RECORD) {
        auto *recordTypePtr = dynamic_cast<RecordType *>(this);
        return recordTypePtr->equiv(std::dynamic_pointer_cast<RecordType>(t)) ||
               recordTypePtr->is_base(t);
    }
    if (id == TypeId::POINTER) {
        auto      *pointerType = dynamic_cast<PointerType *>(this);
        const auto pt = std::dynamic_pointer_cast<PointerType>(t);
        if (pointerType->get_reference()->id == TypeId::RECORD &&
            pt->get_reference()->id == TypeId::RECORD) {
            auto pthisr = std::dynamic_pointer_cast<RecordType>(pointerType->get_reference());
            const auto ptr = std::dynamic_pointer_cast<RecordType>(pt->get_reference());
            if (pthisr->is_base(ptr)) {
                return true;
            }
            if (pthisr->equiv(ptr)) {
                return true;
            }
            return false;
        }
        if (pointerType->get_reference()->equiv(pt)) {
            return true;
        }
    }
    return true;
}

llvm::Value *Type_::min() const {
    return llvm::ConstantPointerNull::get(
        llvm::dyn_cast<llvm::PointerType>(TypeTable::VoidType->get_llvm()));
}

llvm::Value *Type_::max() const {
    return llvm::ConstantPointerNull::get(
        llvm::dyn_cast<llvm::PointerType>(TypeTable::VoidType->get_llvm()));
}

} // namespace ax
