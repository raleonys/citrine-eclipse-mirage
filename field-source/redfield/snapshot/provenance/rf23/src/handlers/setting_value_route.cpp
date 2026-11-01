// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/handlers/setting_value_route.hpp"
#include "redfield/core/checked_math.hpp"
#include "redfield/xml/value_codec.hpp"

namespace redfield { namespace handlers {
namespace {
constexpr char prefix[] = " <LSX>\n  <Response id=\"";
constexpr char middle_a[] = "\" sender=\"";
constexpr char middle_b[] = "\">\n    <GetSettingResponse Setting=\"";
constexpr char suffix[] = "\"/>\n  </Response>\n </LSX>\n";

void append(char*& dst, const char* src, size_t n) noexcept { for (size_t i=0;i<n;++i) *dst++=src[i]; }
bool add(size_t& total,size_t value) noexcept { size_t next=0; if(!redfield::core::checked_add_size(total,value,next))return false; total=next; return true; }

bool source_size(redfield::xml::TextSpan v,size_t* n) noexcept {
    return redfield::xml::normalize_source_attribute_value(v,nullptr,0,n)==redfield::xml::ValueCodecStatus::ok;
}
bool semantic_size(redfield::xml::TextSpan v,size_t* n) noexcept {
    return redfield::xml::escape_attribute_value(v,nullptr,0,n)==redfield::xml::ValueCodecStatus::ok;
}
bool emit_source(redfield::xml::TextSpan v,char*& dst,size_t remaining,size_t expected) noexcept {
    size_t actual=0;
    const auto st=redfield::xml::normalize_source_attribute_value(v,dst,remaining,&actual);
    if(st!=redfield::xml::ValueCodecStatus::ok||actual!=expected)return false;
    dst+=actual;return true;
}
bool emit_semantic(redfield::xml::TextSpan v,char*& dst,size_t remaining,size_t expected) noexcept {
    size_t actual=0;
    const auto st=redfield::xml::escape_attribute_value(v,dst,remaining,&actual);
    if(st!=redfield::xml::ValueCodecStatus::ok||actual!=expected)return false;
    dst+=actual;return true;
}
}

HandlerStatus format_get_setting_response(const redfield::xml::RequestView& request, redfield::config::ValueView setting, char* output, size_t capacity, size_t* written) noexcept {
    if (written) *written=0;
    if (!written || (!output && capacity!=0)) return HandlerStatus::invalid_request;
    if (!redfield::xml::span_equals(request.command,"GetSetting")) return HandlerStatus::wrong_command;
    if (!request.id.data || request.id.size==0) return HandlerStatus::invalid_request;
    if (!setting.present || (!setting.data && setting.size!=0)) return HandlerStatus::unsupported;

    size_t id_n=0,sender_n=0,setting_n=0;
    if(!source_size(request.id,&id_n))return HandlerStatus::invalid_request;
    const bool sender_from_source=request.recipient.data&&request.recipient.size;
    const auto sender=redfield::xml::response_sender_policy(request);
    if(!sender.data||sender.size==0)return HandlerStatus::invalid_request;
    if(sender_from_source){ if(!source_size(sender,&sender_n))return HandlerStatus::invalid_request; }
    else { if(!semantic_size(sender,&sender_n))return HandlerStatus::invalid_request; }
    if(!semantic_size({setting.data,setting.size},&setting_n))return HandlerStatus::response_error;

    size_t need=0;
    if(!add(need,sizeof(prefix)-1)||!add(need,id_n)||!add(need,sizeof(middle_a)-1)||!add(need,sender_n)||!add(need,sizeof(middle_b)-1)||!add(need,setting_n)||!add(need,sizeof(suffix)-1)) return HandlerStatus::response_error;
    *written=need;
    size_t with_nul=0;if(!redfield::core::checked_add_size(need,1,with_nul)||!output||capacity<with_nul)return HandlerStatus::response_error;

    char* d=output;
    append(d,prefix,sizeof(prefix)-1);
    if(!emit_source(request.id,d,capacity-static_cast<size_t>(d-output),id_n))return HandlerStatus::invalid_request;
    append(d,middle_a,sizeof(middle_a)-1);
    if(sender_from_source){ if(!emit_source(sender,d,capacity-static_cast<size_t>(d-output),sender_n))return HandlerStatus::invalid_request; }
    else if(!emit_semantic(sender,d,capacity-static_cast<size_t>(d-output),sender_n))return HandlerStatus::invalid_request;
    append(d,middle_b,sizeof(middle_b)-1);
    if(!emit_semantic({setting.data,setting.size},d,capacity-static_cast<size_t>(d-output),setting_n))return HandlerStatus::response_error;
    append(d,suffix,sizeof(suffix)-1);
    *d='\0';*written=need;
    return HandlerStatus::ok;
}

HandlerStatus handle_get_setting(const redfield::xml::RequestView& request, redfield::config::SettingKey key, const redfield::config::SettingsProvider& provider, char* output, size_t capacity, size_t* written) noexcept {
    redfield::config::ValueView value=redfield::config::missing_value();
    if(!redfield::config::lookup_setting(provider,key,&value)) { if(written)*written=0; return HandlerStatus::unsupported; }
    return format_get_setting_response(request,value,output,capacity,written);
}

} }
