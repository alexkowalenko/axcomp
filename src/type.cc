//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type.hh"
#include "typetable.hh"

#include <llvm/Support/Debug.h>
#include <llvm/Support/FormatVariadic.h>
#include <memory>

#include "utf8.h"

namespace ax {

using namespace llvm;

static std::unordered_map<TypeId, std::string> mapping{
    {TypeId::null, "null"},           {TypeId::any, "any"},         {TypeId::integer, "integer"},
    {TypeId::real, "real"},           {TypeId::boolean, "boolean"}, {TypeId::chr, "chr"},
    {TypeId::procedure, "procedure"}, {TypeId::array, "array"},     {TypeId::string, "string"},
    {TypeId::record, "record"},       {TypeId::alias, "alias"},     {TypeId::pointer, "pointer"},
    {TypeId::module, "module"},
};

std::string string(TypeId const t) {
    return mapping[t];
}

bool Type::equiv(TypePtr const &t) {
    if (t->id == TypeId::str1) {
        // String char strings are the same as STRING or CHAR
        return id == TypeId::string || id == TypeId::chr;
    }
    if (id != t->id) {
        return false;
    }
    if (id == TypeId::record) {
        auto *rthis = dynamic_cast<RecordType *>(this);
        return rthis->equiv(std::dynamic_pointer_cast<RecordType>(t)) || rthis->is_base(t);
    }
    if (id == TypeId::pointer) {
        auto *pthis = dynamic_cast<PointerType *>(this);
        auto  pt = std::dynamic_pointer_cast<PointerType>(t);
        if (pthis->get_reference()->id == TypeId::record &&
            pt->get_reference()->id == TypeId::record) {
            auto pthisr = std::dynamic_pointer_cast<RecordType>(pthis->get_reference());
            auto ptr = std::dynamic_pointer_cast<RecordType>(pt->get_reference());
            if (pthisr->is_base(ptr)) {
                return true;
            }
            if (pthisr->equiv(ptr)) {
                return true;
            }
            return false;
        }
        if (pthis->get_reference()->equiv(pt)) {
            return true;
        }
    }
    return true;
}

llvm::Value *Type::min() {
    return llvm::ConstantPointerNull::get(
        dyn_cast<llvm::PointerType>(TypeTable::VoidType->get_llvm()));
}

llvm::Value *Type::max() {
    return llvm::ConstantPointerNull::get(
        dyn_cast<llvm::PointerType>(TypeTable::VoidType->get_llvm()));
}

SimpleType::operator std::string() {
    return name;
}

llvm::Constant *IntegerType::make_value(long i) {
    return ConstantInt::get(get_llvm(), i);
}

llvm::Constant *BooleanType::make_value(bool b) {
    return ConstantInt::get(get_llvm(), static_cast<uint64_t>(b));
}

llvm::Constant *CharacterType::make_value(Char c) {
    return ConstantInt::get(get_llvm(), static_cast<Char>(c));
}

llvm::Constant *StringType::make_value(std::string const &s) {

    std::vector<Constant *> array;
    auto                    it = s.cbegin();
    while (it != s.cend()) {
        auto c = utf8::next(it, s.end());
        array.push_back(TypeTable::CharType->make_value(c));
    }
    array.push_back(TypeTable::CharType->make_value(0)); // null terminate
    return ConstantArray::get(llvm::dyn_cast<llvm::ArrayType>(make_type(s)), array);
}

llvm::Type *StringType::make_type(std::string const & /* unused */) {
    // Type dependant on string size

    return llvm::ArrayType::get(TypeTable::CharType->get_llvm(),
                                // utf8::distance(s.begin(), s.end()) + 1
                                0); // All strings are undetermined length
}

ProcedureType::operator std::string() {
    std::string res{"("};
    for (auto &t : params) {
        if (t.second == Attr::var) {
            res += " VAR ";
        }
        res += std::string(*t.first);
        if (t != *(params.end() - 1)) {
            res += ",";
        }
    }
    res += "):";
    res += std::string(*ret);
    return res;
}

llvm::Type *ProcedureType::get_llvm() {
    std::vector<llvm::Type *> proto;
    std::for_each(cbegin(params), cend(params),
                  [this, &proto](auto const &t) { proto.push_back(t.first->get_llvm()); });

    return FunctionType::get(ret->get_llvm(), proto, false);
}

ProcedureFwdType::operator std::string() {
    std::string res{"^("};
    for (auto &t : params) {
        if (t.second == Attr::var) {
            res += " VAR ";
        }
        res += std::string(*t.first);
        if (t != *(params.end() - 1)) {
            res += ",";
        }
    }
    res += "):";
    res += std::string(*ret);
    return res;
}

ArrayType::operator std::string() {
    std::string result = llvm::formatv("{0}[", std::string(*base_type));
    for (auto iter = dimensions.cbegin(); iter != dimensions.cend(); iter++) {
        result += llvm::formatv("{0}", *iter);
        if ((iter + 1) != dimensions.end()) {
            result += ',';
        }
    };
    result += ']';
    return result;
}

llvm::Type *ArrayType::get_llvm() {
    if (dimensions.empty()) {
        return llvm::ArrayType::get(base_type->get_llvm(), 0);
    }

    llvm::Type *array_type = base_type->get_llvm();
    for (auto d : dimensions) {
        array_type = llvm::ArrayType::get(array_type, d);
    }
    return array_type;
}

llvm::Constant *ArrayType::get_init() {
    if (dimensions.empty()) {
        auto const_array = std::vector<Constant *>(0, base_type->get_init());
        return ConstantArray::get(dyn_cast<llvm::ArrayType>(get_llvm()), const_array);
    }

    std::vector<Constant *> const_array =
        std::vector<Constant *>(dimensions[0], base_type->get_init());
    return ConstantArray::get(dyn_cast<llvm::ArrayType>(get_llvm()), const_array);
}

RecordType::operator std::string() {
    std::string str{"{"};
    if (base) {
        str += "(" + std::string(*base) + ")";
    }
    for (auto iter = index.cbegin(); iter != index.cend(); ++iter) {
        str += *iter;
        if ((iter + 1) != index.cend()) {
            str += ',';
        }
    };
    str += "}";
    return str;
}

std::vector<llvm::Type *> RecordType::get_fieldTypes() {
    std::vector<llvm::Type *> fs;
    if (base) {
        auto b_fs = base->get_fieldTypes();
        fs.insert(cbegin(fs), cbegin(b_fs), cend(b_fs));
    }
    std::for_each(cbegin(index), cend(index), [&fs, this](auto const &name) {
        auto res = TypeTable::sgl()->resolve(fields[name]->get_name());
        fs.push_back(res->get_llvm());
    });
    if (fs.empty()) {
        // Empty records will crash LLVM
        fs.push_back(TypeTable::BoolType->get_llvm());
    }
    return fs;
}

llvm::Type *RecordType::get_llvm() {
    auto fs = get_fieldTypes();
    if (identified.empty()) {
        return StructType::create(fs);
    }
    return StructType::create(fs, identified);
}

std::vector<llvm::Constant *> RecordType::get_fieldInit() {
    std::vector<llvm::Constant *> fs;
    if (base) {
        auto b_fs = base->get_fieldInit();
        fs.insert(cbegin(fs), cbegin(b_fs), cend(b_fs));
    }
    std::for_each(cbegin(index), cend(index), [&fs, this](auto const &name) {
        auto res = TypeTable::sgl()->resolve(fields[name]->get_name());
        fs.push_back(res->get_init());
    });
    if (fs.empty()) {
        fs.push_back(TypeTable::BoolType->get_init());
    }
    return fs;
}

llvm::Constant *RecordType::get_init() {
    auto fs = get_fieldInit();
    return ConstantStruct::get(dyn_cast<llvm::StructType>(get_llvm()), fs);
}

void RecordType::insert(std::string const &field, TypePtr type) {
    fields[field] = std::move(type);
    index.push_back(field);
}

bool RecordType::has_field(std::string const &field) {
    if (fields.find(field) != fields.end()) {
        return true;
    }
    if (base) {
        return base->has_field(field);
    }
    return false;
}

unsigned int RecordType::get_size() {
    return std::accumulate(cbegin(fields), cend(fields), cbegin(fields)->second->get_size(),
                           [](unsigned int x, auto &y) { return x + y.second->get_size(); });
}

std::optional<TypePtr> RecordType::get_type(std::string const &field) {
    auto res = fields.find(field);
    if (res != fields.end()) {
        return std::make_optional<TypePtr>(res->second);
    }
    if (base) {
        return base->get_type(field);
    }
    return std::nullopt;
}

int RecordType::get_index(std::string const &field) {
    int base_count{0};
    if (base) {
        auto d = base->get_index(field);
        if (d >= 0) {
            return d;
        }
        base_count = base->count();
    }
    auto it = std::find(cbegin(index), cend(index), field);
    if (it == end(index)) {
        return -1;
    }
    return base_count + std::distance(cbegin(index), it);
}

bool RecordType::is_base(TypePtr t) {
    if (!t || !base) {
        return false;
    }
    if (t->id != TypeId::record) {
        return false;
    }
    return t->equiv(base);
}

bool RecordType::equiv(std::shared_ptr<RecordType> r) {

    if ((base != nullptr) != (r->base != nullptr)) { // xor
        return false;
    }
    if (fields.size() != r->fields.size()) {
        return false;
    }
    for (int i = 0; i < fields.size(); i++) {
        if (!fields[index[i]]->equiv(r->fields[r->index[i]])) {
            return false;
        }
    }
    return true;
}

llvm::Type *PointerType::get_llvm() {
    return reference->get_llvm()->getPointerTo();
}

llvm::Constant *PointerType::get_init() {
    return Constant::getNullValue(get_llvm());
}

llvm::Type *SetCType::get_llvm() {
    return TypeTable::IntType->get_llvm(); // 64-bit set
};

llvm::Constant *SetCType::get_init() {
    return TypeTable::IntType->get_init(); // 64-bit set
};

llvm::Value *SetCType::min() {
    return TypeTable::IntType->get_init();
};

llvm::Value *SetCType::max() {
    return TypeTable::IntType->make_value(SET_MAX);
};

} // namespace ax
