#pragma once
#include "../anadius-foreign-callback-source/foreign.hpp"
#include <atomic>
#include <functional>

namespace anadius::startup_publication {
// Initialization milestones, NOT worker completion, hook success, health,
// PTD quiescence, join or external provider/stream/fallback proof. No reset or
// reuse for another Runtime. Each event uses the bound Execution identity.
class Milestones final:public execution_recovered::Publication {
public:
    void bind(const void*,execution_recovered::FlagReferences) override;
    void published(execution_recovered::ReadyStage,const void*) noexcept override;
    bool observed_all() const noexcept;
    bool invalid() const noexcept;
    std::uint32_t bits() const noexcept;
    bool matches_execution(const void*) const noexcept;
private:
    std::atomic<const void*> execution_=nullptr;
    std::atomic<std::uint32_t> bits_=0;
    std::atomic<bool> invalid_=false;
};
bootstrap_backend::Resources bind(bootstrap_backend::Resources,std::shared_ptr<Milestones>);
// Narrows the caller proof: all four SOURCE milestones must be published first.
// Caller still proves provider/stream/fallback ownership, hook state and other
// external mutation constraints. Call on controller via ForeignDomain activate;
// milestone arrival never invokes or schedules activation on a worker.
std::function<bool()> readiness_proof(std::shared_ptr<Milestones>,std::function<bool()> remaining_provider_proof);
struct Binding {
    bootstrap_backend::Resources resources;
    std::shared_ptr<Milestones> milestones;
    std::shared_ptr<foreign_callback::Domain> callbacks;
};
// Creates one matched publication/foreign-Context pair. Rejects replacement of
// existing observer/attachment; actual callback view must be the bound Execution.
Binding bind_foreign(bootstrap_backend::Resources,std::shared_ptr<thread_domain::DomainStorage>,
    native_context::CallbackSeed,callback_policy::Resources,native_recovered::Errors& permanent_errors,
    std::function<bool()> remaining_provider_proof,native_context::Quiescence external_quiescence);
} // namespace anadius::startup_publication
