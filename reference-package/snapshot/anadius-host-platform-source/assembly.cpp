#include "assembly.hpp"
#include "source_stop.hpp"
#include <stdexcept>

namespace anadius::host_platform {
namespace {
struct Lifetime {
    // Reverse destruction keeps code through callback and milestone teardown.
    std::shared_ptr<void> code;
    std::shared_ptr<original_provider::Publication> originals;
    std::shared_ptr<startup_publication::Milestones> milestones;
    std::shared_ptr<foreign_callback::Domain> callbacks;
};
}
Assembly install_host(bootstrap_backend::Resources& r,AssemblyInputs& in){
    if(in.consumed_)throw std::logic_error("Host assembly inputs already consumed");
    if(r.host||r.hook_cleanup||r.callback_attachment||r.startup_publication)
        throw std::invalid_argument("Host, hook cleanup, callback attachment and publication slots must be empty");
    if(!bootstrap_backend::lifetime_configuration_valid(r))throw std::invalid_argument("Selected source lifetime configuration required before host assembly");
    if(in.host.original_publication)throw std::invalid_argument("Host original publication slot must be empty before matched assembly");
    const auto& p=in.callbacks;const auto& h=in.host;const auto& c=p.crt;
    const bool original=in.closure_profile==native_context::ClosureProfile::OriginalCrt;
    if(original!=(r.lifetime_profile==bootstrap_backend::LifetimeProfile::OriginalCrt))
        throw std::invalid_argument("Host closure and owning Resources lifetime profiles must match");
    if(!in.domain||in.domain->phase()!=thread_domain::Phase::Ready||!in.stream||!in.engine||!in.code_lifetime||!in.seed||!in.permanent_errors||
       !native_context::closure_configuration_valid(in.closure_profile,in.external_quiescence)||
       (original?bool(in.providers_ready):!in.providers_ready))
        throw std::invalid_argument("Ready Domain, managed stream, owned engine/code, callback seed/errors and selected closure settings required");
    if(!p.memory||p.memory->domain_identity()!=in.domain.get()||(p.stream&&p.stream.get()!=static_cast<std::ostream*>(in.stream.get()))||p.originals||p.callback_scope||p.original_scope||p.connect_wait_scope||p.connect_body_scope)
        throw std::invalid_argument("Exact Domain memory/shared stream and unbound original/admission policies required");
    if(!p.encoding||!p.errors||!p.exit||(!p.owned_crt&&(!c.compare_a||!c.copy_a||!c.compare_w||!c.copy_w))||!h.encoding||!h.errors||!h.api.condition||!h.api.verify||!h.api.module||!h.api.procedure)
        throw std::invalid_argument("Complete callback and host SDK/encoding/CRT providers required");
    in.consumed_=true;
    auto lifetime=std::make_shared<Lifetime>();lifetime->code=std::move(in.code_lifetime);
    lifetime->milestones=std::make_shared<startup_publication::Milestones>();
    std::function<bool()> readiness;
    if(!original)readiness=startup_publication::readiness_proof(lifetime->milestones,std::move(in.providers_ready));
    in.providers_ready={};
    lifetime->originals=std::make_shared<original_provider::Publication>(lifetime->code);
    auto original_proof=[publication=lifetime->originals]{
        const auto state=publication->snapshot();
        return (state.phase==original_provider::PublicationPhase::Setup||state.phase==original_provider::PublicationPhase::Ready)&&
            (state.published_mask&(1u<<host_init_recovered::slot(native_recovered::Callback::connect_00fd00)));
    };
    auto web_proof=[milestones=lifetime->milestones](std::uint16_t requested){
        const auto port=milestones->server_port();return port&&*port!=requested;
    };
    auto other_proof=[milestones=lifetime->milestones](std::uint16_t requested){
        const auto port=milestones->server_port();return port&&*port!=requested;
    };
    auto redirect_proof=[milestones=lifetime->milestones]{return milestones->server_port();};
    if(original){
        foreign_callback::HookPublicationProof hook=[publication=lifetime->originals](native_recovered::Callback callback){
            const auto state=publication->snapshot();const auto index=host_init_recovered::slot(callback);
            return index<host_init_recovered::callback_count&&(state.phase==original_provider::PublicationPhase::Setup||state.phase==original_provider::PublicationPhase::Ready)&&
                (state.published_mask&(1u<<index));
        };
        lifetime->callbacks=std::make_shared<foreign_callback::Domain>(std::move(in.domain),std::function<bool()>{},std::function<bool()>{},
            std::function<bool(std::uint16_t)>{},std::function<bool(std::uint16_t)>{},std::function<std::optional<std::uint16_t>()>{},std::move(hook));
    }else lifetime->callbacks=std::make_shared<foreign_callback::Domain>(std::move(in.domain),std::move(readiness),std::move(original_proof),std::move(web_proof),std::move(other_proof),std::move(redirect_proof));
    in.callbacks.stream=in.stream;
    in.callbacks.originals=original_provider::make_publishing_factory(lifetime->originals);
    in.host.original_publication=lifetime->originals;
    auto attachment=foreign_callback::make_factory(lifetime->callbacks,std::move(*in.seed),std::move(in.callbacks),*in.permanent_errors,std::move(in.external_quiescence),in.closure_profile);
    in.seed.reset();in.external_quiescence={};in.callbacks.originals={};in.callbacks.callback_scope={};in.callbacks.original_scope={};in.callbacks.connect_wait_scope={};in.callbacks.connect_body_scope={};
    in.callbacks.stream.reset();in.callbacks.memory.reset();in.callbacks.owned_crt.reset();
    auto resolver=std::make_shared<native_recovered::Resolver>();
    auto domain=std::make_shared<Domain>(std::move(in.stream),std::move(in.engine),std::move(resolver),lifetime,std::move(in.host));
    auto host=std::make_unique<HostPlatform>(domain);
    auto cleanup=std::make_unique<HookCleanup>(std::move(domain));
    bootstrap_backend::CallbackAttachmentFactory matched=[milestones=lifetime->milestones,attachment=std::move(attachment)](bootstrap_backend::CallbackStateView view){
        if(!milestones->matches_execution(&view.execution))throw std::logic_error("Host callback Context requires exact publishing Backend Execution");
        return attachment(view);
    };
    // Seal new callback admission before the caller's source cancellation/drain.
    // Existing callbacks may still finish; no provider is retired by this gate.
    BoundSourceStop stop;
    if(r.stop_sources)stop=bind_source_stop_with_receipt(lifetime->callbacks,r.stop_sources);
    // Everything above is passive. Commit only complete owning providers.
    r.stop_sources=std::move(stop.stop);
    r.host=std::move(host);r.hook_cleanup=std::move(cleanup);
    r.startup_publication=lifetime->milestones;r.callback_attachment=std::move(matched);
    return {lifetime->milestones,lifetime->callbacks,std::move(stop.prerequisite)};
}
}
