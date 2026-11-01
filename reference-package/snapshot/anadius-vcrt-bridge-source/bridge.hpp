#pragma once
#include "../anadius-crt-initializers-source/initializers.hpp"
#include "../anadius-crt-cpu-source/cpu.hpp"
#include "../anadius-vcrt-api-source/api.hpp"
#include "../anadius-vcrt-ptd-source/vcrt_ptd.hpp"
#include "../anadius-crt-lock-source/locks.hpp"
#include "../anadius-crt-heap-source/heap.hpp"

namespace anadius::vcrt_bridge {
class LockPlatform {
public:
    virtual ~LockPlatform()=default;
    // Must return live, correctly typed provider-owned native CS storage, or
    // an opaque fixture accepted by that same provider. Never an original VA.
    virtual void* storage(crt_locks_recovered::LockRef)=0;
    virtual void destroy(crt_locks_recovered::LockRef)=0;
    virtual void enter(crt_locks_recovered::LockRef)=0;
    virtual void leave(crt_locks_recovered::LockRef)=0;
};
class LockEffects final:public crt_locks_recovered::Effects {
public:
    // InitializeEffects and LockPlatform must map the SAME ACRT bank; the
    // VCRT API provider must accept the VCRT storage from that platform.
    // All providers, state and callback bindings outlive late FLS callbacks.
    LockEffects(crt_runtime_recovered::State&,crt_runtime_recovered::Effects&,
        crt_locks_recovered::InitializeEffects&,
        vcrt_api_recovered::State&,vcrt_api_recovered::Effects&,LockPlatform&);
    std::uint64_t initialize_acrt_06707c(crt_locks_recovered::LockRef,std::uint32_t,std::uint32_t) override;
    std::uint64_t initialize_vcrt_050c38(crt_locks_recovered::LockRef,std::uint32_t,std::uint32_t) override;
    void delete_critical_section(crt_locks_recovered::LockRef) override;
    void enter_critical_section(crt_locks_recovered::LockRef) override;
    void leave_critical_section(crt_locks_recovered::LockRef) override;
private:
    crt_runtime_recovered::State& runtime_;
    crt_runtime_recovered::Effects& runtime_effects_;
    crt_locks_recovered::InitializeEffects& initialize_;
    vcrt_api_recovered::State& api_;
    vcrt_api_recovered::Effects& api_effects_;
    LockPlatform& platform_;
};
class ThreadPolicy {
public:
    virtual ~ThreadPolicy()=default;
    // Same OS/TLS error domain used by the VCRT API provider's last_error().
    virtual void set_last_error(std::uint32_t)=0;
    [[noreturn]] virtual void abort_05f03c()=0;
};
class PtdEffects final:public vcrt_ptd_recovered::Effects {
public:
    PtdEffects(vcrt_api_recovered::State&,vcrt_api_recovered::Effects&,
        vcrt_api_recovered::FlsCallback,
        crt_heap_recovered::State&,crt_heap_recovered::Effects&,
        crt_locks_recovered::State&,crt_locks_recovered::Effects&,ThreadPolicy&);
    std::uint32_t fls_alloc_050b0c(vcrt_ptd_recovered::FlsCallback) override;
    bool fls_free_050b54(std::uint32_t) override;
    void* fls_get_050b9c(std::uint32_t) override;
    bool fls_set_050be4(std::uint32_t,void*) override;
    vcrt_ptd_recovered::Ptd* calloc_05cae4(std::size_t,std::size_t) override;
    void free_05caec(void*) override;
    std::uint32_t last_error() override;
    void set_last_error(std::uint32_t) override;
    bool initialize_locks_05093c() override;
    void uninitialize_locks_050984() override;
    [[noreturn]] void abort_05f03c() override;
private:
    vcrt_api_recovered::State& api_;
    vcrt_api_recovered::Effects& api_effects_;
    const vcrt_api_recovered::FlsCallback callback_;
    crt_heap_recovered::State& heap_;
    crt_heap_recovered::Effects& heap_effects_;
    crt_locks_recovered::State& locks_;
    crt_locks_recovered::Effects& lock_effects_;
    ThreadPolicy& policy_;
};
struct Dependencies {
    vcrt_ptd_recovered::State& ptd;
    vcrt_ptd_recovered::Effects& effects;
    crt_cpu_recovered::State& cpu;
    crt_cpu_recovered::Effects& cpu_effects;
};
// New source composition, not a new recovered function. VCRT and CPU callbacks
// replace six previous outer-CRT boundaries with actual recovered algorithms.
// The remaining external subsystems are still pure and mandatory.
class LifecycleBridge:public crt_initializers_recovered::AcrtTablesBridge {
public:
    LifecycleBridge(crt_initializers_recovered::State&,
        std::span<crt_initializers_recovered::Pair>,
        std::span<crt_initializers_recovered::Address> c,
        std::span<crt_initializers_recovered::Address> cpp,
        crt_initializers_recovered::Effects&,
        crt_ptd_recovered::State&,crt_ptd_recovered::Effects&,Dependencies);
    void initialize_cpu_04b26c() final;
    std::uint64_t vcrt_initialize_04d55c() final;
    std::uint64_t vcrt_uninitialize_04d5a8(bool) final;
    std::uint64_t vcrt_thread_attach_04d584() final;
    void vcrt_thread_detach_04d598() final;
    void vcrt_late_cleanup_04d5c4() final;
private:
    const Dependencies deps_;
};
}
