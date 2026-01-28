//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "type.hh"
#include "type_record.hh"

namespace ax {

class PointerType : public Type_ {
  public:
    explicit PointerType(std::string r) : Type_(TypeId::POINTER), ref_name{std::move(r)} {};
    explicit PointerType(Type const &r) : Type_(TypeId::POINTER), reference{r} {
        ref_name = r->get_name();
    };

    ~PointerType() override = default;

    explicit             operator std::string() override { return '^' + ref_name; };
    [[nodiscard]] size_t get_size() const override { return PTR_SIZE; };

    [[nodiscard]] llvm::Type     *get_llvm() const override;
    [[nodiscard]] llvm::Constant *get_init() const override;

    Type get_reference() { return reference; }
    void set_reference(const Type &r) { reference = r; }

    std::string &get_ref_name() { return ref_name; };

  private:
    constexpr static size_t PTR_SIZE = 8;
    Type                    reference = nullptr;
    std::string             ref_name;
};

inline bool is_ptr_to_record(Type const &t) {
    if (t->id == TypeId::POINTER) {
        if (const auto p = std::dynamic_pointer_cast<PointerType>(t);
            p->get_reference()->id == TypeId::RECORD) {
            return true;
        }
    }
    return false;
}

} // namespace ax
