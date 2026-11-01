#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <array>
#include "../anadius-vcrt-bridge-source/bridge.hpp"

namespace anadius::crt_lock_native {
using Address=crt_runtime_recovered::Address;
using LockRef=crt_locks_recovered::LockRef;
struct Api {
    decltype(&::InitializeCriticalSectionAndSpinCount) initialize_spin;
    decltype(&::DeleteCriticalSection) destroy;
    decltype(&::EnterCriticalSection) enter;
    decltype(&::LeaveCriticalSection) leave;
};
const Api& native_api() noexcept;
// Provider-owned, live SDK objects; original image addresses are never storage.
// Init/delete and all lifetime changes require external serialization. After
// init, concurrent enter/leave is allowed while initialized bits remain fixed.
// Caller must invoke recovered uninitialization after all users/callbacks stop.
class Banks final:public vcrt_bridge::LockPlatform,
                  public crt_locks_recovered::InitializeEffects {
public:
    explicit Banks(Api api=native_api());
    ~Banks() override;
    Banks(const Banks&)=delete;
    Banks& operator=(const Banks&)=delete;
    Banks(Banks&&)=delete;
    Banks& operator=(Banks&&)=delete;
    void* storage(LockRef) override;
    void destroy(LockRef) override;
    void enter(LockRef) override;
    void leave(LockRef) override;
    std::uint64_t initialize_and_spin(LockRef,std::uint32_t) override;
    std::uint64_t call_initialize_ex(Address,LockRef,std::uint32_t,std::uint32_t) override;
    std::int32_t init_ex_at(Address,void*,std::uint32_t,std::uint32_t);
    std::int32_t init_spin_at(void*,std::uint32_t);
    bool initialized(LockRef) const;
private:
    struct Cell { CRITICAL_SECTION section{}; bool initialized{}; };
    Cell& cell(LockRef);
    const Cell& cell(LockRef) const;
    Cell& cell_at(void*);
    static void require_fresh(const Cell&);
    static void require_initialized(const Cell&);
    const Api api_;
    std::array<Cell,14> acrt_{};
    std::array<Cell,1> vcrt_{};
};
}
