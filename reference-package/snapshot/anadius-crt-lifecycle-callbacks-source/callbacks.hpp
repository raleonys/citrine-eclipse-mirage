#pragma once
#include "../anadius-crt-termination-source/termination.hpp"
#include <cstdint>
namespace anadius::crt_lifecycle_callbacks_recovered {
using Address=std::uint64_t;
// References the actual shared normal/quick-exit tables in the termination
// state; does not create shadow copies. Return recovers the low AL boolean.
bool initialize_onexit_tables_06447c(crt_termination_recovered::State&);
struct MultibyteEffects {
    virtual ~MultibyteEffects()=default;
    virtual Address current_multibyte_0a59d0()=0;
    virtual Address initial_multibyte_0a1c70()=0;
    virtual std::uint32_t atomic_fetch_add_32(Address,std::uint32_t)=0;
    virtual void free_0668c0(Address)=0;
    virtual void set_current_multibyte_0a59d0(Address)=0;
};
// Current must identify a live four-byte counter on entry. The source has no
// null/initialized guard and no lock5. Calls require caller's lifecycle ordering.
// Only normal flow/AL is recovered; no additional field resets or SEH policy.
bool uninitialize_multibyte_0644f8(MultibyteEffects&);
}
