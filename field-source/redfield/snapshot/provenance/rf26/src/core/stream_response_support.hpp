// SPDX-License-Identifier: GPL-3.0-only
// RF08 exact stream writer, excluding two unused global-service view overloads.
#pragma once
#include "response_formatter_support.hpp"
namespace preloader_l { namespace {
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

} }
