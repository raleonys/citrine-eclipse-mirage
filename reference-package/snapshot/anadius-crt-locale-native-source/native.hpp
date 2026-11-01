#pragma once
#include "../anadius-crt-locale-resource-bridge-source/bridge.hpp"
#include "../anadius-crt-counter-native-source/counters.hpp"
#include "../anadius-crt-lock-source/locks.hpp"
namespace anadius::crt_locale_native {
// New native provider composition, not original image-global construction.
// Borrowed graph, typed counters, memory and lock banks outlive all callbacks.
// Registry mutation requires whole-registry quiescence, including counters
// outside a locale being freed. Callers synchronize global publication.
class Resources final:public crt_locale_resource_bridge::Resources {
public:
    Resources(crt_locale_bridge::Globals&,crt_locale_bridge::Memory&,
        const crt_locale_ref_recovered::Identities&,
        const crt_locale_free_recovered::Defaults&,
        crt_heap_recovered::State&,crt_heap_recovered::Effects&,
        crt_locks_recovered::Effects&,crt_counter_native::Counters&);
    void lock_0625f8(std::uint32_t) final;
    void unlock_06264c(std::uint32_t) final;
    std::uint32_t atomic_fetch_add_32(crt_ptd_data_recovered::Address,std::uint32_t) final;
private:
    crt_locks_recovered::Effects& locks_;
    crt_counter_native::Counters& counters_;
};
}
