#include "callbacks.hpp"
namespace anadius::crt_lifecycle_callbacks_recovered {
bool initialize_onexit_tables_06447c(crt_termination_recovered::State& s) {
    (void)crt_onexit_recovered::initialize_onexit_table_0643f8(&s.onexit.crt_0a52a8,s.onexit);
    (void)crt_onexit_recovered::initialize_onexit_table_0643f8(&s.quick_table_a52c0,s.onexit);
    return true;
}
bool uninitialize_multibyte_0644f8(MultibyteEffects& e) {
    const auto before=e.atomic_fetch_add_32(e.current_multibyte_0a59d0(),0xffffffffU);
    if(before==1) {
        const auto candidate=e.current_multibyte_0a59d0();
        const auto initial=e.initial_multibyte_0a1c70();
        if(candidate!=initial) {
            // 064524 uses this second load as its free argument. No third load.
            e.free_0668c0(candidate);
            e.set_current_multibyte_0a59d0(initial);
        }
    }
    return true;
}
}
