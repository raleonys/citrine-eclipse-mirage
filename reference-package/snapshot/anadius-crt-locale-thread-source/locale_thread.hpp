#pragma once
#include <cstdint>

namespace anadius::crt_locale_thread_recovered {
using Address = std::uint64_t;
struct Effects {
    virtual ~Effects() = default;
    virtual Address read_address(Address) = 0;
    virtual void write_address(Address, Address) = 0;
    virtual std::uint32_t read_u32(Address) = 0;
    virtual std::uint32_t locale_mask_0a1c6c() = 0;
    virtual Address current_locale_0a52e0() = 0;
    // Identity of the live pointer cell, not a snapshot of its contents.
    virtual Address current_multibyte_slot_0a59d0() = 0;
    virtual Address initial_locale_0a17e0() = 0;
    virtual Address initial_multibyte_0a1c70() = 0;
    virtual Address required_ptd_064b90() = 0;
    virtual void lock_0625f8(std::uint32_t) = 0;
    virtual void unlock_06264c(std::uint32_t) = 0;
    virtual std::uint32_t atomic_fetch_add_32(Address, std::uint32_t) = 0;
    virtual void free_0668c0(Address) = 0;
    virtual void add_locale_ref_070634(Address) = 0;
    virtual void release_locale_ref_0708c0(Address) = 0;
    virtual void free_locale_0706c0(Address) = 0;
    [[noreturn]] virtual void abort_05f03c() = 0;
};
Address update_thread_multibyte_data_internal_06f540(Address ptd, Address current_slot, Effects&);
Address update_thread_multibyte_06f658(Effects&);
Address update_thread_locale_070968(Effects&);
Address update_locale_nolock_0709d8(Address destination_pointer_cell, Address replacement, Effects&);
}
