#include "socket_exchange_internal.hpp"
#include "socket_exchange_transaction_owner_private.hpp"
#include "preloader_l/handlers/achievement_routes.hpp"
#include "preloader_l/handlers/integration_routes.hpp"
#include "preloader_l/handlers/social_mutation_routes.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include <stddef.h>
#include <stdint.h>
#include <new>

namespace preloader_l { namespace protocol { namespace detail {
namespace {
bool response_capacity(size_t logical, size_t* capacity) noexcept {
    if (!capacity || logical == static_cast<size_t>(-1)) return false;
    *capacity = logical + 1u;
    return *capacity > logical;
}

struct OuterDispatchTransaction {
    OuterDispatchTransaction() noexcept { preloader_l::runtime::begin_outer_dispatch_transaction(); }
    ~OuterDispatchTransaction() noexcept { preloader_l::runtime::end_outer_dispatch_transaction(); }
};

bool is_local_social_route(preloader_l::router::RouteId id) noexcept {
    using preloader_l::router::RouteId;
    return id==RouteId::block_user || id==RouteId::unblock_user ||
           id==RouteId::request_friend || id==RouteId::remove_friend ||
           id==RouteId::send_invite;
}

preloader_l::core::DispatchResult measure_dispatch(
    const char* xml,
    size_t xml_length,
    const preloader_l::core::DispatchEnvironment* environment) noexcept {
    return environment
        ? preloader_l::core::dispatch_plaintext_request_with_environment(
              xml, xml_length, nullptr, 0, environment)
        : preloader_l::core::dispatch_plaintext_request(xml, xml_length, nullptr, 0);
}

preloader_l::core::DispatchResult render_dispatch(
    const char* xml,
    size_t xml_length,
    char* output,
    size_t capacity,
    const preloader_l::core::DispatchEnvironment* environment) noexcept {
    // F02: shipping socket delivery owns the FIFO reservation. Ask the core
    // dispatcher to render only the route response so no event is destructively
    // drained before the encoded frame is successfully sent.
    preloader_l::core::DispatchEnvironment deferred{nullptr,0u,true};
    if(environment){
        deferred.peer_address=environment->peer_address;
        deferred.peer_address_size=environment->peer_address_size;
        deferred.local_social_session=environment->local_social_session;
    }
    return preloader_l::core::dispatch_plaintext_request_with_environment(
        xml, xml_length, output, capacity, &deferred);
}
}  // namespace

namespace {
#if defined(_WIN32)
bool prepare_encoded_transaction(PreparedResponseTransaction* transaction,
    ExchangeState* s,const char* xml,size_t xml_n,
    const preloader_l::core::DispatchEnvironment* environment) noexcept {
    auto& result=transaction->result;
    HeapBytes& plain = transaction->plain;
    preloader_l::core::DispatchResult dr{};
    GrantSocketDelivery& grant_delivery = transaction->grant_delivery;
    QueuedEventSocketDelivery& show_igo_delivery = transaction->show_igo_delivery;
    QueuedEventSocketDelivery& generic_event_delivery = transaction->generic_event_delivery;
    LocalSocialSocketDelivery& social_delivery = transaction->social_delivery;
    size_t response_plain_n=0u;
    size_t event_plain_n=0u;
    {
        OuterDispatchTransaction dispatch_transaction;
        const auto measured = measure_dispatch(xml, xml_n, environment);
        result.dispatch_status = measured.status;
        if (measured.status != preloader_l::core::DispatchStatus::capacity) return false;
        if (measured.written == 0) {
            result.status = ExchangeStatus::dispatch_error;
            return false;
        }

        size_t plain_capacity = 0;
        if (!response_capacity(measured.written, &plain_capacity)) {
            result.status = ExchangeStatus::resource_error;
            return false;
        }
        if (!plain.allocate(plain_capacity)) {
            result.status = ExchangeStatus::resource_error;
            return false;
        }

        if(is_local_social_route(measured.route)){
            preloader_l::xml::RequestView request{};
            if(preloader_l::xml::parse_request_view(xml,xml_n,&request)!=preloader_l::xml::RequestStatus::ok){
                result.status=ExchangeStatus::dispatch_error;return false;
            }
            // Failure is a negative, fail-closed ErrorSuccess from the normal
            // handler. It must never become a success without local prepare.
            const bool staged=environment && social_delivery.begin(request,environment->local_social_session);
            if(staged){
                size_t n=0;
                const auto hs=preloader_l::handlers::render_local_social_success(
                    request,plain.data,plain.capacity,&n);
                if(hs!=preloader_l::handlers::HandlerStatus::ok||n>plain.capacity){
                    result.status=ExchangeStatus::dispatch_error;return false;
                }
                dr=measured;dr.status=preloader_l::core::DispatchStatus::ok;
                dr.handler_status=hs;dr.written=n;
            }else{
                dr=render_dispatch(xml,xml_n,plain.data,plain.capacity,environment);
                if(dr.status!=preloader_l::core::DispatchStatus::ok||dr.written==0){
                    result.status=ExchangeStatus::dispatch_error;return false;
                }
            }
            const size_t base=dr.written;
            size_t event_n=0;
            if(base>plain.capacity ||
               !generic_event_delivery.begin(nullptr,plain.data+base,plain.capacity-base,&event_n) ||
               event_n>plain.capacity-base){
                result.status=ExchangeStatus::dispatch_error;return false;
            }
            response_plain_n=base;
            event_plain_n=event_n;
            dr.written=base+event_n;
        }else if(measured.route==preloader_l::router::RouteId::grant_achievement){
            size_t delivered_written=0;
            if(!grant_delivery.render(xml,xml_n,plain.data,plain.capacity,&delivered_written)){
                result.status = ExchangeStatus::dispatch_error;
                return false;
            }
            dr=measured;dr.status=preloader_l::core::DispatchStatus::ok;
            dr.handler_status=preloader_l::handlers::HandlerStatus::ok;dr.written=delivered_written;
            response_plain_n=grant_delivery.response_bytes;
            event_plain_n=grant_delivery.event_bytes;
        }else if(measured.route==preloader_l::router::RouteId::show_igo ||
                 measured.route==preloader_l::router::RouteId::show_igo_window){
            preloader_l::xml::RequestView request{};
            if(preloader_l::xml::parse_request_view(xml,xml_n,&request)!=preloader_l::xml::RequestStatus::ok){
                result.status=ExchangeStatus::dispatch_error;
                return false;
            }
            size_t base=0;
            preloader_l::runtime::PreparedQueuedEvent* candidate=nullptr;
            const auto hs=preloader_l::handlers::prepare_show_igo_delivery(
                request,plain.data,plain.capacity,&base,&candidate);
            if(hs!=preloader_l::handlers::HandlerStatus::ok||!candidate||base>plain.capacity){
                result.status=ExchangeStatus::dispatch_error;
                return false;
            }
            size_t event_n=0;
            if(!show_igo_delivery.begin(candidate,plain.data+base,plain.capacity-base,&event_n)){
                result.status=ExchangeStatus::dispatch_error;
                return false;
            }
            if(event_n>plain.capacity-base){
                result.status=ExchangeStatus::dispatch_error;
                return false;
            }
            dr=measured;dr.status=preloader_l::core::DispatchStatus::ok;
            response_plain_n=base;
            event_plain_n=event_n;
            dr.handler_status=preloader_l::handlers::HandlerStatus::ok;dr.written=base+event_n;
        }else{
            dr = render_dispatch(xml, xml_n, plain.data, plain.capacity, environment);
            if (dr.status != preloader_l::core::DispatchStatus::ok || dr.written == 0) {
                result.status = ExchangeStatus::dispatch_error;
                return false;
            }
            const size_t base=dr.written;
            if(base>plain.capacity){
                result.status=ExchangeStatus::dispatch_error;
                return false;
            }
            size_t event_n=0;
            if(!generic_event_delivery.begin(nullptr,plain.data+base,plain.capacity-base,&event_n) ||
               event_n>plain.capacity-base){
                result.status=ExchangeStatus::dispatch_error;
                return false;
            }
            response_plain_n=base;
            event_plain_n=event_n;
            dr.written=base+event_n;
        }
    }

    // B: each logical <LSX> document owns one independent AES/hex+NUL
    // frame. Prepare BOTH ciphertexts before any byte reaches the socket.
    if(response_plain_n==0u||response_plain_n>dr.written||
       event_plain_n!=dr.written-response_plain_n){
        result.status=ExchangeStatus::dispatch_error;return false;
    }
    size_t response_required=0u;
    if(preloader_l::codec::encode_response(s->session_key,16,
          reinterpret_cast<const uint8_t*>(plain.data),response_plain_n,
          nullptr,0,&response_required)!=preloader_l::codec::MessageCodecStatus::capacity||
       response_required==0u){
        result.status=ExchangeStatus::encode_error;return false;
    }
    size_t event_required=0u;
    if(event_plain_n!=0u &&
       (preloader_l::codec::encode_response(s->session_key,16,
          reinterpret_cast<const uint8_t*>(plain.data+response_plain_n),event_plain_n,
          nullptr,0,&event_required)!=preloader_l::codec::MessageCodecStatus::capacity||
        event_required==0u)){
        result.status=ExchangeStatus::encode_error;return false;
    }
    if(response_required>static_cast<size_t>(-1)-event_required){
        result.status=ExchangeStatus::resource_error;return false;
    }
    HeapBytes& wire = transaction->wire;
    if(!wire.allocate(response_required+event_required)){
        result.status=ExchangeStatus::resource_error;return false;
    }
    size_t response_encoded=0u;
    if(preloader_l::codec::encode_response(s->session_key,16,
          reinterpret_cast<const uint8_t*>(plain.data),response_plain_n,
          wire.data,response_required,&response_encoded)!=preloader_l::codec::MessageCodecStatus::ok||
       response_encoded!=response_required){
        result.status=ExchangeStatus::encode_error;return false;
    }
    if(event_plain_n!=0u){
        size_t event_encoded=0u;
        if(preloader_l::codec::encode_response(s->session_key,16,
              reinterpret_cast<const uint8_t*>(plain.data+response_plain_n),event_plain_n,
              wire.data+response_required,event_required,&event_encoded)!=
              preloader_l::codec::MessageCodecStatus::ok||event_encoded!=event_required){
            result.status=ExchangeStatus::encode_error;return false;
        }
    }

    transaction->response_required = response_required;
    transaction->event_required = event_required;
    if (!begin_delivery_writes(&transaction->cursor,response_required,event_required)) {
        result.status=ExchangeStatus::encode_error;
        return false;
    }
    return true;
 }
#endif
} // namespace

PreparedResponseTransaction* prepare_response_transaction(
    ExchangeState* state,const char* xml,size_t xml_length,
    const preloader_l::core::DispatchEnvironment* environment,
    DispatchTransactionResult* outcome) noexcept {
    if (outcome) *outcome=DispatchTransactionResult{
        ExchangeStatus::invalid_argument,preloader_l::core::DispatchStatus::invalid_argument,0u,false};
    if (!outcome||!state||!xml) return nullptr;
#if defined(_WIN32)
    void* heap=GetProcessHeap();
    if (!heap) {outcome->status=ExchangeStatus::resource_error;return nullptr;}
    const preloader_l::runtime::AllocatorPlatformOps ops{
        heap,&transaction_owner_internal::transient_heap_alloc,&transaction_owner_internal::transient_heap_free,nullptr};
    void* storage=preloader_l::runtime::allocate_transient_bytes(
        sizeof(PreparedResponseTransaction),ops);
    if (!storage) {outcome->status=ExchangeStatus::resource_error;return nullptr;}
    auto* transaction=new(storage) PreparedResponseTransaction{};
    const bool valid=prepare_encoded_transaction(transaction,state,xml,xml_length,environment);
    *outcome=transaction->result;
    if (!valid) {destroy_prepared_response_transaction(transaction);return nullptr;}
    return transaction;
#else
    (void)environment;
    (void)xml_length;
    outcome->status=ExchangeStatus::resource_error;
    return nullptr;
#endif
}

DispatchTransactionResult execute_response_transaction(
    preloader_l::net::TransportEndpoint ep,
    ExchangeState* s,
    const char* xml,
    size_t xml_n,
    const preloader_l::core::DispatchEnvironment* environment) noexcept {
    DispatchTransactionResult outcome{};
    auto* transaction=prepare_response_transaction(s,xml,xml_n,environment,&outcome);
    if (!transaction) return outcome;
    PreparedWireFrame wire{};
    if (!prepared_response_frame(transaction,&wire)) {
        outcome.status=ExchangeStatus::encode_error;
        destroy_prepared_response_transaction(transaction);
        return outcome;
    }
    for (;;) {
        size_t sent=0u;
        const auto send_status=send_all(ep,wire.bytes,wire.length,&sent);
        PreparedWireFrame next{};
        const auto action=complete_prepared_transaction_write(
            transaction,send_status==ExchangeStatus::ok,sent,&next,&outcome);
        if (action==PreparedWriteDisposition::send_event) {wire=next;continue;}
        if (send_status!=ExchangeStatus::ok) outcome.status=send_status;
        destroy_prepared_response_transaction(transaction);
        return outcome;
    }
}

} } }
