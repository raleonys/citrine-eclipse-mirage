#include "termination.hpp"
#include <bit>

namespace anadius::crt_termination_recovered {
void termination_lambda_05cb38(Arguments args,State& state,Effects& effects) {
    if(state.completed_a4f18) return;
    state.begun_a4f08.exchange(1,std::memory_order_seq_cst);
    if(args.mode==0) {
        const auto cookie=state.onexit.cookie_0a1580;
        const auto callback=state.encoded_callback_a4f10;
        if(callback!=cookie)
            effects.invoke_termination_callback(
                crt_onexit_recovered::decode_pointer(callback,cookie),0,0,0);
        crt_onexit_recovered::execute_onexit_table_0643b4(
            &state.onexit.crt_0a52a8,state.onexit,effects);
    } else if(args.mode==1) {
        crt_onexit_recovered::execute_onexit_table_0643b4(
            &state.quick_table_a52c0,state.onexit,effects);
    }
    // The lambda rereads mode after callbacks rather than retaining its branch.
    if(args.mode==0)
        effects.initterm_range_064614(0x18007f728,0x18007f748);
    effects.initterm_range_064614(0x18007f750,0x18007f758);
    if(args.return_to_caller==0) {
        state.completed_a4f18=1;
        args.completed_this_call=1;
    }
}
void guarded_call_05cb00(const std::int32_t& lock_id,Arguments args,
    const std::int32_t& unlock_id,State& state,Effects& effects) {
    effects.lock_0625f8(static_cast<std::uint32_t>(lock_id));
    termination_lambda_05cb38(args,state,effects);
    effects.unlock_06264c(static_cast<std::uint32_t>(unlock_id));
}
void managed_exit_05cd24(std::uint32_t status,Effects& effects) {
    Address module=0;
    const auto result=effects.get_module_handle_ex_w(0,u"mscoree.dll",module);
    if(result!=0) {
        const auto callback=effects.get_proc_address(module,"CorExitProcess");
        if(callback) effects.invoke_cor_exit_process(callback,status);
    }
    // Output is read even on failed GetModuleHandleEx; no invented success test.
    if(module) effects.free_library(module);
}
bool use_terminate_process_05cd00(Effects& effects) {
    if(effects.policy_06c110()==1) return false; // full EAX comparison
    return static_cast<std::uint8_t>(effects.policy_06c09c())==0; // AL only
}
[[noreturn]] void terminate_05cccc(std::uint32_t status,Effects& effects) {
    if(use_terminate_process_05cd00(effects))
        effects.terminate_process(effects.get_current_process(),status);
    managed_exit_05cd24(status,effects);
    effects.exit_process(status);
    __builtin_unreachable();
}
void common_exit_05cc04(std::uint32_t status,std::uint32_t mode,
    std::uint32_t return_to_caller,State& state,Effects& effects) {
    if(return_to_caller==0) {
        const auto image=effects.get_module_handle_w_null();
        if(image && effects.read_image_u16(image)==0x5a4d) {
            const auto displacement=std::bit_cast<std::int32_t>(
                effects.read_image_u32(image+0x3c));
            const auto nt=image+static_cast<std::uint64_t>(
                static_cast<std::int64_t>(displacement));
            if(effects.read_image_u32(nt)==0x4550 &&
               effects.read_image_u16(nt+0x18)==0x20b &&
               effects.read_image_u32(nt+0x84)>14 &&
               effects.read_image_u32(nt+0xf8)!=0)
                managed_exit_05cd24(status,effects);
        }
    }
    std::uint8_t completed_this_call=0;
    std::int32_t lock_id=2,unlock_id=2;
    guarded_call_05cb00(lock_id,{mode,return_to_caller,completed_this_call},
        unlock_id,state,effects);
    if(return_to_caller!=0) return;
    terminate_05cccc(status,effects);
}
void cexit_05cd9c(State& state,Effects& effects) {
    common_exit_05cc04(0,0,1,state,effects);
}
std::uint32_t c_termination_complete_05cdb8(const State& state) noexcept {
    return state.begun_a4f08.load(std::memory_order_relaxed);
}
void immediate_exit_05cdac(std::uint32_t status,State& state,Effects& effects) {
    common_exit_05cc04(status,2,0,state,effects);
}
void exit_05cdc0(std::uint32_t status,State& state,Effects& effects) {
    common_exit_05cc04(status,0,0,state,effects);
}
}
