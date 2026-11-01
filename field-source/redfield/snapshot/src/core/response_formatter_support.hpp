// SPDX-License-Identifier: GPL-3.0-only
// RF07 shared exact formatter fragments; DERIVED_SOURCES.json owns reconstruction.
#pragma once
#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/value_codec.hpp"
#include "preloader_l/core/checked_math.hpp"
#include "preloader_l/handlers/handler_status.hpp"
namespace preloader_l { namespace {
using preloader_l::handlers::HandlerStatus;
enum class SpanEncoding : uint8_t { raw = 0, semantic_attribute, source_attribute };
struct Span {
    const char* data;
    size_t size;
    SpanEncoding encoding;
    Span(const char* d=nullptr,size_t n=0,SpanEncoding e=SpanEncoding::raw) noexcept : data(d),size(n),encoding(e) {}
};
inline size_t slen(const char* s) noexcept { size_t n=0; if(s) while(s[n]) ++n; return n; }
inline Span lit(const char* s) noexcept { return Span{s,slen(s),SpanEncoding::raw}; }
[[maybe_unused]] inline Span semantic_span(const char* p,size_t n) noexcept { return Span{p,n,SpanEncoding::semantic_attribute}; }
inline Span semantic_span(Span v) noexcept { v.encoding=SpanEncoding::semantic_attribute; return v; }
inline Span source_span(preloader_l::xml::TextSpan v) noexcept { return Span{v.data,v.size,v.decoded() ? SpanEncoding::semantic_attribute : SpanEncoding::source_attribute}; }

inline bool safe_attr(preloader_l::xml::TextSpan s) noexcept {
    if(!s.data || s.size==0) return false;
    size_t encoded=0;
    return preloader_l::xml::normalize_source_attribute_value(s,nullptr,0,&encoded)==preloader_l::xml::ValueCodecStatus::ok;
}
[[maybe_unused]] inline bool safe_attr_value(preloader_l::xml::TextSpan s) noexcept {
    if(!s.data && s.size!=0) return false;
    size_t decoded=0;
    return preloader_l::xml::decode_source_value(s,nullptr,0,&decoded)==preloader_l::xml::ValueCodecStatus::ok;
}
[[maybe_unused]] inline bool safe_semantic_attr_value(preloader_l::xml::TextSpan s) noexcept {
    if(!s.data && s.size!=0) return false;
    size_t encoded=0;
    return preloader_l::xml::escape_attribute_value(s,nullptr,0,&encoded)==preloader_l::xml::ValueCodecStatus::ok;
}
inline bool eq(preloader_l::xml::TextSpan s,const char* x) noexcept { return preloader_l::xml::span_equals(s,x); }
inline void cp(char*& d,const char* s,size_t n) noexcept { for(size_t i=0;i<n;++i)*d++=s[i]; }
inline bool add(size_t& x,size_t y) noexcept { size_t z=0; if(!preloader_l::core::checked_add_size(x,y,z))return false; x=z; return true; }
inline HandlerStatus format_parts(const preloader_l::xml::RequestView& r,const Span* parts,size_t count,char* out,size_t cap,size_t* written) noexcept {
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
inline Span response_sender_span(const preloader_l::xml::RequestView& r) noexcept {
    if(r.recipient.data&&r.recipient.size)return source_span(r.recipient);
    return lit("EbisuSDK");
}

} }
