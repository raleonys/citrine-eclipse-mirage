#pragma once
// Private production type identity shared across response preparation and
// send-completion. This header is deliberately NOT an exported public ABI.
// No alternative transaction implementation or test-only production branches.
#include "socket_exchange_internal.hpp"
#include "socket_exchange_write_completion.hpp"
#include "preloader_l/handlers/achievement_routes.hpp"
#include "preloader_l/handlers/integration_routes.hpp"
#include "preloader_l/handlers/social_mutation_routes.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <stddef.h>
#include <stdint.h>
#if defined(_WIN32)
extern "C" {
__declspec(dllimport) void* __stdcall GetProcessHeap();
__declspec(dllimport) void* __stdcall HeapAlloc(void*, unsigned long, size_t);
__declspec(dllimport) int __stdcall HeapFree(void*, unsigned long, void*);
}
#endif
namespace preloader_l { namespace protocol { namespace detail {
namespace transaction_owner_internal {
#if defined(_WIN32)
inline void* transient_heap_alloc(void* context,size_t n) noexcept {
    return context ? HeapAlloc(context,0,n) : nullptr;
}
inline int transient_heap_free(void* context,void* p) noexcept {
    return context && HeapFree(context,0,p) ? 1 : 0;
}
#endif
struct HeapBytes {
    void* heap;
    char* data;
    size_t capacity;
    HeapBytes() noexcept : heap(nullptr), data(nullptr), capacity(0) {}
    ~HeapBytes() noexcept {
#if defined(_WIN32)
        if (data) (void)preloader_l::runtime::release_transient_bytes(data);
#endif
    }
    bool allocate(size_t n) noexcept {
#if defined(_WIN32)
        heap = GetProcessHeap();
        if (!heap || n == 0) return false;
        const preloader_l::runtime::AllocatorPlatformOps ops{heap,&transient_heap_alloc,&transient_heap_free,nullptr};
        data = static_cast<char*>(preloader_l::runtime::allocate_transient_bytes(n,ops));
        if (!data) return false;
        capacity = n;
        return true;
#else
        (void)n;
        return false;
#endif
    }
};

struct QueuedEventSocketDelivery {
    preloader_l::runtime::QueuedEventDelivery queue{nullptr,nullptr,false};
    ~QueuedEventSocketDelivery() noexcept { rollback(); }
    bool begin(preloader_l::runtime::PreparedQueuedEvent* candidate,char* output,size_t capacity,size_t* written) noexcept {
        return preloader_l::runtime::begin_queued_event_delivery(candidate,output,capacity,written,&queue);
    }
    void commit() noexcept { preloader_l::runtime::commit_queued_event_delivery(&queue); }
    void retry() noexcept { preloader_l::runtime::resolve_queued_event_delivery_for_retry(&queue); }
    void rollback() noexcept { preloader_l::runtime::rollback_queued_event_delivery(&queue); }
};

struct GrantSocketDelivery {
    preloader_l::handlers::GrantAchievementDelivery grant;
    preloader_l::runtime::QueuedEventDelivery queue;
    bool active;
    size_t response_bytes;
    size_t event_bytes;
    GrantSocketDelivery() noexcept : grant{nullptr,nullptr,false},queue{nullptr,nullptr,false},active(false),response_bytes(0u),event_bytes(0u) {}
    ~GrantSocketDelivery() noexcept { rollback(); }

    bool render(const char* xml,size_t xml_n,char* output,size_t capacity,size_t* written) noexcept {
        if(written)*written=0;
        if(!written||!xml||!output||capacity==0u)return false;
        preloader_l::xml::RequestView request{};
        if(preloader_l::xml::parse_request_view(xml,xml_n,&request)!=preloader_l::xml::RequestStatus::ok)return false;
        size_t base=0;
        const auto hs=preloader_l::handlers::prepare_grant_achievement_delivery(request,output,capacity,&base,&grant);
        if(hs!=preloader_l::handlers::HandlerStatus::ok||!grant.active||base>capacity){rollback();return false;}
        active=true;
        auto* candidate=preloader_l::handlers::take_grant_achievement_delivery_event(&grant);
        size_t event_n=0;
        if(!preloader_l::runtime::begin_queued_event_delivery(candidate,output+base,capacity-base,&event_n,&queue)){
            rollback();return false;
        }
        if(event_n>capacity-base){rollback();return false;}
        response_bytes=base;
        event_bytes=event_n;
        *written=base+event_n;
        return response_bytes!=0u;
    }

    void commit_response() noexcept {
        if(!active)return;
        // B: the *complete local Response frame*, never Event delivery, is
        // the GrantAchievement store commit boundary. Retain FIFO ownership
        // for the independent Event resolution below.
        preloader_l::handlers::commit_grant_achievement_delivery_store(&grant);
        preloader_l::handlers::finish_grant_achievement_delivery(&grant);
        active=false;
    }
    void commit_event() noexcept {
        preloader_l::runtime::commit_queued_event_delivery(&queue);
    }
    void retry_event() noexcept {
        preloader_l::runtime::resolve_queued_event_delivery_for_retry(&queue);
    }
    void rollback() noexcept {
        if(!active&& !grant.active && !queue.active)return;
        preloader_l::runtime::rollback_queued_event_delivery(&queue);
        preloader_l::handlers::rollback_grant_achievement_delivery(&grant);
        active=false;
    }
};

// W06 production transaction owner. A completed encoded response is the
// *local* acceptance boundary; no remote friend/invite delivery is asserted.
// The model is reserved before send, committed after send_all, and rolled
// back for measure/render/encode/write failures. A successful wire send with
// failed local commit remains explicitly indeterminate (never exactly-once).
struct LocalSocialSocketDelivery {
    preloader_l::runtime::LocalSocialPending pending{};
    preloader_l::runtime::LocalSocialSession session{};
    bool active{false};
    ~LocalSocialSocketDelivery() noexcept { rollback(); }
    bool begin(const preloader_l::xml::RequestView& request,
               const preloader_l::runtime::LocalSocialSession* owner) noexcept {
        if(!owner)return false;
        session=*owner;
        if(!preloader_l::handlers::prepare_local_social_socket_request(request,session,&pending))return false;
        active=pending.active;
        return active;
    }
    bool commit() noexcept {
        if(!active)return true;
        const auto result=preloader_l::runtime::commit_local_social_mutation(session,&pending);
        if(result!=preloader_l::runtime::LocalSocialResult::ok)return false;
        active=false;return true;
    }
    void rollback() noexcept {
        if(active)preloader_l::runtime::rollback_local_social_mutation(&pending);
        active=false;
    }
};
 } // namespace transaction_owner_internal
using transaction_owner_internal::HeapBytes;
using transaction_owner_internal::QueuedEventSocketDelivery;
using transaction_owner_internal::GrantSocketDelivery;
using transaction_owner_internal::LocalSocialSocketDelivery;
// Stage04A: the reservations, encoded frames, and transaction decisions live
// together beyond the initiating stack frame. This opaque owner can be kept
// alive by an Asio async_write callback without exposing queue internals or
// duplicating dispatcher semantics. Shipping WinSock uses the same owner.
struct PreparedResponseTransaction {
    HeapBytes plain;
    HeapBytes wire;
    GrantSocketDelivery grant_delivery;
    QueuedEventSocketDelivery show_igo_delivery;
    QueuedEventSocketDelivery generic_event_delivery;
    LocalSocialSocketDelivery social_delivery;
    DeliveryWriteCursor cursor{};
    DispatchTransactionResult result{ExchangeStatus::ok,preloader_l::core::DispatchStatus::invalid_argument,0u,false};
    size_t response_required{0u};
    size_t event_required{0u};
    bool resolved{false};
};

 } } }
