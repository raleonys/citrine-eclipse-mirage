#pragma once
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/handlers/selector_pipeline.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#if defined(_WIN32)
#include "preloader_l/config/process_configuration.hpp"
#endif
#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/value_codec.hpp"
#include "preloader_l/core/checked_math.hpp"
#include "preloader_l/runtime/utc_time.hpp"
#include "preloader_l/runtime/service_state.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/runtime/achievement_watcher.hpp"
#include "preloader_l/runtime/avatar_path.hpp"
#include "preloader_l/runtime/auth_code_service.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#if defined(_WIN32)
extern "C" {
__declspec(dllimport) void* __stdcall GetProcessHeap();
__declspec(dllimport) void* __stdcall HeapAlloc(void*, unsigned long, size_t);
__declspec(dllimport) int __stdcall HeapFree(void*, unsigned long, void*);
}
#endif

namespace preloader_l { namespace handlers {
namespace {
#if defined(_WIN32)
void* transient_heap_alloc(void* context,size_t n) noexcept {
    return context ? HeapAlloc(context,0,n) : nullptr;
}
int transient_heap_free(void* context,void* p) noexcept {
    return context && HeapFree(context,0,p) ? 1 : 0;
}
#endif
struct HeapBytes {
    void* heap;
    char* data;
    size_t capacity;
    HeapBytes() noexcept : heap(nullptr),data(nullptr),capacity(0) {}
    ~HeapBytes() noexcept {
#if defined(_WIN32)
        if(data)(void)preloader_l::runtime::release_transient_bytes(data);
#endif
    }
    bool allocate(size_t n) noexcept {
#if defined(_WIN32)
        if(!n) return false;
        heap=GetProcessHeap();
        if(!heap) return false;
        const preloader_l::runtime::AllocatorPlatformOps ops{heap,&transient_heap_alloc,&transient_heap_free,nullptr};
        data=static_cast<char*>(preloader_l::runtime::allocate_transient_bytes(n,ops));
        if(!data) return false;
        capacity=n;
        return true;
#else
        (void)n;return false;
#endif
    }
};

struct BehavioralServiceStateScope {
    BehavioralServiceStateScope() noexcept { preloader_l::runtime::acquire_behavioral_service_state(); }
    ~BehavioralServiceStateScope() noexcept { preloader_l::runtime::release_behavioral_service_state(); }
    BehavioralServiceStateScope(const BehavioralServiceStateScope&)=delete;
    BehavioralServiceStateScope& operator=(const BehavioralServiceStateScope&)=delete;
};
enum class SpanEncoding : uint8_t { raw = 0, semantic_attribute, source_attribute };
struct Span {
    const char* data;
    size_t size;
    SpanEncoding encoding;
    Span(const char* d=nullptr,size_t n=0,SpanEncoding e=SpanEncoding::raw) noexcept : data(d),size(n),encoding(e) {}
};
size_t slen(const char* s) noexcept { size_t n=0; if(s) while(s[n]) ++n; return n; }
Span lit(const char* s) noexcept { return Span{s,slen(s),SpanEncoding::raw}; }
Span semantic_span(const char* p,size_t n) noexcept { return Span{p,n,SpanEncoding::semantic_attribute}; }
Span semantic_span(Span v) noexcept { v.encoding=SpanEncoding::semantic_attribute; return v; }
Span semantic_span(preloader_l::runtime::ServiceTextView v) noexcept { return Span{v.data,v.size,SpanEncoding::semantic_attribute}; }
Span source_span(preloader_l::xml::TextSpan v) noexcept { return Span{v.data,v.size,v.decoded() ? SpanEncoding::semantic_attribute : SpanEncoding::source_attribute}; }

bool safe_attr(preloader_l::xml::TextSpan s) noexcept {
    if(!s.data || s.size==0) return false;
    size_t encoded=0;
    return preloader_l::xml::normalize_source_attribute_value(s,nullptr,0,&encoded)==preloader_l::xml::ValueCodecStatus::ok;
}
bool safe_attr_value(preloader_l::xml::TextSpan s) noexcept {
    if(!s.data && s.size!=0) return false;
    size_t decoded=0;
    return preloader_l::xml::decode_source_value(s,nullptr,0,&decoded)==preloader_l::xml::ValueCodecStatus::ok;
}
bool safe_semantic_attr_value(preloader_l::xml::TextSpan s) noexcept {
    if(!s.data && s.size!=0) return false;
    size_t encoded=0;
    return preloader_l::xml::escape_attribute_value(s,nullptr,0,&encoded)==preloader_l::xml::ValueCodecStatus::ok;
}
bool eq(preloader_l::xml::TextSpan s,const char* x) noexcept { return preloader_l::xml::span_equals(s,x); }
void cp(char*& d,const char* s,size_t n) noexcept { for(size_t i=0;i<n;++i)*d++=s[i]; }
bool add(size_t& x,size_t y) noexcept { size_t z=0; if(!preloader_l::core::checked_add_size(x,y,z))return false; x=z; return true; }
HandlerStatus format_parts(const preloader_l::xml::RequestView& r,const Span* parts,size_t count,char* out,size_t cap,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!out&&cap))return HandlerStatus::invalid_request;
    if(!safe_attr(r.id)||!safe_attr(preloader_l::xml::response_sender_policy(r)))return HandlerStatus::invalid_request;
    size_t n=0;
    for(size_t i=0;i<count;++i){
        size_t part_size=parts[i].size;
        if(parts[i].encoding!=SpanEncoding::raw){
            const preloader_l::xml::TextSpan value{parts[i].data,parts[i].size};
            const auto st=parts[i].encoding==SpanEncoding::semantic_attribute
                ? preloader_l::xml::escape_attribute_value(value,nullptr,0,&part_size)
                : preloader_l::xml::normalize_source_attribute_value(value,nullptr,0,&part_size);
            if(st!=preloader_l::xml::ValueCodecStatus::ok)
                return parts[i].encoding==SpanEncoding::source_attribute ? HandlerStatus::invalid_request : HandlerStatus::response_error;
        }
        if(!add(n,part_size))return HandlerStatus::response_error;
    }
    *written=n;
    size_t with0=0; if(!add(with0,n) || !add(with0,1) || !out || cap<with0) return HandlerStatus::response_error;
    char* d=out;
    for(size_t i=0;i<count;++i){
        if(parts[i].encoding==SpanEncoding::raw){ cp(d,parts[i].data,parts[i].size); continue; }
        size_t encoded=0;
        const preloader_l::xml::TextSpan value{parts[i].data,parts[i].size};
        const auto st=parts[i].encoding==SpanEncoding::semantic_attribute
            ? preloader_l::xml::escape_attribute_value(value,d,cap-static_cast<size_t>(d-out),&encoded)
            : preloader_l::xml::normalize_source_attribute_value(value,d,cap-static_cast<size_t>(d-out),&encoded);
        if(st!=preloader_l::xml::ValueCodecStatus::ok)
            return parts[i].encoding==SpanEncoding::source_attribute ? HandlerStatus::invalid_request : HandlerStatus::response_error;
        d+=encoded;
    }
    *d='\0'; *written=n; return HandlerStatus::ok;
}
Span cfg_or(const char* key,const char* fallback) noexcept {
    const auto v=preloader_l::config::runtime_configuration_setting(key);
    return v.present ? Span{v.data,v.size} : lit(fallback);
}
preloader_l::config::ValueView runtime_online() noexcept {
    preloader_l::config::ValueView v=preloader_l::config::missing_value();
    const auto& p=preloader_l::config::runtime_settings_provider();
    if(preloader_l::config::lookup_setting(p,preloader_l::config::SettingKey::online,&v))return v;
    return preloader_l::config::literal_value("true");
}
bool offline() noexcept { return !preloader_l::config::online_enabled(runtime_online()); }
Span response_sender_span(const preloader_l::xml::RequestView& r) noexcept {
    if(r.recipient.data&&r.recipient.size)return source_span(r.recipient);
    return lit("EbisuSDK");
}
Span gameinfo_span(const char* key) noexcept {
    const preloader_l::xml::TextSpan selector{key,slen(key)};
    const auto resolution=resolve_get_game_info_selector(selector);
    return resolution.recognized && resolution.value.present
        ? Span{resolution.value.data,resolution.value.size}
        : Span{nullptr,0};
}
struct StreamWriter {
    char* out;
    size_t cap;
    size_t pos;
    bool valid;
    bool capacity_exhausted;
    StreamWriter(char* o,size_t c) noexcept : out(o),cap(c),pos(0),valid(true),capacity_exhausted(false) {}
    void invalidate_output() noexcept {
        if(out&&cap)out[0]='\0';
    }
    void append(const char* p,size_t n) noexcept {
        if(!valid)return;
        if(!p&&n){valid=false;invalidate_output();return;}
        size_t next=0;
        if(!preloader_l::core::checked_add_size(pos,n,next)){valid=false;invalidate_output();return;}
        if(out&&!capacity_exhausted){
            if(next>=cap){capacity_exhausted=true;invalidate_output();}
            else for(size_t i=0;i<n;++i)out[pos+i]=p[i];
        }
        pos=next;
    }
    void append(const char* z) noexcept { append(z,slen(z)); }
    void append(Span v) noexcept { append(v.data,v.size); }
    void append(preloader_l::runtime::ServiceTextView v) noexcept { append(v.data,v.size); }
    void append_encoded(preloader_l::xml::TextSpan v,bool source) noexcept {
        if(!valid)return;
        size_t encoded=0;
        const auto measure=source
            ? preloader_l::xml::normalize_source_attribute_value(v,nullptr,0,&encoded)
            : preloader_l::xml::escape_attribute_value(v,nullptr,0,&encoded);
        if(measure!=preloader_l::xml::ValueCodecStatus::ok){valid=false;invalidate_output();return;}
        size_t next=0;
        if(!preloader_l::core::checked_add_size(pos,encoded,next)){valid=false;invalidate_output();return;}
        if(out&&!capacity_exhausted){
            if(next>=cap){capacity_exhausted=true;invalidate_output();}
            else {
                size_t actual=0;
                const auto render=source
                    ? preloader_l::xml::normalize_source_attribute_value(v,out+pos,cap-pos,&actual)
                    : preloader_l::xml::escape_attribute_value(v,out+pos,cap-pos,&actual);
                if(render!=preloader_l::xml::ValueCodecStatus::ok||actual!=encoded){valid=false;invalidate_output();return;}
            }
        }
        pos=next;
    }
    void append_semantic(Span v) noexcept { append_encoded({v.data,v.size},false); }
    void append_semantic(preloader_l::runtime::ServiceTextView v) noexcept { append_encoded({v.data,v.size},false); }
    void append_source(preloader_l::xml::TextSpan v) noexcept { append_encoded(v,true); }
    bool finish(size_t* written) noexcept {
        if(written)*written=pos;
        if(!valid)return false;
        if(!out)return false; // size-query: publish full logical length through *written and signal capacity
        if(capacity_exhausted||pos>=cap){invalidate_output();return false;}
        out[pos]='\0';
        return true;
    }
};
size_t u64_text(uint64_t v,char* out) noexcept {
    char tmp[32]{}; size_t n=0; do{tmp[n++]=static_cast<char>('0'+v%10u);v/=10u;}while(v);
    for(size_t i=0;i<n;++i)out[i]=tmp[n-1u-i];
    out[n]='\0';
    return n;
}
size_t i32_text(int32_t v,char* out) noexcept {
    uint32_t mag=v<0?static_cast<uint32_t>(-(int64_t)v):static_cast<uint32_t>(v); char tmp[16]{}; size_t n=0;
    do{tmp[n++]=static_cast<char>('0'+mag%10u);mag/=10u;}while(mag);
    size_t pos=0;if(v<0)out[pos++]='-';for(size_t i=0;i<n;++i)out[pos+i]=tmp[n-1u-i];pos+=n;out[pos]='\0';return pos;
}
bool safe_service_text(preloader_l::runtime::ServiceTextView v) noexcept {
    return safe_semantic_attr_value(preloader_l::xml::TextSpan{v.data,v.size});
}

}

} }
