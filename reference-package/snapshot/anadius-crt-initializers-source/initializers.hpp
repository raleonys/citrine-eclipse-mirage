#pragma once
#include <cstdint>
#include <span>
#include "../anadius-crt-ptd-source/ptd.hpp"
#include "../anadius-crt-entry-source/entry.hpp"

namespace anadius::crt_initializers_recovered {
using Address=std::uintptr_t;
struct Pair {Address initialize,uninitialize;};
static_assert(sizeof(Pair)==16);
class Effects {
public:
    virtual ~Effects()=default;
    // Opaque caller-bound rebuilt callback identities. Never call preferred
    // original-image addresses directly. These are distinct signature domains.
    virtual std::uint64_t invoke_boolean_initializer(Address)=0;
    virtual void invoke_uninitializer(Address,std::uint32_t argument)=0;
    virtual void invoke_void_initializer(Address)=0;
    virtual std::uint64_t invoke_int_initializer(Address)=0;
    virtual std::uint32_t common_flush_all(bool)=0;
};
// Span backing storage stays live and fixed in size, but callback effects can
// mutate later entries; each entry is read when reached. Only normal flow is
// reconstructed; no inferred SEH rollback or protected-call policy is added.
bool initialize_pairs_0704f4(std::span<Pair>,Effects&);
bool uninitialize_pairs_070588(std::span<Pair>,Effects&);
void initterm_064614(std::span<Address>,Effects&);
std::uint32_t initterm_e_064658(std::span<Address>,Effects&);
bool acrt_initialize_064594(std::span<Pair>,Effects&);
struct State {std::uint64_t flush_enabled_0a4ef0{};};
bool acrt_uninitialize_0645cc(bool terminating,State&,std::span<Pair>,Effects&);
bool acrt_thread_attach_0645a8(crt_ptd_recovered::State&,crt_ptd_recovered::Effects&);
bool acrt_thread_detach_0645bc(crt_ptd_recovered::State&,crt_ptd_recovered::Effects&);
bool acrt_late_cleanup_064604(crt_ptd_recovered::State&,crt_ptd_recovered::Effects&);

// New source composition. Seven outer-CRT methods use the recovered cores;
// VCRT, CPU, argv/environment, stdio/SList, exit/validation/app policies stay
// mandatory. Spans contain rebuilt bindings, not raw original VA table bytes.
class AcrtTablesBridge:public crt_entry_recovered::Effects {
public:
    AcrtTablesBridge(State&,std::span<Pair>,std::span<Address> c_initializers,
        std::span<Address> cpp_initializers,crt_initializers_recovered::Effects&,
        crt_ptd_recovered::State&,crt_ptd_recovered::Effects&);
    std::uint64_t acrt_initialize_064594() final;
    void acrt_uninitialize_0645cc(bool) final;
    std::uint64_t acrt_thread_attach_0645a8() final;
    void acrt_thread_detach_0645bc() final;
    void acrt_late_cleanup_064604(bool) final;
    std::uint32_t initterm_e_064658() final;
    void initterm_064614() final;
private:
    State& state_;
    std::span<Pair> pairs_;
    std::span<Address> c_,cpp_;
    crt_initializers_recovered::Effects& effects_;
    crt_ptd_recovered::State& ptd_;
    crt_ptd_recovered::Effects& ptd_effects_;
};
}
