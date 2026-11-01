#include "service_state_internal.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#if defined(_WIN32)
#include "preloader_l/runtime/heap_allocator.hpp"
#endif
#if !defined(_WIN32)
#include <stdlib.h>
#endif
#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedExchange)
extern "C" {
__declspec(dllimport) void __stdcall AcquireSRWLockExclusive(void*);
__declspec(dllimport) void __stdcall ReleaseSRWLockExclusive(void*);
}
#endif

namespace preloader_l { namespace runtime { namespace behavioral_internal {

namespace {
Store g_store{};
volatile long g_lock=0;
#if defined(_MSC_VER)
void* g_grant_delivery_lock_word=nullptr;
#else
volatile long g_grant_delivery_lock=0;
#endif
} // namespace

void replace_active_store(Store* replacement,Store* retired) noexcept {
    if(!replacement||!retired)return;
    *retired=g_store;g_store=*replacement;*replacement=Store{};
}
void clear_active_store() noexcept { clear_store(g_store); }
bool append_active_content(const ContentRecordInput& in) noexcept { return append_content(g_store,in); }
bool append_active_entitlement(const EntitlementRecordInput& in) noexcept { return append_entitlement(g_store,in); }
AchievementRecord* find_active_achievement(ServiceTextView id) noexcept { return find_achievement(g_store,id); }
bool reserve_active_achievement_slot() noexcept {
    return grow_ptrs(reinterpret_cast<void***>(&g_store.achievements),&g_store.achievement_capacity,
                     g_store.achievement_count,g_store.achievement_count+1u);
}
void append_reserved_active_achievement(AchievementRecord* record) noexcept {
    if(record)g_store.achievements[g_store.achievement_count++]=record;
}
size_t active_content_count() noexcept { return g_store.content_count; }
bool active_content_at(size_t index,ContentRecordView* out) noexcept {
    if(!out||index>=g_store.content_count)return false;
    auto& rec=*g_store.content[index];
    *out={view(rec.content_id),view(rec.display_name),view(rec.version),view(rec.state)};
    return true;
}
size_t active_entitlement_count() noexcept { return g_store.entitlement_count; }
bool active_entitlement_at(size_t index,EntitlementRecordView* out) noexcept {
    if(!out||index>=g_store.entitlement_count)return false;
    auto& rec=*g_store.entitlements[index];
    *out={view(rec.item_id),view(rec.group),view(rec.version),view(rec.type),view(rec.entitlement_tag)};
    return true;
}
size_t active_achievement_count() noexcept { return g_store.achievement_count; }
bool active_achievement_at(size_t index,AchievementRecordView* out) noexcept {
    if(!out||index>=g_store.achievement_count)return false;
    export_achievement(*g_store.achievements[index],out);
    return true;
}

void* alloc(size_t bytes) noexcept {
    if(!bytes) return nullptr;
#if defined(_WIN32)
    return allocate_transient_bytes(bytes,windows_allocator_platform_ops());
#else
    return malloc(bytes);
#endif
}
void dealloc(void* p,size_t bytes) noexcept {
    if(!p) return;
#if defined(_WIN32)
    (void)bytes;
    // Audit3 S0277: logical state may clear only after exact physical
    // ownership has either released or moved into the intrusive retry owner.
    (void)release_transient_bytes(p);
#else
    (void)bytes; free(p);
#endif
}
void lock_state() noexcept {
#if defined(_MSC_VER)
    while(_InterlockedCompareExchange(&g_lock,1,0)!=0) {}
#else
    while(__atomic_exchange_n(&g_lock,1L,__ATOMIC_ACQUIRE)!=0L) {
        while(__atomic_load_n(&g_lock,__ATOMIC_RELAXED)!=0L) {}
    }
#endif
}
void unlock_state() noexcept {
#if defined(_MSC_VER)
    (void)_InterlockedExchange(&g_lock,0);
#else
    __atomic_store_n(&g_lock,0L,__ATOMIC_RELEASE);
#endif
}
void lock_grant_delivery() noexcept {
#if defined(_MSC_VER)
    AcquireSRWLockExclusive(&g_grant_delivery_lock_word);
#else
    while(__atomic_exchange_n(&g_grant_delivery_lock,1L,__ATOMIC_ACQUIRE)!=0L) {
        while(__atomic_load_n(&g_grant_delivery_lock,__ATOMIC_RELAXED)!=0L) {}
    }
#endif
}
void unlock_grant_delivery() noexcept {
#if defined(_MSC_VER)
    ReleaseSRWLockExclusive(&g_grant_delivery_lock_word);
#else
    __atomic_store_n(&g_grant_delivery_lock,0L,__ATOMIC_RELEASE);
#endif
}

size_t cstrlen(const char* s) noexcept { size_t n=0;if(s)while(s[n])++n;return n; }
bool same(ServiceTextView a,const char* b) noexcept { const size_t n=cstrlen(b);if(a.size!=n||(!a.data&&a.size))return false;for(size_t i=0;i<n;++i)if(a.data[i]!=b[i])return false;return true; }
bool same2(ServiceTextView a,ServiceTextView b) noexcept { if(a.size!=b.size||(!a.data&&a.size)||(!b.data&&b.size))return false;for(size_t i=0;i<a.size;++i)if(a.data[i]!=b.data[i])return false;return true; }

bool set_text(OwnedText& dst,ServiceTextView src) noexcept {
    if(!src.data&&src.size) return false;
    if(src.size==static_cast<size_t>(-1)) return false;
    const size_t bytes=src.size+1u;
    char* p=static_cast<char*>(alloc(bytes)); if(!p) return false;
    for(size_t i=0;i<src.size;++i)p[i]=src.data[i];
    p[src.size]='\0';
    dst=OwnedText{p,src.size,bytes}; return true;
}
void free_text(OwnedText& v) noexcept { dealloc(v.data,v.bytes); v=OwnedText{}; }
ServiceTextView view(const OwnedText& v) noexcept { return ServiceTextView{v.data?v.data:"",v.size}; }

bool grow_ptrs(void*** raw,size_t* cap,size_t used,size_t need) noexcept {
    if(need<=*cap) return true;
    size_t next=*cap?*cap:8u;
    while(next<need){ const size_t n=next+next/2u+1u; if(n<=next){next=need;break;} next=n; }
    if(next>static_cast<size_t>(-1)/sizeof(void*)) return false;
    const size_t bytes=next*sizeof(void*);
    void** p=static_cast<void**>(alloc(bytes)); if(!p)return false;
    for(size_t i=0;i<used;++i)p[i]=(*raw)[i];
    dealloc(*raw,(*cap)*sizeof(void*)); *raw=p; *cap=next; return true;
}
void free_content(ContentRecord* r) noexcept { if(!r)return;free_text(r->content_id);free_text(r->display_name);free_text(r->version);free_text(r->state);dealloc(r,sizeof(*r)); }
void free_entitlement(EntitlementRecord* r) noexcept { if(!r)return;free_text(r->item_id);free_text(r->group);free_text(r->version);free_text(r->type);free_text(r->entitlement_tag);dealloc(r,sizeof(*r)); }
void free_achievement(AchievementRecord* r) noexcept { if(!r)return;free_text(r->id);free_text(r->name);free_text(r->description);free_text(r->how_to);free_text(r->image_id);free_text(r->grant_date);free_text(r->expiration);dealloc(r,sizeof(*r)); }
void clear_store(Store& s) noexcept {
    for(size_t i=0;i<s.content_count;++i)free_content(s.content[i]);
    dealloc(s.content,s.content_capacity*sizeof(void*));
    for(size_t i=0;i<s.entitlement_count;++i)free_entitlement(s.entitlements[i]);
    dealloc(s.entitlements,s.entitlement_capacity*sizeof(void*));
    for(size_t i=0;i<s.achievement_count;++i)free_achievement(s.achievements[i]);
    dealloc(s.achievements,s.achievement_capacity*sizeof(void*));
    s=Store{};
}

bool append_content(Store& s,const ContentRecordInput& in) noexcept {
    auto* r=static_cast<ContentRecord*>(alloc(sizeof(ContentRecord)));if(!r)return false;*r=ContentRecord{};
    if(!set_text(r->content_id,in.content_id)||!set_text(r->display_name,in.display_name)||!set_text(r->version,in.version)||!set_text(r->state,in.state)){free_content(r);return false;}
    if(!grow_ptrs(reinterpret_cast<void***>(&s.content),&s.content_capacity,s.content_count,s.content_count+1u)){free_content(r);return false;}
    s.content[s.content_count++]=r;return true;
}
bool append_entitlement(Store& s,const EntitlementRecordInput& in) noexcept {
    auto* r=static_cast<EntitlementRecord*>(alloc(sizeof(EntitlementRecord)));if(!r)return false;*r=EntitlementRecord{};
#define SETF(name) if(!set_text(r->name,in.name)){free_entitlement(r);return false;}
    SETF(item_id) SETF(group) SETF(version) SETF(type) SETF(entitlement_tag)
#undef SETF
    if(!grow_ptrs(reinterpret_cast<void***>(&s.entitlements),&s.entitlement_capacity,s.entitlement_count,s.entitlement_count+1u)){free_entitlement(r);return false;}
    s.entitlements[s.entitlement_count++]=r;return true;
}
AchievementRecord* create_achievement(ServiceTextView id,ServiceTextView name,int32_t total) noexcept {
    if(!id.data||!id.size)return nullptr;
    auto* r=static_cast<AchievementRecord*>(alloc(sizeof(AchievementRecord)));if(!r)return nullptr;*r=AchievementRecord{};
    static constexpr char zero_date[]="0000-00-00T00:00:00";
    if(!set_text(r->id,id)||!set_text(r->name,name)||!set_text(r->description,{"",0})||!set_text(r->how_to,{"",0})||!set_text(r->image_id,{"",0})||!set_text(r->grant_date,{zero_date,sizeof(zero_date)-1u})||!set_text(r->expiration,{zero_date,sizeof(zero_date)-1u})){free_achievement(r);return nullptr;}
    r->total=total;r->count=0;r->progress=0;
    return r;
}
bool append_achievement(Store& s,ServiceTextView id,ServiceTextView name,int32_t total) noexcept {
    if(!id.data||!id.size)return true;
    AchievementRecord* r=create_achievement(id,name,total);if(!r)return false;
    if(!grow_ptrs(reinterpret_cast<void***>(&s.achievements),&s.achievement_capacity,s.achievement_count,s.achievement_count+1u)){free_achievement(r);return false;}
    s.achievements[s.achievement_count++]=r;return true;
}

ContentRecord* find_content(Store& s,ServiceTextView id) noexcept { for(size_t i=0;i<s.content_count;++i)if(same2(view(s.content[i]->content_id),id))return s.content[i];return nullptr; }
EntitlementRecord* find_entitlement(Store& s,ServiceTextView id) noexcept { for(size_t i=0;i<s.entitlement_count;++i)if(same2(view(s.entitlements[i]->item_id),id))return s.entitlements[i];return nullptr; }
AchievementRecord* find_achievement(Store& s,ServiceTextView id) noexcept { for(size_t i=0;i<s.achievement_count;++i)if(same2(view(s.achievements[i]->id),id))return s.achievements[i];return nullptr; }
bool replace_text(OwnedText& dst,ServiceTextView src) noexcept { OwnedText fresh{};if(!set_text(fresh,src))return false;free_text(dst);dst=fresh;return true; }
void export_achievement(const AchievementRecord& r,AchievementRecordView* out) noexcept {if(out)*out=AchievementRecordView{view(r.id),view(r.name),view(r.description),view(r.how_to),view(r.image_id),view(r.grant_date),view(r.expiration),r.total,r.count,r.progress};}

} } }
