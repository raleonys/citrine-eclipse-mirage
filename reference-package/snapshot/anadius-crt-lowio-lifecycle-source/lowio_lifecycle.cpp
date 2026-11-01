#include "lowio_lifecycle.hpp"
#include <type_traits>
namespace anadius::crt_lowio_lifecycle_recovered {
bool initialize_lowio_067890(Effects& e) {
    e.lock_0625f8(7);
    // TEST EAX checks all32bits, not AL, and ignores incidental upper RAX bits.
    const bool okay=static_cast<std::uint32_t>(e.ensure_fh_exists_0732a8(0))==0;
    if(okay) {
        e.initialize_inherited_067698();
        e.initialize_standard_067788();
    }
    e.unlock_06264c(7);
    return okay;
}
bool uninitialize_lowio_0678cc(State& state,Effects& e) {
    for(std::size_t i=0;i<table_slots;++i) {
        const auto current=state.arrays_0a54a0[i];
        if(current!=0) {
            e.destroy_handle_array_073258(current);
            // Retain the post-call zero store even if effects changed this slot.
            state.arrays_0a54a0[i]=0;
        }
    }
    // The original leaves the separate capacity DWORD unchanged.
    return true;
}
void destroy_handle_array_073258(Address array,Effects& e) {
    if(array==0)return;
    for(std::size_t i=0;i<records_per_array;++i)
        e.delete_critical_section(array+i*record_stride);
    e.free_0668c0(array);
}
void ArrayEffects::destroy_handle_array_073258(Address array) {
    crt_lowio_lifecycle_recovered::destroy_handle_array_073258(array,*this);
}
static_assert(records_per_array*record_stride==array_bytes);
static_assert(std::is_abstract_v<ArrayEffects>);
}
