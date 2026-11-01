#pragma once
#include "../anadius-crt-onexit-source/onexit.hpp"
#include <cstdint>

namespace anadius::crt_entry_recovered {
// Caller-owned recovered globals. There is no implicit synchronization. Only
// the shipped static-UCRT mode (04bfd8 == 0) and its normal flow are modeled.
struct State {
    // The application's onexit table is the SAME object used by registration,
    // not a copied view. Its owner must outlive this state and every Entry call.
    explicit State(crt_onexit_recovered::Table& application) : onexit_table(application) {}
    std::uint32_t startup_state{};       // 1800a4790: 0, 1, 2
    std::uint32_t attach_count_bits{};   // 1800a47ec: signed comparison, wrapping INC
    std::uint8_t initialized_as_dll{};   // 1800a47a0
    std::uint8_t onexit_initialized{};   // 1800a47a1
    crt_onexit_recovered::Table& onexit_table;       // 1800a47a8
    crt_onexit_recovered::Table quickexit_table{};   // 1800a47c0
};

struct Effects {
    virtual ~Effects() = default;
    virtual void security_init_cookie_04c134() = 0;
    virtual void initialize_cpu_04b26c() = 0;
    // Returned register bits matter: the recovered callers test AL, not EAX.
    virtual std::uint64_t vcrt_initialize_04d55c() = 0;
    virtual std::uint64_t acrt_initialize_064594() = 0;
    virtual std::uint64_t vcrt_uninitialize_04d5a8(bool terminating) = 0;
    virtual void acrt_uninitialize_0645cc(bool terminating) = 0;
    virtual std::uint64_t vcrt_thread_attach_04d584() = 0;
    virtual std::uint64_t acrt_thread_attach_0645a8() = 0;
    virtual void acrt_thread_detach_0645bc() = 0;
    virtual void vcrt_thread_detach_04d598() = 0;
    virtual void initialize_slist_04c1e0() = 0;
    virtual void configure_stdio_04c1fc() = 0;
    // Explicit recovered table ranges. Implementations must bind rebuilt table
    // entries, never execute the preferred-image addresses in this comment.
    virtual std::uint32_t initterm_e_064658() = 0; // 7f6d8..7f710, half-open
    virtual void initterm_064614() = 0;           // 7f5d0..7f6d0, half-open
    virtual std::uint32_t configure_narrow_argv_0638e8(std::uint32_t mode) = 0;
    virtual void initialize_narrow_environment_064054() = 0;
    virtual std::uint32_t c_termination_complete_05cdb8() = 0;
    virtual void cexit_05cd9c() = 0;
    virtual void release_slist_04c1f0() = 0;
    virtual void acrt_late_cleanup_064604(bool terminating) = 0;
    virtual void vcrt_late_cleanup_04d5c4() = 0;
    // Non-null, live callback slot. The validation checks the slot address,
    // then the code reloads its contents. Slot lifetime covers both reads.
    virtual const std::uintptr_t* tls_callback_slot_04c218() = 0;
    virtual std::uint64_t slot_in_nonwritable_image_04b64c(
        const std::uintptr_t* slot) = 0;
    virtual void invoke_tls_callback(std::uintptr_t callback, std::uintptr_t module,
        std::uint32_t reason, std::uintptr_t reserved) = 0;
    // Full EAX bits are preserved for the application's return value.
    virtual std::uint32_t application_dll_main_038870(std::uintptr_t module,
        std::uint32_t reason, std::uintptr_t reserved) = 0;
    [[noreturn]] virtual void fastfail_04bfec(std::uint32_t code) = 0;
};

class Entry {
public:
    Entry(State& state, Effects& effects) : state_(state), effects_(effects) {}
    std::uint32_t entry_04bcfc(std::uintptr_t module, std::uint32_t reason,
        std::uintptr_t reserved);
    std::uint32_t dispatch_04bbc8(std::uintptr_t module, std::uint32_t reason,
        std::uintptr_t reserved);
    std::uint32_t crt_reason_04b9e0(std::uintptr_t module, std::uint32_t reason,
        std::uintptr_t reserved);
    bool process_attach_04ba30(std::uintptr_t module, std::uintptr_t reserved);
    bool process_detach_04bb48(bool terminating);
    bool initialize_crt_04b584(std::uint32_t module_kind);
    bool uninitialize_crt_04b708(bool terminating, bool from_exit);
    bool initialize_onexit_tables_04b5c0(std::uint32_t module_kind);
    bool initialize_onexit_tables_04b488();
    bool thread_attach_04b4a0();
    bool thread_detach_04b4c8();
    bool after_initialize_c_04b454();
    void uninitialize_c_04b540();
    void late_cleanup_04b570();
    static bool acquire_startup_lock_04b418() noexcept { return false; }
    static void release_startup_lock_04b6e4(bool) noexcept {}
private:
    bool attach_count_positive() const noexcept;
    State& state_;
    Effects& effects_;
};
}
