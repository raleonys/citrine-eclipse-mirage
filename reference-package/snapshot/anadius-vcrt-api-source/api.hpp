#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <span>
#include <string_view>

namespace anadius::vcrt_api_recovered {
using Address = std::uintptr_t;
using Module = std::uintptr_t;
using FlsCallback = void (*)(void*);
inline constexpr Address missing = ~Address{0};
// New source atomic model of original qword MOV/XCHG cells, not original ABI.
struct State {
    std::array<std::atomic<Module>, 3> modules{};    // A4EA8
    std::array<std::atomic<Address>, 5> procedures{}; // A4EC0
    std::array<std::u16string_view, 3> module_names{
        u"api-ms-win-core-fibers-l1-1-1", u"api-ms-win-core-synch-l1-2-0", u"kernel32"};
};
class Effects {
public:
    virtual ~Effects() = default;
    virtual Module load_library_ex_w(std::u16string_view, Module file, std::uint32_t flags) = 0;
    virtual std::uint32_t last_error() = 0;
    virtual bool free_library(Module) = 0;
    virtual Address get_proc_address(Module, std::string_view) = 0;
    // Opaque addresses identify a rebuilt provider, never original preferred VAs.
    virtual std::uint32_t call_fls_alloc(Address, FlsCallback) = 0;
    virtual std::int32_t call_fls_free(Address, std::uint32_t) = 0;
    virtual void* call_fls_get_value(Address, std::uint32_t) = 0;
    virtual std::int32_t call_fls_set_value(Address, std::uint32_t, void*) = 0;
    virtual std::int32_t call_initialize_critical_section_ex(
        Address, void* critical_section, std::uint32_t spin, std::uint32_t flags) = 0;
    virtual std::uint32_t tls_alloc() = 0;
    virtual std::int32_t tls_free(std::uint32_t) = 0;
    virtual void* tls_get_value(std::uint32_t) = 0;
    virtual std::int32_t tls_set_value(std::uint32_t, void*) = 0;
    virtual std::int32_t initialize_critical_section_and_spin_count(void*, std::uint32_t) = 0;
};
// Valid procedure/module indices and immutable, NUL-free names are required.
// Shutdown/unload and mutation of module_names require external quiescence.
Address resolve_0509bc(State&, Effects&, std::uint32_t procedure,
                      std::string_view name, std::span<const std::uint32_t> candidates);
std::uint32_t fls_alloc_050b0c(State&, Effects&, FlsCallback);
std::int32_t fls_free_050b54(State&, Effects&, std::uint32_t);
void* fls_get_value_050b9c(State&, Effects&, std::uint32_t);
std::int32_t fls_set_value_050be4(State&, Effects&, std::uint32_t, void*);
std::int32_t initialize_critical_section_ex_050c38(State&, Effects&, void*, std::uint32_t, std::uint32_t);
} // namespace anadius::vcrt_api_recovered
