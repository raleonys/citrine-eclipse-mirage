#pragma once
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace preloader_l::registry_recovered {
// A nullable, non-owning scalar location. A raw ABI argument need not point to
// a C++ object whose type is T (Win32 DWORD can differ from uint32_t, and HKEY
// is a pointer). Access its object representation rather than aliasing it as T.
// The caller owns the storage and must provide sizeof(T) readable/writable
// bytes for each actual load/store. No initialization or deferred writeback.
template<class T>
class ScalarCell {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
    void* address_ = nullptr;
    explicit ScalarCell(void* address, int) noexcept : address_(address) {}
public:
    constexpr ScalarCell() noexcept = default;
    constexpr ScalarCell(std::nullptr_t) noexcept {}
    ScalarCell(T* address) noexcept : address_(address) {}
    static ScalarCell from_raw(void* address) noexcept {
        return ScalarCell(address, 0);
    }
    explicit operator bool() const noexcept { return address_ != nullptr; }
    void* raw() const noexcept { return address_; }
    T load() const {
        if (!address_) throw std::logic_error("load from null ABI scalar cell");
        T value; // Never read until all sizeof(T) bytes have been copied.
        std::memcpy(&value, address_, sizeof value);
        return value;
    }
    void store(T value) const {
        if (!address_) throw std::logic_error("store to null ABI scalar cell");
        std::memcpy(address_, &value, sizeof value);
    }
};
}
