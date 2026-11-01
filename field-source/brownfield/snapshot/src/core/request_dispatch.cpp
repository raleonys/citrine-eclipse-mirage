#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/handlers/handler_registry.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/xml/response_builder.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/core/checked_math.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/core/local_social_reader_policy.hpp"
#include "preloader_l/handlers/local_social_list_route.hpp"

namespace preloader_l { namespace core {
namespace {
struct RuntimePolicyReadScope {
    preloader_l::config::RuntimeConfigurationReadLease lease{};
    RuntimePolicyReadScope() noexcept { (void)preloader_l::config::begin_runtime_configuration_read(&lease); }
    ~RuntimePolicyReadScope() noexcept { preloader_l::config::end_runtime_configuration_read(&lease); }
};
constexpr preloader_l::router::RouteId no_route = static_cast<preloader_l::router::RouteId>(0);

bool append_one_event(char* output,size_t capacity,size_t base,size_t expected,size_t* total) noexcept {
    if(!total)return false;
    *total=base;
    if(expected==0)return true;
    if(!output||base>capacity||expected>capacity-base)return false;
    size_t event_n=0;
    const bool ok=preloader_l::runtime::drain_one_queued_event(output+base,capacity-base,&event_n);
    if(!preloader_l::core::checked_add_size(base,ok?event_n:expected,*total))return false;
    return ok;
}
}

DispatchResult dispatch_plaintext_request_with_environment(
    const char* input, size_t length, char* output, size_t capacity,
    const DispatchEnvironment* environment) noexcept
{
    // The current normalized handlers do not yet consume the peer string, but
    // the curio active dispatcher receives it for every request. Preserve
    // that semantic input boundary rather than discarding it in the worker.
    const bool defer_event_delivery = environment && environment->defer_event_delivery;
    DispatchResult result{DispatchStatus::invalid_argument, no_route, preloader_l::handlers::HandlerStatus::invalid_request, 0};
    if ((!input && length != 0) || (!output && capacity != 0)) return result;
    RuntimePolicyReadScope policy_read_scope{};

    preloader_l::xml::RequestView request{};
    const auto parsed = preloader_l::xml::parse_request_view(input, length, &request);
    if (parsed == preloader_l::xml::RequestStatus::incomplete_frame) {
        result.status = DispatchStatus::incomplete_request;
        return result;
    }
    if (parsed != preloader_l::xml::RequestStatus::ok) {
        result.status = DispatchStatus::malformed_request;
        return result;
    }

    const auto* descriptor = preloader_l::router::find_known_route(request.command);
    if (!descriptor) {
        // F1427-001 behavioral convergence: a true router miss is not the behavioral's
        // generic-unimplemented ErrorSuccess path. It produces no normal XML
        // response; the persistent outer session decides to continue.
        result.status = DispatchStatus::unknown_route;
        result.written = 0;
        return result;
    }
    result.route = preloader_l::router::descriptor_route_id(*descriptor);

    // A stateful socket may never report a fixed empty donor list after a
    // committed local mutation. Existing no-session direct/debug behavior is
    // preserved; populated list XML remains gated on Consumer wire evidence.
    preloader_l::runtime::LocalSocialSnapshot social_reader_snapshot{};
    bool render_local_list=false;
    if (environment && requires_committed_social_read(
            environment->local_social_session, result.route)) {
        preloader_l::runtime::LocalSocialSnapshot snapshot{};
        const auto read = preloader_l::runtime::snapshot_local_social(
            *environment->local_social_session, &snapshot);
        const auto decision = local_social_reader_decision(result.route, read, snapshot);
        if(decision==LocalSocialReaderDecision::deny_untrusted_or_stale){
            result.status=DispatchStatus::handler_error;
            result.handler_status=preloader_l::handlers::HandlerStatus::unsupported;
            return result;
        }
        if(decision==LocalSocialReaderDecision::render_populated_local_view){
            social_reader_snapshot=snapshot;
            render_local_list=true;
        }
    }

    const auto handler = preloader_l::handlers::find_bound_handler(result.route);
    if (!handler) {
        result.status = DispatchStatus::unimplemented_route;
        return result;
    }

    size_t response_written=0;
    result.handler_status=render_local_list
        ? preloader_l::handlers::render_local_social_list(
            request,result.route,social_reader_snapshot,output,capacity,&response_written)
        : handler(request,output,capacity,&response_written);

    // Phases101-106: the outer consumer drains at most one FIFO item. During
    // a size probe, predict only the event the real handler will enqueue before
    // that drain. Existing queued items remain ahead of the candidate.
    size_t candidate_event=0;
    if(!defer_event_delivery &&
       result.handler_status==preloader_l::handlers::HandlerStatus::response_error &&
       response_written!=0 && output==nullptr){
        if(result.route==preloader_l::router::RouteId::show_igo ||
           result.route==preloader_l::router::RouteId::show_igo_window)
            candidate_event=preloader_l::runtime::show_igo_down_event_size();
        else if(result.route==preloader_l::router::RouteId::grant_achievement)
            candidate_event=preloader_l::handlers::preview_grant_achievement_event_size(request);
    }
    const size_t event_expected=defer_event_delivery
        ? 0u
        : preloader_l::runtime::queued_event_preview_size(candidate_event);
    size_t total=response_written;
    if(!preloader_l::core::checked_add_size(response_written,event_expected,total)){
        result.written=response_written; result.status=DispatchStatus::handler_error; return result;
    }
    result.written=total;

    if(result.handler_status==preloader_l::handlers::HandlerStatus::ok){
        if(defer_event_delivery){
            result.written=response_written;
            result.status=DispatchStatus::ok;
            return result;
        }
        // The real ShowIGO handler has already enqueued its DOWN event here.
        const size_t actual_expected=preloader_l::runtime::queued_event_preview_size(0u);
        if(!append_one_event(output,capacity,response_written,actual_expected,&total)){
            result.written=total; result.handler_status=preloader_l::handlers::HandlerStatus::response_error; result.status=DispatchStatus::capacity; return result;
        }
        result.written=total; result.status=DispatchStatus::ok;
    } else if(result.handler_status==preloader_l::handlers::HandlerStatus::unsupported) {
        result.status=DispatchStatus::unsupported_route;
    } else if(result.handler_status==preloader_l::handlers::HandlerStatus::no_response_required) {
        result.written=0;
        result.status=DispatchStatus::no_response_required;
    } else if(result.handler_status==preloader_l::handlers::HandlerStatus::response_error &&
              result.written!=0 && result.written>=capacity) {
        result.status=DispatchStatus::capacity;
    } else {
        result.status=DispatchStatus::handler_error;
    }
    return result;
}

DispatchResult dispatch_plaintext_request(
    const char* input, size_t length, char* output, size_t capacity) noexcept
{
    return dispatch_plaintext_request_with_environment(
        input, length, output, capacity, nullptr);
}

} }
