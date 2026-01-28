//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type_record.hh"

#include <format>
#include <ranges>

#include "../typetable.hh"

namespace ax {

RecordType::operator std::string() {
    std::string str{"{"};
    if (base) {
        str += "(" + string(base) + ")";
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

std::vector<llvm::Type *> RecordType::get_fieldTypes() const {
    std::vector<llvm::Type *> fs;
    if (base) {
        const auto b_fs = base->get_fieldTypes();
        fs.insert(cbegin(fs), cbegin(b_fs), cend(b_fs));
    }
    for (auto const &name : index) {
        const auto res = TypeTable::sgl()->resolve(fields.find(name)->second->get_name());
        fs.push_back(res->get_llvm());
    }
    if (fs.empty()) {
        // Empty records will crash LLVM
        fs.push_back(TypeTable::BoolType->get_llvm());
    }
    return fs;
}

llvm::Type *RecordType::get_llvm() const {
    if (cache) {
        return cache;
    }
    const auto fs = get_fieldTypes();
    if (identified.empty()) {
        cache = llvm::StructType::create(fs);
    } else {
        cache = llvm::StructType::create(fs, identified);
    }
    return cache;
}

std::vector<llvm::Constant *> RecordType::get_fieldInit() const {
    std::vector<llvm::Constant *> fs;
    if (base) {
        const auto b_fs = base->get_fieldInit();
        fs.insert(cbegin(fs), cbegin(b_fs), cend(b_fs));
    }
    for (auto const &name : index) {
        const auto res = TypeTable::sgl()->resolve(fields.find(name)->second->get_name());
        fs.push_back(res->get_init());
    }
    if (fs.empty()) {
        fs.push_back(TypeTable::BoolType->get_init());
    }
    return fs;
}

llvm::Constant *RecordType::get_init() const {
    const auto fs = get_fieldInit();
    return llvm::ConstantStruct::get(llvm::dyn_cast<llvm::StructType>(get_llvm()), fs);
}

void RecordType::insert(std::string const &field, Type type) {
    fields[field] = std::move(type);
    index.push_back(field);
}

bool RecordType::has_field(std::string const &field) const {
    if (fields.contains(field)) {
        return true;
    }
    if (base) {
        return base->has_field(field);
    }
    return false;
}

size_t RecordType::get_size() const {
    return std::accumulate(cbegin(fields), cend(fields), cbegin(fields)->second->get_size(),
                           [](size_t x, auto &y) { return x + y.second->get_size(); });
}

std::optional<Type> RecordType::get_type(std::string const &field) {
    auto res = fields.find(field);
    if (res != fields.end()) {
        return std::make_optional<Type>(res->second);
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
    const auto it = std::ranges::find(std::as_const(index), field);
    if (it == end(index)) {
        return -1;
    }
    return base_count + static_cast<int>(std::distance(cbegin(index), it));
}

bool RecordType::is_base(Type const &t) const {
    if (!t || !base) {
        return false;
    }
    if (t->id != TypeId::RECORD) {
        return false;
    }
    return t->equiv(base);
}

bool RecordType::equiv(std::shared_ptr<RecordType> const &r) {

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

} // namespace ax
