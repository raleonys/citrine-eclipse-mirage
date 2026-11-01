#pragma once
#include "../anadius-crt-onexit-source/onexit.hpp"
#include <atomic>
#include <cstdint>
#include <string_view>

namespace anadius::crt_termination_recovered {
using Address=std::uint64_t;
struct State {
    State(crt_onexit_recovered::State& tables,std::uint64_t& encoded_callback)
        : onexit(tables),encoded_callback_a4f10(encoded_callback) {}
    crt_onexit_recovered::State& onexit;
    std::uint64_t& encoded_callback_a4f10;
    std::atomic<std::uint32_t> begun_a4f08{};
    std::uint8_t completed_a4f18{};
    crt_onexit_recovered::Table quick_table_a52c0{};
};
struct Arguments {
    std::uint32_t& mode;
    std::uint32_t& return_to_caller;
    std::uint8_t& completed_this_call;
};
struct Effects:crt_onexit_recovered::Effects {
    // Opaque callback binding only; original preferred VAs are never callable.
    virtual void invoke_termination_callback(Address,std::uint32_t,
        std::uint32_t,std::uint32_t)=0;
    // Metadata selectors for rebuilt, mutable callback spans, half open.
    virtual void initterm_range_064614(Address begin,Address end)=0;
    virtual Address get_module_handle_w_null()=0;
    // Providers enforce live bounds. Reads outside a supported image fail
    // explicitly rather than emulating an original access violation/SEH.
    virtual std::uint16_t read_image_u16(Address)=0;
    virtual std::uint32_t read_image_u32(Address)=0;
    virtual std::int32_t get_module_handle_ex_w(std::uint32_t flags,
        std::u16string_view name,Address& output)=0;
    virtual Address get_proc_address(Address,std::string_view name)=0;
    virtual void invoke_cor_exit_process(Address,std::uint32_t status)=0;
    virtual bool free_library(Address)=0;
    virtual std::uint32_t policy_06c110()=0;
    virtual std::uint32_t policy_06c09c()=0;
    virtual Address get_current_process()=0;
    virtual std::int32_t terminate_process(Address,std::uint32_t status)=0;
    [[noreturn]] virtual void exit_process(std::uint32_t status)=0;
};
// Normal paths only. No synthetic RAII, catch, SEH filter, unwind or finally.
void termination_lambda_05cb38(Arguments,State&,Effects&);
void guarded_call_05cb00(const std::int32_t& lock_id,Arguments,
    const std::int32_t& unlock_id,State&,Effects&);
void managed_exit_05cd24(std::uint32_t status,Effects&);
bool use_terminate_process_05cd00(Effects&);
[[noreturn]] void terminate_05cccc(std::uint32_t status,Effects&);
void common_exit_05cc04(std::uint32_t status,std::uint32_t mode,
    std::uint32_t return_to_caller,State&,Effects&);
void cexit_05cd9c(State&,Effects&);
std::uint32_t c_termination_complete_05cdb8(const State&) noexcept;
void immediate_exit_05cdac(std::uint32_t status,State&,Effects&);
void exit_05cdc0(std::uint32_t status,State&,Effects&);
}
