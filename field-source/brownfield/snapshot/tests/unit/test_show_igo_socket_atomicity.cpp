#include "preloader_l/protocol/socket_exchange.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/net/socket_transport.hpp"
#include "../../src/protocol/socket_exchange_internal.hpp"
#include <cstdlib>
#include <cstring>

namespace {
size_t g_alloc_call=0;
size_t g_fail_alloc_call=0;
struct FakeSend { bool fail=false; bool block=false; unsigned calls=0; size_t bytes=0; uint64_t now_ms=0; };

preloader_l::net::TransportStatus send_fn(void* c,const uint8_t*,size_t n,size_t* sent) noexcept {
    auto* s=static_cast<FakeSend*>(c); ++s->calls;
    if(s->fail){ if(sent)*sent=0; return preloader_l::net::TransportStatus::io_error; }
    if(s->block){ s->now_ms+=1000u; if(sent)*sent=0; return preloader_l::net::TransportStatus::would_block; }
    s->bytes+=n; if(sent)*sent=n; return preloader_l::net::TransportStatus::ok;
}
uint64_t clock_fn(void* c) noexcept { auto* s=static_cast<FakeSend*>(c); return s?s->now_ms:0u; }
preloader_l::net::TransportStatus recv_fn(void*,uint8_t*,size_t,size_t* received) noexcept {
    if(received)*received=0; return preloader_l::net::TransportStatus::closed;
}
void close_fn(void*) noexcept {}
}

extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(1); }
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){
    ++g_alloc_call;
    if(g_fail_alloc_call && g_alloc_call==g_fail_alloc_call) return nullptr;
    return std::malloc(n);
}
extern "C" int HeapFree(void*,unsigned long,void* p){ std::free(p); return 1; }

namespace preloader_l { namespace runtime {
bool normalized_avatar_path(char*,size_t,size_t* written) noexcept { if(written)*written=0; return false; }
int query_image_size_bucket(int width) noexcept { return width/208; }
} }

static preloader_l::protocol::detail::DispatchTransactionResult run_tx(FakeSend& io,const char* xml,size_t n){
    preloader_l::protocol::ExchangeState state{};
    preloader_l::net::TransportEndpoint ep{&io,recv_fn,send_fn,close_fn,&clock_fn,2500u};
    return preloader_l::protocol::detail::execute_response_transaction(ep,&state,xml,n,nullptr);
}

int main(){
    using namespace preloader_l;
    constexpr char show[]="<LSX><Request id=\"301\"><ShowIGO/></Request></LSX>";
    constexpr char alias[]="<LSX><Request id=\"302\"><ShowIGOWindow/></Request></LSX>";
    constexpr char older[]="<LSX><Event sender=\"EbisuSDK\"><Older/></Event></LSX>";

    runtime::clear_queued_events_for_test();
    g_alloc_call=0; g_fail_alloc_call=0;
    FakeSend ok{};
    auto tx=run_tx(ok,show,sizeof(show)-1u);
    if(tx.status!=protocol::ExchangeStatus::ok||!tx.response_sent||ok.calls!=2u||runtime::queued_event_count_for_test()!=0u) return 1;

    runtime::clear_queued_events_for_test();
    g_alloc_call=0; g_fail_alloc_call=0;
    FakeSend fail{}; fail.fail=true;
    tx=run_tx(fail,show,sizeof(show)-1u);
    if(tx.status!=protocol::ExchangeStatus::send_error||tx.response_sent||runtime::queued_event_count_for_test()!=0u) return 2;

    runtime::clear_queued_events_for_test();
    if(!runtime::publish_achievement_event(older,sizeof(older)-1u)) return 3;
    g_alloc_call=0; g_fail_alloc_call=0;
    FakeSend ordered{};
    tx=run_tx(ordered,show,sizeof(show)-1u);
    if(tx.status!=protocol::ExchangeStatus::ok||runtime::queued_event_count_for_test()!=1u) return 4;
    char event_out[256]{}; size_t event_n=0;
    if(!runtime::drain_one_queued_event(event_out,sizeof(event_out),&event_n)||!std::strstr(event_out,"<IGOEvent State=\"DOWN\"/>")) return 5;

    runtime::clear_queued_events_for_test();
    if(!runtime::publish_achievement_event(older,sizeof(older)-1u)) return 6;
    g_alloc_call=0; g_fail_alloc_call=0;
    FakeSend ordered_fail{}; ordered_fail.fail=true;
    tx=run_tx(ordered_fail,show,sizeof(show)-1u);
    if(tx.status!=protocol::ExchangeStatus::send_error||runtime::queued_event_count_for_test()!=1u) return 7;
    std::memset(event_out,0,sizeof(event_out)); event_n=0;
    if(!runtime::drain_one_queued_event(event_out,sizeof(event_out),&event_n)||std::strcmp(event_out,older)!=0) return 8;

    // Current owner topology: 1=transaction owner, 2=plaintext buffer,
    // 3=prepared DOWN event node, 4=encoded wire buffer. Verify each
    // fail-closed outcome independently, including zero socket publication.
    runtime::clear_queued_events_for_test();
    g_alloc_call=0; g_fail_alloc_call=1;
    FakeSend owner_fail{};
    tx=run_tx(owner_fail,show,sizeof(show)-1u);
    if(tx.status!=protocol::ExchangeStatus::resource_error||owner_fail.calls!=0u||
       runtime::queued_event_count_for_test()!=0u||g_alloc_call!=1u) return 16;

    runtime::clear_queued_events_for_test();
    g_alloc_call=0; g_fail_alloc_call=2;
    FakeSend plain_fail{};
    tx=run_tx(plain_fail,show,sizeof(show)-1u);
    if(tx.status!=protocol::ExchangeStatus::resource_error||plain_fail.calls!=0u||
       runtime::queued_event_count_for_test()!=0u||g_alloc_call!=2u) return 17;

    runtime::clear_queued_events_for_test();
    g_alloc_call=0; g_fail_alloc_call=3;
    FakeSend alloc_fail{};
    tx=run_tx(alloc_fail,show,sizeof(show)-1u);
    if(tx.status!=protocol::ExchangeStatus::dispatch_error||alloc_fail.calls!=0u||
       runtime::queued_event_count_for_test()!=0u||g_alloc_call!=3u) return 9;

    runtime::clear_queued_events_for_test();
    g_alloc_call=0; g_fail_alloc_call=4;
    FakeSend wire_fail{};
    tx=run_tx(wire_fail,show,sizeof(show)-1u);
    if(tx.status!=protocol::ExchangeStatus::resource_error||wire_fail.calls!=0u||
       runtime::queued_event_count_for_test()!=0u||g_alloc_call!=4u) return 10;

    runtime::clear_queued_events_for_test();
    g_alloc_call=0; g_fail_alloc_call=0;
    FakeSend alias_ok{};
    tx=run_tx(alias_ok,alias,sizeof(alias)-1u);
    if(tx.status!=protocol::ExchangeStatus::ok||!tx.response_sent||runtime::queued_event_count_for_test()!=0u) return 11;

    // F02: a permanently backpressured send hits the monotonic idle budget and
    // rolls the prepared ShowIGO event back exactly like any other send failure.
    runtime::clear_queued_events_for_test();
    g_alloc_call=0; g_fail_alloc_call=0;
    FakeSend stalled{}; stalled.block=true;
    tx=run_tx(stalled,show,sizeof(show)-1u);
    if(tx.status!=protocol::ExchangeStatus::send_timeout||tx.response_sent||
       stalled.calls!=3u||runtime::queued_event_count_for_test()!=0u) return 12;

    runtime::clear_queued_events_for_test();
    if(!runtime::publish_achievement_event(older,sizeof(older)-1u)) return 13;
    g_alloc_call=0; g_fail_alloc_call=0;
    FakeSend stalled_ordered{}; stalled_ordered.block=true;
    tx=run_tx(stalled_ordered,show,sizeof(show)-1u);
    if(tx.status!=protocol::ExchangeStatus::send_timeout||runtime::queued_event_count_for_test()!=1u) return 14;
    std::memset(event_out,0,sizeof(event_out)); event_n=0;
    if(!runtime::drain_one_queued_event(event_out,sizeof(event_out),&event_n)||std::strcmp(event_out,older)!=0) return 15;

    g_fail_alloc_call=0;
    return 0;
}
