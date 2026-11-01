#include "preloader_l/runtime/event_queue.hpp"
#include <stddef.h>
#include <stdint.h>
#if defined(_WIN32)
extern "C" {
__declspec(dllimport) void* __stdcall GetProcessHeap();
__declspec(dllimport) void* __stdcall HeapAlloc(void*, unsigned long, size_t);
__declspec(dllimport) int __stdcall HeapFree(void*, unsigned long, void*);
}
#endif
#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_InterlockedCompareExchange)
extern "C" {
__declspec(dllimport) void __stdcall AcquireSRWLockExclusive(void*);
__declspec(dllimport) void __stdcall ReleaseSRWLockExclusive(void*);
}
#endif

namespace preloader_l { namespace runtime {
namespace {
constexpr char k_igo_down_event[]="<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>";
volatile long g_event_lock=0;
#if defined(_MSC_VER)
void* g_event_delivery_lock_word=nullptr;
void* g_outer_dispatch_lock_word=nullptr;
#else
volatile long g_event_delivery_lock=0;
volatile long g_outer_dispatch_lock=0;
#endif

enum class EventNodeState : uint8_t {
    available = 0,
    reserved = 1,
    provisional = 2,
};

struct EventNode {
    EventNode* next;
    size_t size;
    EventNodeState state;
};
EventNode* g_head=nullptr;
EventNode* g_tail=nullptr;
size_t g_count=0;
EventNode* g_pending_free_head=nullptr;
EventNode* g_pending_free_tail=nullptr;
size_t g_pending_free_count=0;

void lock_event() noexcept {
#if defined(_MSC_VER)
    while(_InterlockedCompareExchange(&g_event_lock,1,0)!=0) {}
#else
    while(__atomic_exchange_n(&g_event_lock,1L,__ATOMIC_ACQUIRE)!=0L) {
        while(__atomic_load_n(&g_event_lock,__ATOMIC_RELAXED)!=0L) {}
    }
#endif
}
void unlock_event() noexcept {
#if defined(_MSC_VER)
    (void)_InterlockedCompareExchange(&g_event_lock,0,1);
#else
    __atomic_store_n(&g_event_lock,0L,__ATOMIC_RELEASE);
#endif
}
void lock_event_delivery() noexcept {
#if defined(_MSC_VER)
    AcquireSRWLockExclusive(&g_event_delivery_lock_word);
#else
    while(__atomic_exchange_n(&g_event_delivery_lock,1L,__ATOMIC_ACQUIRE)!=0L) {
        while(__atomic_load_n(&g_event_delivery_lock,__ATOMIC_RELAXED)!=0L) {}
    }
#endif
}
void unlock_event_delivery() noexcept {
#if defined(_MSC_VER)
    ReleaseSRWLockExclusive(&g_event_delivery_lock_word);
#else
    __atomic_store_n(&g_event_delivery_lock,0L,__ATOMIC_RELEASE);
#endif
}
void lock_outer_dispatch() noexcept {
#if defined(_MSC_VER)
    // HOTFIX10/F1602-001: the correctness transaction remains globally
    // serialized, but contenders block in the SRW lock instead of burning CPU
    // in a tight process-global spin across measure/allocation/render.
    AcquireSRWLockExclusive(&g_outer_dispatch_lock_word);
#else
    while(__atomic_exchange_n(&g_outer_dispatch_lock,1L,__ATOMIC_ACQUIRE)!=0L) {
        while(__atomic_load_n(&g_outer_dispatch_lock,__ATOMIC_RELAXED)!=0L) {}
    }
#endif
}
void unlock_outer_dispatch() noexcept {
#if defined(_MSC_VER)
    ReleaseSRWLockExclusive(&g_outer_dispatch_lock_word);
#else
    __atomic_store_n(&g_outer_dispatch_lock,0L,__ATOMIC_RELEASE);
#endif
}
void* heap_alloc(size_t n) noexcept {
#if defined(_WIN32)
    void* heap=GetProcessHeap();return heap?HeapAlloc(heap,0,n):nullptr;
#else
    (void)n;return nullptr;
#endif
}
bool heap_free(void* p) noexcept {
#if defined(_WIN32)
    if (!p) return true;
    void* heap = GetProcessHeap();
    return heap && HeapFree(heap, 0, p) != 0;
#else
    (void)p;
    return true;
#endif
}
void retain_pending_free(EventNode* node) noexcept {
    if (!node) return;
    node->next = nullptr;
    lock_event();
    if (g_pending_free_tail) g_pending_free_tail->next = node;
    else g_pending_free_head = node;
    g_pending_free_tail = node;
    ++g_pending_free_count;
    unlock_event();
}
bool retry_pending_frees() noexcept {
    lock_event();
    EventNode* list = g_pending_free_head;
    g_pending_free_head = g_pending_free_tail = nullptr;
    g_pending_free_count = 0;
    unlock_event();
    bool ok = true;
    while (list) {
        EventNode* next = list->next;
        list->next = nullptr;
        if (!heap_free(list)) {
            retain_pending_free(list);
            ok = false;
        }
        list = next;
    }
    return ok;
}
void release_node(EventNode* node) noexcept {
    if (node && !heap_free(node)) retain_pending_free(node);
}
char* payload(EventNode* n) noexcept { return reinterpret_cast<char*>(n+1); }

EventNode* first_available_locked() noexcept {
    EventNode* node=g_head;
    while(node&&node->state!=EventNodeState::available)node=node->next;
    return node;
}

bool contains_node_locked(EventNode* target) noexcept {
    for(EventNode* node=g_head;node;node=node->next)if(node==target)return true;
    return false;
}

bool unlink_node_locked(EventNode* target) noexcept {
    if(!target)return false;
    EventNode* prev=nullptr;EventNode* node=g_head;
    while(node&&node!=target){prev=node;node=node->next;}
    if(!node)return false;
    if(prev)prev->next=node->next;else g_head=node->next;
    if(g_tail==node)g_tail=prev;
    if(g_count)--g_count;
    node->next=nullptr;
    return true;
}

void append_node_locked(EventNode* node) noexcept {
    if(!node)return;
    node->next=nullptr;
    if(g_tail)g_tail->next=node;else g_head=node;
    g_tail=node;++g_count;
}

bool has_inflight_nodes_locked() noexcept {
    for(EventNode* node=g_head;node;node=node->next)
        if(node->state!=EventNodeState::available)return true;
    return false;
}

bool enqueue_copy(const char* xml,size_t length) noexcept {
    if((!xml&&length)||length>static_cast<size_t>(-1)-sizeof(EventNode)-1u)return false;
    const size_t bytes=sizeof(EventNode)+length+1u;
    auto* node=static_cast<EventNode*>(heap_alloc(bytes));if(!node)return false;
    node->next=nullptr;node->size=length;node->state=EventNodeState::available;char* dst=payload(node);
    for(size_t i=0;i<length;++i)dst[i]=xml[i];
    dst[length]='\0';
    lock_event();append_node_locked(node);unlock_event();
    return true;
}
}

bool publish_show_igo_down_event() noexcept {
    return enqueue_copy(k_igo_down_event,sizeof(k_igo_down_event)-1u);
}
bool prepare_show_igo_down_event(PreparedQueuedEvent** prepared) noexcept {
    return prepare_achievement_event(k_igo_down_event,sizeof(k_igo_down_event)-1u,prepared);
}
bool publish_achievement_event(const char* xml,size_t length) noexcept {
    return enqueue_copy(xml,length);
}
bool prepare_achievement_event(const char* xml,size_t length,PreparedQueuedEvent** prepared) noexcept {
    if(prepared)*prepared=nullptr;
    if(!prepared||(!xml&&length)||length>static_cast<size_t>(-1)-sizeof(EventNode)-1u)return false;
    const size_t bytes=sizeof(EventNode)+length+1u;
    auto* node=static_cast<EventNode*>(heap_alloc(bytes));if(!node)return false;
    node->next=nullptr;node->size=length;node->state=EventNodeState::available;char* dst=payload(node);
    for(size_t i=0;i<length;++i)dst[i]=xml[i];
    dst[length]='\0';
    *prepared=reinterpret_cast<PreparedQueuedEvent*>(node);
    return true;
}
void commit_prepared_achievement_event(PreparedQueuedEvent* prepared) noexcept {
    auto* node=reinterpret_cast<EventNode*>(prepared);if(!node)return;
    node->state=EventNodeState::available;
    lock_event();append_node_locked(node);unlock_event();
}
void release_prepared_achievement_event(PreparedQueuedEvent* prepared) noexcept {
    release_node(reinterpret_cast<EventNode*>(prepared));
}
size_t show_igo_down_event_size() noexcept { return sizeof(k_igo_down_event)-1u; }

size_t queued_event_preview_size(size_t candidate_size) noexcept {
    // F02: this gate now protects only the short queue-state observation. It is
    // never carried across encode/send, so a stalled client cannot serialize
    // unrelated dispatch work behind event delivery.
    lock_event_delivery();
    lock_event();EventNode* head=first_available_locked();const size_t n=head?head->size:0u;unlock_event();
    unlock_event_delivery();
    return n?n:candidate_size;
}

bool drain_one_queued_event(char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    lock_event_delivery();
    lock_event();
    EventNode* node=first_available_locked();
    if(!node){unlock_event();unlock_event_delivery();return true;}
    const size_t n=node->size;
    if(!output||capacity<n+1u){
        // D03: capacity/copy failure is observational only. The persistent
        // FIFO head remains owned by the queue and can be retried later.
        unlock_event();
        unlock_event_delivery();
        return false;
    }
    const char* src=payload(node);for(size_t i=0;i<n;++i)output[i]=src[i];
    output[n]='\0';if(written)*written=n;
    (void)unlink_node_locked(node);
    unlock_event();
    unlock_event_delivery();
    release_node(node);
    return true;
}

bool begin_queued_event_delivery(PreparedQueuedEvent* candidate,char* output,size_t capacity,
                                 size_t* written,QueuedEventDelivery* delivery) noexcept {
    if(written)*written=0;
    if(!written||!delivery||(!output&&capacity))return false;
    *delivery=QueuedEventDelivery{nullptr,nullptr,false};
    auto* candidate_node=reinterpret_cast<EventNode*>(candidate);

    // F02 explicit reservation: candidate publication and selected-node state
    // transition are atomic under the short queue gate, but the gate is released
    // before this function returns. Provisional candidates are invisible to
    // other deliveries until their owning transaction commits.
    lock_event_delivery();
    lock_event();
    if(candidate_node){
        candidate_node->state=EventNodeState::provisional;
        append_node_locked(candidate_node);
    }

    EventNode* selected=first_available_locked();
    if(!selected&&candidate_node)selected=candidate_node;
    if(!selected){
        unlock_event();unlock_event_delivery();return true;
    }

    const size_t n=selected->size;
    if(!output||capacity<n+1u){
        bool removed=false;
        if(candidate_node)removed=unlink_node_locked(candidate_node);
        unlock_event();unlock_event_delivery();
        if(removed)release_node(candidate_node);
        return false;
    }

    const char* src=payload(selected);for(size_t i=0;i<n;++i)output[i]=src[i];
    output[n]='\0';*written=n;
    selected->state=EventNodeState::reserved;
    delivery->selected=selected;
    delivery->candidate=candidate;
    delivery->active=true;
    unlock_event();
    unlock_event_delivery();
    return true;
}

void commit_queued_event_delivery(QueuedEventDelivery* delivery) noexcept {
    if(!delivery||!delivery->active)return;
    auto* selected=static_cast<EventNode*>(delivery->selected);
    auto* candidate=reinterpret_cast<EventNode*>(delivery->candidate);
    EventNode* release_selected=nullptr;

    lock_event_delivery();
    lock_event();
    if(selected&&selected->state==EventNodeState::reserved&&unlink_node_locked(selected))
        release_selected=selected;
    if(candidate&&candidate!=selected&&contains_node_locked(candidate)&&
       candidate->state==EventNodeState::provisional)
        candidate->state=EventNodeState::available;
    unlock_event();
    unlock_event_delivery();

    delivery->active=false;delivery->selected=nullptr;delivery->candidate=nullptr;
    if(release_selected)release_node(release_selected);
}

void resolve_queued_event_delivery_for_retry(QueuedEventDelivery* delivery) noexcept {
    if(!delivery||!delivery->active)return;
    auto* selected=static_cast<EventNode*>(delivery->selected);
    auto* candidate=reinterpret_cast<EventNode*>(delivery->candidate);
    // A Response has already completed. Unlike rollback, neither a selected
    // candidate nor a queued predecessor may be removed. Restore both nodes
    // to available in place, preserving their FIFO insertion order.
    lock_event_delivery();
    lock_event();
    if(selected&&contains_node_locked(selected)&&selected->state==EventNodeState::reserved)
        selected->state=EventNodeState::available;
    if(candidate&&candidate!=selected&&contains_node_locked(candidate)&&
       candidate->state==EventNodeState::provisional)
        candidate->state=EventNodeState::available;
    unlock_event();
    unlock_event_delivery();
    delivery->active=false;delivery->selected=nullptr;delivery->candidate=nullptr;
}

void rollback_queued_event_delivery(QueuedEventDelivery* delivery) noexcept {
    if(!delivery||!delivery->active)return;
    auto* selected=static_cast<EventNode*>(delivery->selected);
    auto* candidate=reinterpret_cast<EventNode*>(delivery->candidate);
    EventNode* release_candidate=nullptr;

    lock_event_delivery();
    lock_event();
    if(candidate&&candidate==selected){
        if(candidate->state==EventNodeState::reserved&&unlink_node_locked(candidate))
            release_candidate=candidate;
    }else{
        if(selected&&contains_node_locked(selected)&&selected->state==EventNodeState::reserved)
            selected->state=EventNodeState::available;
        if(candidate&&contains_node_locked(candidate)&&
           candidate->state==EventNodeState::provisional&&unlink_node_locked(candidate))
            release_candidate=candidate;
    }
    unlock_event();
    unlock_event_delivery();

    delivery->active=false;delivery->selected=nullptr;delivery->candidate=nullptr;
    if(release_candidate)release_node(release_candidate);
}

bool clear_queued_events() noexcept {
    (void)retry_pending_frees();
    lock_event_delivery();
    lock_event();
    // In-flight reservations are owned by live response transactions. Shutdown
    // must first cancel/join those workers so their normal rollback path restores
    // or releases ownership; never free nodes from under an active transaction.
    if(has_inflight_nodes_locked()){
        unlock_event();unlock_event_delivery();return false;
    }
    EventNode* node=g_head;g_head=g_tail=nullptr;g_count=0;
    unlock_event();
    unlock_event_delivery();
    while(node){EventNode* next=node->next;node->next=nullptr;release_node(node);node=next;}
    (void)retry_pending_frees();
    lock_event();const bool clean=g_pending_free_count==0;unlock_event();
    return clean;
}
void clear_queued_events_for_test() noexcept { (void)clear_queued_events(); }
size_t queued_event_count_for_test() noexcept {
    lock_event();const size_t n=g_count;unlock_event();return n;
}
size_t queued_event_pending_free_count_for_test() noexcept {
    lock_event();const size_t n=g_pending_free_count;unlock_event();return n;
}

void begin_outer_dispatch_transaction() noexcept { lock_outer_dispatch(); }
void end_outer_dispatch_transaction() noexcept { unlock_outer_dispatch(); }

} }
