#pragma once
#include "../anadius-crt-lock-native-source/native.hpp"
#include "../anadius-crt-atomic-native-source/atomic.hpp"
#include "../anadius-crt-ptd-data-source/ptd_data.hpp"
namespace anadius::crt_sync_bridge {
// Deliberately abstract: loader/FLS/TLS policy is not manufactured here.
class VcrtLockCalls:public vcrt_api_recovered::Effects {
public:
    explicit VcrtLockCalls(crt_lock_native::Banks&);
    std::int32_t call_initialize_critical_section_ex(vcrt_api_recovered::Address,
        void*,std::uint32_t,std::uint32_t) final;
    std::int32_t initialize_critical_section_and_spin_count(void*,std::uint32_t) final;
private:
    crt_lock_native::Banks& banks_;
};
class AtomicCells {
public:
    virtual ~AtomicCells()=default;
    // Return only an owned, live, aligned LONG cell; never reinterpret a raw
    // byte/global address as a LONG object. Provider validates the identity.
    virtual crt_atomic_native::Cell32& resolve(crt_ptd_data_recovered::Address)=0;
};
// Actual recovered lock entry points + typed native atomic effects.
// Resource/global/locale methods intentionally remain mandatory and abstract.
class PtdSyncEffects:public crt_ptd_data_recovered::Effects {
public:
    PtdSyncEffects(crt_locks_recovered::Effects&,AtomicCells&);
    void lock_0625f8(std::uint32_t) final;
    void unlock_06264c(std::uint32_t) final;
    std::uint32_t atomic_fetch_add_32(crt_ptd_data_recovered::Address,std::uint32_t) final;
private:
    crt_locks_recovered::Effects& locks_;
    AtomicCells& cells_;
};
}
