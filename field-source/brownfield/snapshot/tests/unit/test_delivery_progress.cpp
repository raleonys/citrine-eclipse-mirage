#include "preloader_l/net/socket_transport.hpp"
#include "preloader_l/protocol/socket_exchange.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "../../src/protocol/socket_exchange_internal.hpp"
#include <cstdlib>
#include <cstring>

namespace {
struct ClockSend {
    uint64_t now_ms=0;
    unsigned calls=0;
    bool block_forever=true;
};

preloader_l::net::TransportStatus send_clocked(void* opaque,const uint8_t*,size_t n,size_t* sent) noexcept {
    auto* state=static_cast<ClockSend*>(opaque);
    if(sent)*sent=0;
    if(!state||!sent||n==0)return preloader_l::net::TransportStatus::invalid_argument;
    ++state->calls;
    state->now_ms+=1000u;
    if(state->block_forever)return preloader_l::net::TransportStatus::would_block;
    *sent=n;
    return preloader_l::net::TransportStatus::ok;
}
uint64_t clock_ms(void* opaque) noexcept {
    auto* state=static_cast<ClockSend*>(opaque);
    return state?state->now_ms:0u;
}
preloader_l::net::TransportStatus recv_closed(void*,uint8_t*,size_t,size_t* received) noexcept {
    if(received)*received=0;
    return preloader_l::net::TransportStatus::closed;
}
void close_noop(void*) noexcept {}
}

extern "C" void* GetProcessHeap(){return reinterpret_cast<void*>(1);}
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){return std::malloc(n);}
extern "C" int HeapFree(void*,unsigned long,void* p){std::free(p);return 1;}

int main(){
    using namespace preloader_l;

    // F02 send-progress budget: a peer that remains would_block cannot spin
    // forever. The fake monotonic clock advances one second per send attempt.
    ClockSend blocked{};
    net::TransportEndpoint endpoint{
        &blocked,&recv_closed,&send_clocked,&close_noop,&clock_ms,2500u};
    const uint8_t byte=0x41u;
    size_t sent=0;
    if(protocol::detail::send_all(endpoint,&byte,1u,&sent)!=protocol::ExchangeStatus::send_timeout)
        return 1;
    if(sent!=0u||blocked.calls!=3u)return 2;

    // Successful progress resets the idle budget and preserves normal send.
    ClockSend ok{};ok.block_forever=false;
    endpoint=net::TransportEndpoint{&ok,&recv_closed,&send_clocked,&close_noop,&clock_ms,2500u};
    sent=0;
    if(protocol::detail::send_all(endpoint,&byte,1u,&sent)!=protocol::ExchangeStatus::ok||sent!=1u)
        return 3;

    constexpr char event_a[]="<LSX><Event sender=\"EbisuSDK\"><A/></Event></LSX>";
    constexpr char event_b[]="<LSX><Event sender=\"EbisuSDK\"><B/></Event></LSX>";
    constexpr char event_c[]="<LSX><Event sender=\"EbisuSDK\"><C/></Event></LSX>";
    runtime::clear_queued_events_for_test();
    if(!runtime::publish_achievement_event(event_a,sizeof(event_a)-1u)||
       !runtime::publish_achievement_event(event_b,sizeof(event_b)-1u))return 4;

    runtime::PreparedQueuedEvent* candidate=nullptr;
    if(!runtime::prepare_achievement_event(event_c,sizeof(event_c)-1u,&candidate)||!candidate)return 5;
    char out_a[256]{};size_t out_a_n=0;runtime::QueuedEventDelivery first{};
    if(!runtime::begin_queued_event_delivery(candidate,out_a,sizeof(out_a),&out_a_n,&first)||
       !first.active||std::strcmp(out_a,event_a)!=0)return 6;
    candidate=nullptr;

    // The delivery gate is no longer held across send. While A is reserved and
    // C is provisional, another transaction can inspect/reserve B immediately.
    if(runtime::queued_event_preview_size(0u)!=sizeof(event_b)-1u)return 7;
    char out_b[256]{};size_t out_b_n=0;runtime::QueuedEventDelivery second{};
    if(!runtime::begin_queued_event_delivery(nullptr,out_b,sizeof(out_b),&out_b_n,&second)||
       !second.active||std::strcmp(out_b,event_b)!=0)return 8;
    runtime::commit_queued_event_delivery(&second);

    // Rollback restores A in place and discards only the first transaction's
    // provisional C. B was independently committed by the second transaction.
    runtime::rollback_queued_event_delivery(&first);
    if(runtime::queued_event_count_for_test()!=1u)return 9;
    char drained[256]{};size_t drained_n=0;
    if(!runtime::drain_one_queued_event(drained,sizeof(drained),&drained_n)||
       std::strcmp(drained,event_a)!=0)return 10;
    if(runtime::queued_event_count_for_test()!=0u)return 11;

    // Shutdown cleanup refuses to free nodes still owned by an in-flight
    // reservation; normal rollback releases that ownership and makes cleanup
    // retryable/safe.
    if(!runtime::publish_achievement_event(event_a,sizeof(event_a)-1u))return 12;
    runtime::QueuedEventDelivery held{};size_t held_n=0;
    if(!runtime::begin_queued_event_delivery(nullptr,out_a,sizeof(out_a),&held_n,&held)||!held.active)return 13;
    if(runtime::clear_queued_events())return 14;
    runtime::rollback_queued_event_delivery(&held);
    if(!runtime::clear_queued_events())return 15;
    return 0;
}
