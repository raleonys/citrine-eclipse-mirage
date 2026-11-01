#include "runtime_configuration_internal.hpp"
#if defined(_WIN32)
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#else
#include <stdlib.h>
#endif

namespace preloader_l { namespace config { namespace runtime_configuration_detail {

void* transient_alloc(size_t bytes) noexcept {
    if(!bytes) return nullptr;
#if defined(_WIN32)
    return preloader_l::runtime::allocate_transient_bytes(
        bytes, preloader_l::runtime::windows_allocator_platform_ops());
#else
    return malloc(bytes);
#endif
}

void transient_free(void* ptr,size_t bytes) noexcept {
    if(!ptr) return;
#if defined(_WIN32)
    (void)bytes;
    // Audit3 S0277: release failure transfers physical ownership to the
    // allocator's intrusive pending-owner list before this logical view clears.
    (void)preloader_l::runtime::release_transient_bytes(ptr);
#else
    (void)bytes; free(ptr);
#endif
}

namespace {

bool grow_capacity(size_t current,size_t required,size_t* next) noexcept {
    if(!next) return false;
    size_t value=current?current:8u;
    while(value<required){
        const size_t grown=value+value/2u+1u;
        if(grown<=value){ value=required; break; }
        value=grown;
    }
    *next=value; return value>=required;
}

template<class T>
bool reserve_array(T** data,size_t* capacity,size_t used,size_t required) noexcept {
    if(!data||!capacity) return false;
    if(required<=*capacity) return true;
    size_t next=0;
    if(!grow_capacity(*capacity,required,&next)||next>static_cast<size_t>(-1)/sizeof(T)) return false;
    const size_t bytes=next*sizeof(T);
    auto* fresh=static_cast<T*>(transient_alloc(bytes));
    if(!fresh) return false;
    for(size_t i=0;i<used;++i) fresh[i]=(*data)[i];
    transient_free(*data,*capacity*sizeof(T));
    *data=fresh; *capacity=next; return true;
}

bool reserve_arena(State& st,size_t required) noexcept {
    if(required<=st.arena_capacity) return true;
    size_t next=0;
    if(!grow_capacity(st.arena_capacity,required,&next)) return false;
    auto* fresh=static_cast<char*>(transient_alloc(next));
    if(!fresh) return false;
    for(size_t i=0;i<st.arena_used;++i) fresh[i]=st.arena[i];
    transient_free(st.arena,st.arena_capacity);
    st.arena=fresh; st.arena_capacity=next; return true;
}

bool arena_append(State& st,const char* p,size_t n,size_t* off) noexcept {
    if(n && !p) return false;
    if(st.arena_used>static_cast<size_t>(-1)-n-1u) return false;
    if(!reserve_arena(st,st.arena_used+n+1u)) return false;
    if(off) *off=st.arena_used;
    for(size_t i=0;i<n;++i) st.arena[st.arena_used++]=p[i];
    st.arena[st.arena_used++]=0;
    return true;
}

Setting* find_setting(State& st,const char* key,size_t key_len) noexcept {
    for(size_t i=0;i<st.setting_count;++i){
        auto& s=st.settings[i];
        if(s.used&&s.name_len==key_len&&bytes_equal(st.arena+s.name_off,s.name_len,key,key_len)) return &s;
    }
    return nullptr;
}

} // namespace

void reset_state(State& st) noexcept {
    transient_free(st.settings,st.setting_capacity*sizeof(Setting));
    transient_free(st.arena,st.arena_capacity);
    st=State{};
}

size_t cstr_len(const char* s) noexcept { size_t n=0; if(s) while(s[n]) ++n; return n; }

bool bytes_equal(const char* a,size_t an,const char* b,size_t bn) noexcept {
    if(an!=bn) return false;
    for(size_t i=0;i<an;++i) if(a[i]!=b[i]) return false;
    return true;
}

bool put_setting(State& st,const char* key,size_t key_len,const char* value,size_t value_len) noexcept {
    if(!key||!key_len||(!value&&value_len)) return false;
    Setting* existing=find_setting(st,key,key_len);
    size_t value_off=0;
    if(!arena_append(st,value,value_len,&value_off)) return false;
    if(existing){ existing->value_off=value_off; existing->value_len=value_len; return true; }
    if(!reserve_array(&st.settings,&st.setting_capacity,st.setting_count,st.setting_count+1u)) return false;
    size_t name_off=0;
    if(!arena_append(st,key,key_len,&name_off)) return false;
    st.settings[st.setting_count++]=Setting{name_off,key_len,value_off,value_len,true};
    return true;
}

} } }
