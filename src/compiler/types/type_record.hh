//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <optional>

#include "type.hh"

namespace ax {

class RecordType : public Type_ {
  public:
    RecordType() : Type_(TypeId::RECORD) {};
    ~RecordType() override = default;

    explicit operator std::string() override;

    [[nodiscard]] bool is_assignable() const override { return false; };

    std::vector<llvm::Type *>     get_fieldTypes() const;
    std::vector<llvm::Constant *> get_fieldInit() const;

    llvm::Type     *get_llvm() const override;
    llvm::Constant *get_init() const override;

    void   insert(std::string const &field, Type type);
    bool   has_field(std::string const &field) const;
    size_t count() const { return index.size(); };

    void                        set_baseType(std::shared_ptr<RecordType> const &b) { base = b; };
    std::shared_ptr<RecordType> baseType() { return base; };
    bool                        is_base(Type const &t) const;

    std::optional<Type> get_type(std::string const &field);
    int                 get_index(std::string const &field);

    size_t get_size() const override;

    void        set_identified(std::string const &s) { identified = s; };
    std::string get_identified() { return identified; };

    bool equiv(std::shared_ptr<RecordType> const &r);

  private:
    std::string                 identified{}; // identified records
    std::shared_ptr<RecordType> base{nullptr};
    llvm::StringMap<Type>       fields;
    std::vector<std::string>    index;

    mutable llvm::Type *cache{nullptr};
};

} // namespace ax
