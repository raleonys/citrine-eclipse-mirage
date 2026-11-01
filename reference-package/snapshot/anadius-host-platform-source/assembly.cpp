#include "assembly.hpp"
#include "source_stop.hpp"
#include <stdexcept>

namespace anadius::host_platform {
namespace {
struct Lifetime {
    // Reverse destruction keeps code through callback and milestone teardown.
    std::shared_ptr<void> code;
    std::shared_ptr<startup_publication::Milestones> milestones;
    std::shared_ptr<foreign_callback::Domain> callbacks;
};
}
Assembly install_host(bootstrap_backend::Resources& r,AssemblyInputs& in){
    if(in.consumed_)throw std::logic_error("Host assembly inputs already consumed");
    if(r.host||r.hook_cleanup||r.callback_attachment||r.startup_publication)
        throw std::invalid_argument("Host, hook cleanup, callback attachment and publication slots must be empty");
    if(!r.stop_sources)throw std::invalid_argument("Explicit source admission stopper required before host assembly");
    const auto& p=in.callbacks;const auto& h=in.host;const auto& c=p.crt;
    if(!in.domain||in.domain->phase()!=thread_domain::Phase::Ready||!in.stream||!in.engine||!in.code_lifetime||!in.seed||!in.permanent_errors||!in.providers_ready||!in.external_quiescence)
        throw std::invalid_argument("Ready Domain, managed stream, owned engine/code, explicit callback seed/errors and lifecycle proofs required");
    if(!p.memory||p.memory->domain_identity()!=in.domain.get()||(p.stream&&p.stream.get()!=static_cast<std::ostream*>(in.stream.get()))||p.originals||p.callback_scope)
        throw std::invalid_argument("Exact Domain memory/shared stream and unbound original/admission policies required");
    if(!p.encoding||!p.errors||!p.exit||(!p.owned_crt&&(!c.compare_a||!c.copy_a||!c.compare_w||!c.copy_w))||!h.encoding||!h.errors||!h.api.condition||!h.api.verify||!h.api.module||!h.api.procedure)
        throw std::invalid_argument("Complete callback and host SDK/encoding/CRT providers required");
    in.consumed_=true;
    auto lifetime=std::make_shared<Lifetime>();lifetime->code=std::move(in.code_lifetime);
    lifetime->milestones=std::make_shared<startup_publication::Milestones>();
    auto readiness=startup_publication::readiness_proof(lifetime->milestones,std::move(in.providers_ready));
    in.providers_ready={};
    lifetime->callbacks=std::make_shared<foreign_callback::Domain>(std::move(in.domain),std::move(readiness));
    in.callbacks.stream=in.stream;
    in.callbacks.originals=original_provider::make_factory(lifetime->code);
    auto attachment=foreign_callback::make_factory(lifetime->callbacks,std::move(*in.seed),std::move(in.callbacks),*in.permanent_errors,std::move(in.external_quiescence));
    in.seed.reset();in.external_quiescence={};in.callbacks.originals={};in.callbacks.callback_scope={};
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
    auto stop=bind_source_stop(lifetime->callbacks,r.stop_sources);
    // Everything above is passive. Commit only complete owning providers.
    r.stop_sources=std::move(stop);
    r.host=std::move(host);r.hook_cleanup=std::move(cleanup);
    r.startup_publication=lifetime->milestones;r.callback_attachment=std::move(matched);
    return {lifetime->milestones,lifetime->callbacks};
}
}
