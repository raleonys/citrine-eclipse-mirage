#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace anadius::crt_ptd_data_recovered {
using Address = std::uintptr_t;
// Relocated identities supplied by the rebuilt image, not original VAs.
struct Identities {
    Address exception_table; // original18008a930
    Address multibyte_info;  // original1800a1c70
    Address initial_locale;  // original1800a17e0
};
// Borrowed live writable PTD storage, at least0x3c8 bytes. All accesses are
// bytewise; no invented __acrt_ptd or __crt_locale_data alias is dereferenced.
class View {
public:
    explicit View(void* storage) : storage_(static_cast<std::byte*>(storage)) {}
    template<class T> T load(std::size_t offset) const {
        T value; std::memcpy(&value, storage_+offset, sizeof value); return value;
    }
    template<class T> void store(std::size_t offset, T value) const {
        std::memcpy(storage_+offset, &value, sizeof value);
    }
private:
    std::byte* storage_;
};
class Effects {
public:
    virtual ~Effects() = default;
    virtual void lock_0625f8(std::uint32_t id) = 0;
    virtual void unlock_06264c(std::uint32_t id) = 0;
    virtual std::uint32_t atomic_fetch_add_32(Address, std::uint32_t increment) = 0;
    virtual void free_0668c0(Address) = 0;
    virtual Address current_locale_0a52e0() = 0;
    virtual void release_locale_ref_0708c0(Address) = 0;
    virtual std::uint32_t locale_ref_count_10(Address) = 0;
    virtual void free_locale_0706c0(Address) = 0;
    virtual void add_locale_ref_070634(Address) = 0;
};
void replace_locale_064ae4(void* ptd, Address replacement, const Identities&, Effects&);
// These four specialized helper bodies preserve normal instruction flow.
// They do not claim MSVC __crt_seh_guarded_call funclet/SEH reconstruction.
void retain_multibyte_0647d4(void*, std::uint32_t lock, std::uint32_t unlock, Effects&);
void clear_locale_064814(void*, std::uint32_t lock, std::uint32_t unlock, const Identities&, Effects&);
void current_locale_064854(void*, std::uint32_t lock, std::uint32_t unlock, const Identities&, Effects&);
void release_multibyte_06489c(void*, std::uint32_t lock, std::uint32_t unlock, const Identities&, Effects&);
void construct_0648fc(void*, const Identities&, Effects&);
void destroy_0649ec(void*, const Identities&, Effects&);
void fls_destroy_0649cc(void*, const Identities&, Effects&);
}
