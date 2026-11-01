#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "api.hpp"
#include <memory>

namespace recovered_hooks {
enum class OwnedPhase {Fresh,NeedsCleanup,Closed,Poisoned,Retiring};
enum class Operation {Initialize,Create,Enable,Disable,Uninitialize};
struct EngineSnapshot {OwnedPhase phase;bool active,has_result;Operation operation;int status;};
// Explicit owner for one selected MinHook-compatible engine, exclusively used
// through this wrapper. The lower Api and actual code/trampoline backing must
// be valid; nonnull ownership alone is not executable-code provenance proof.
// Lower state MUST be fresh/uninitialized at construction. A newly allocated
// NativeApi object alone does not reset per-image MinHook global state.
// Real NativeApi must statically link the image's own compatibility engine.
// No engine call, patch, implicit cleanup or image pin occurs at construction.
// Controller/outside-loader-lock operations reject overlap/reentry. No mutex
// spans lower calls or owner retirement. Unknown outcomes poison retention.
// BEFORE uninitialize caller must drain/exclude every trampoline/detour user;
// the later empty-engine check does not establish that earlier safety proof.
class OwnedEngine final:public Api {
public:
    OwnedEngine(std::unique_ptr<Api> lower,std::shared_ptr<void> code_owner);
    ~OwnedEngine() noexcept override;
    OwnedEngine(const OwnedEngine&)=delete;OwnedEngine& operator=(const OwnedEngine&)=delete;
    int initialize() override;
    int create(void*,void*,void**) override;
    int enable(void*) override;
    int disable(void*) override;
    int uninitialize() override;
    EngineSnapshot snapshot() const;
    // Fresh is passive; after any engine operation only a normal uninitialize
    // MH_OK/MH_ERROR_NOT_INITIALIZED result proves the selected engine empty.
    // NOT a callback/foreign-source quiescence or image-unload proof.
    bool empty() const;
private:
    struct Impl;std::unique_ptr<Impl> impl_;
    template<class F>int run(Operation,F&&);
};
// Preferred owning construction. Borrowed donors are validated before taking
// them, then kept in ordered local ownership across facade/Impl/control-block
// allocation failure. Valid consumption empties both donors, even on failure.
// Direct/forwarding callers must retain actual code through their own argument
// and outer-allocation failure prefixes; the constructor protects its own body.
std::shared_ptr<OwnedEngine> make_owned_engine(std::unique_ptr<Api>&,std::shared_ptr<void>&);
}
