#include "publication.hpp"
#include <stdexcept>
#include <type_traits>

namespace anadius::startup_publication {
void Milestones::bind(const void* execution,execution_recovered::FlagReferences flags){
    if(!execution||&flags.first_0a3c3d==&flags.second_0a3c3c||&flags.first_0a3c3d==&flags.third_0a3c3e||&flags.second_0a3c3c==&flags.third_0a3c3e)
        throw std::invalid_argument("Milestones require one Execution and three distinct flag cells");
    const void* expected=nullptr;
    if(!execution_.compare_exchange_strong(expected,execution,std::memory_order_release,std::memory_order_relaxed))
        throw std::logic_error("Milestones cannot be reused by another Execution");
}
void Milestones::published(execution_recovered::ReadyStage stage,const void* issuer) noexcept{
    const auto bound=execution_.load(std::memory_order_acquire);
    if(!bound||issuer!=bound){invalid_.store(true,std::memory_order_release);return;}
    std::uint32_t bit=0;
    switch(stage){
    case execution_recovered::ReadyStage::Server:bit=1;break;
    case execution_recovered::ReadyStage::Online:bit=2;break;
    case execution_recovered::ReadyStage::Libraries:bit=4;break;
    case execution_recovered::ReadyStage::Host:bit=8;break;
    }
    if(!bit){invalid_.store(true,std::memory_order_release);return;}
    bits_.fetch_or(bit,std::memory_order_acq_rel);
}
bool Milestones::observed_all() const noexcept{return bits_.load(std::memory_order_acquire)==15&&!invalid_.load(std::memory_order_acquire);}
bool Milestones::invalid() const noexcept{return invalid_.load(std::memory_order_acquire);}
std::uint32_t Milestones::bits() const noexcept{return bits_.load(std::memory_order_acquire);}
bool Milestones::matches_execution(const void* value) const noexcept{return value&&execution_.load(std::memory_order_acquire)==value;}
bootstrap_backend::Resources bind(bootstrap_backend::Resources resources,std::shared_ptr<Milestones> milestones){
    if(!milestones||resources.startup_publication)throw std::invalid_argument("Explicit unbound startup observer required");
    resources.startup_publication=std::move(milestones);return resources;
}
std::function<bool()> readiness_proof(std::shared_ptr<Milestones> milestones,std::function<bool()> proof){
    if(!milestones||!proof)throw std::invalid_argument("Owned milestones and remaining provider proof required");
    return [milestones=std::move(milestones),proof=std::move(proof)]{return milestones->observed_all()&&proof();};
}
Binding bind_foreign(bootstrap_backend::Resources resources,std::shared_ptr<thread_domain::DomainStorage> threads,
    native_context::CallbackSeed seed,callback_policy::Resources policies,native_recovered::Errors& errors,
    std::function<bool()> proof,native_context::Quiescence quiescence){
    if(resources.startup_publication||resources.callback_attachment)throw std::invalid_argument("Existing startup observer/attachment cannot be replaced");
    auto milestones=std::make_shared<Milestones>();
    auto callbacks=std::make_shared<foreign_callback::Domain>(std::move(threads),readiness_proof(milestones,std::move(proof)));
    auto attachment=foreign_callback::make_factory(callbacks,std::move(seed),std::move(policies),errors,std::move(quiescence));
    resources.startup_publication=milestones;
    resources.callback_attachment=[milestones,attachment=std::move(attachment)](bootstrap_backend::CallbackStateView view){
        if(!milestones->matches_execution(&view.execution))throw std::logic_error("Callback Context and startup publisher must share exact Execution");
        return attachment(view);
    };
    return {std::move(resources),std::move(milestones),std::move(callbacks)};
}
static_assert(!std::is_abstract_v<Milestones>);
} // namespace anadius::startup_publication
