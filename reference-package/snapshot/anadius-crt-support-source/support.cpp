#include "support.hpp"

namespace anadius::crt_support_recovered {
std::uint64_t* printf_options_0067e0(State& state) noexcept{return &state.printf_options_0a37d0;}
std::uint64_t* scanf_options_0067f0(State& state) noexcept{return &state.scanf_options_0a37d8;}
void configure_stdio_04c1fc(State& state) noexcept {
    *printf_options_0067e0(state)|=std::uint64_t{0x24};
    *scanf_options_0067f0(state)|=std::uint64_t{2};
}
const Address* tls_callback_slot_04c218(const State& state) noexcept{return &state.tls_callback_0a5a70;}
void initialize_slist_04c1e0(Head head,Effects& effects){effects.initialize_slist_head(head);}
void release_list_04d2f8(Head head,Effects& effects) {
    auto node=effects.interlocked_flush_slist(head);
    while(node!=0) {
        const auto next=effects.next(node);
        effects.free_05caec(node);
        node=next;
    }
}
void release_slist_04c1f0(Head head,Effects& effects){release_list_04d2f8(head,effects);}
}
