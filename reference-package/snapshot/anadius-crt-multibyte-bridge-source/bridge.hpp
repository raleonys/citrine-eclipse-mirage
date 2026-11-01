#pragma once
#include "../anadius-crt-multibyte-control-source/multibyte_control.hpp"
#include "../anadius-crt-locale-resource-bridge-source/bridge.hpp"
#include "../anadius-crt-realloc-source/realloc.hpp"
#include "../anadius-crt-invalid-source/invalid.hpp"
namespace anadius::crt_multibyte_bridge {
using Address=std::uint64_t;
struct Globals {
    Address type_table_0a59c0{},case_table_0a59c8{};
    std::uint32_t system_cp_flag_0a59d8{};
    std::uint8_t initialized_0a59dc{};
};
class Memory:public crt_locale_bridge::Memory {
public:
    virtual std::uint8_t read_u8(Address)=0;
    virtual void write_u8(Address,std::uint8_t)=0;
};
class NativeEffects {
public:
    virtual ~NativeEffects()=default;
    virtual std::uint32_t get_acp()=0;
    virtual std::uint32_t get_oemcp()=0;
    // The 06f674 record/table builder is still an unrecovered, mandatory dependency.
    virtual std::uint32_t build_multibyte_06f674(std::int32_t,Address)=0;
    virtual std::uint32_t exchange_32(Address,std::uint32_t)=0;
};
// All globals, live memory, identities, PTD and heap belong to one synchronized
// address domain. No native global initialization or concurrency is supplied.
struct Dependencies {
    crt_locale_bridge::Globals& locale_globals;
    Globals& globals;
    Memory& memory;
    crt_ptd_recovered::State& ptd;
    crt_ptd_recovered::Effects& ptd_fx;
    crt_heap_recovered::State& heap;
    crt_realloc_recovered::Effects& heap_fx;
    crt_errors_recovered::State& errors;
    crt_errors_recovered::Effects& error_fx;
    crt_invalid_recovered::Effects& invalid;
    crt_locale_bridge::ThreadEffects& thread;
    crt_locale_update_recovered::Effects& locale;
    crt_ptd_data_recovered::Effects& resources;
    NativeEffects& native;
    const crt_multibyte_control_recovered::Identities& identities;
};
class Effects final:public crt_multibyte_control_recovered::Effects {
public:
    explicit Effects(Dependencies);
    crt_byteclass_recovered::LocaleContext locale_update_058b04(Address) final;
    Address read_address(Address) final;
    void write_address(Address,Address) final;
    std::uint8_t read_u8(Address) final;
    void write_u8(Address,std::uint8_t) final;
    std::uint32_t read_u32(Address) final;
    void write_u32(Address,std::uint32_t) final;
    std::uint32_t get_acp() final;
    std::uint32_t get_oemcp() final;
    void write_system_cp_flag_0a59d8(std::uint32_t) final;
    Address type_table_0a59c0() final;
    Address case_table_0a59c8() final;
    Address initial_multibyte_0a1c70() final;
    void lock_0625f8(std::uint32_t) final;
    void unlock_06264c(std::uint32_t) final;
    std::uint32_t atomic_fetch_add_32(Address,std::uint32_t) final;
    void free_0668c0(Address) final;
    crt_errors_recovered::Cell32 errno_cell_05d724() final;
    void invalid_parameter_05c954() final;
    std::uint32_t exchange_locale_changed_0a52d8(std::uint32_t) final;
    Address malloc_06a308(std::uint64_t) final;
    std::uint32_t build_multibyte_06f674(std::int32_t,Address) final;
    Address update_thread_multibyte_06f540(Address,Address) final;
    Address required_ptd_raw_064c64() final;
    std::uint8_t initialized_0a59dc() final;
    void set_initialized_0a59dc(std::uint8_t) final;
    void set_type_table_0a59c0(Address) final;
    void set_case_table_0a59c8(Address) final;
    Address current_multibyte_slot_0a59d0() final;
    void set_initial_pair_multibyte_0a1940(Address) final;
    std::uint32_t locale_mask_0a1c6c() final;
private:
    Dependencies d_;
};
class Policy final:public crt_find_bridge::Policy {
public:
    Policy(crt_wildcard_recovered::Effects&,crt_module_path_recovered::Effects&,
        const crt_multibyte_control_recovered::Identities&,crt_multibyte_control_recovered::Effects&);
    void initialize_multibyte_06f5f8() final;
private:
    const crt_multibyte_control_recovered::Identities& identities_;
    crt_multibyte_control_recovered::Effects& effects_;
};
}
