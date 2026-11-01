// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/handlers/settings_snapshot_route.hpp"
#include "redfield/core/checked_math.hpp"
#include "redfield/xml/value_codec.hpp"

namespace redfield { namespace handlers {
namespace {
constexpr char prefix[]=" <LSX>\n  <Response id=\"";
constexpr char a[]="\" sender=\"";
constexpr char a2[]="\">\n    <GetSettingsResponse IsManualOffline=\"";
constexpr char b[]="\" Environment=\"";
constexpr char c[]="\" IsTelemetryEnabled=\"";
constexpr char c2[]="\" IsIGOAvailable=\"";
constexpr char d[]="\" IsIGOEnabled=\"";
constexpr char e[]="\" Language=\"";
constexpr char suffix[]="\" IsAutomaticGameUpdatesEnabled=\"false\"/>\n  </Response>\n </LSX>\n";

const char* normalized_bool(redfield::config::ValueView v) noexcept {
    if(redfield::config::explicit_true(v))return "true";
    if(redfield::config::explicit_false(v))return "false";
    return nullptr;
}
bool add(size_t& total,size_t value) noexcept {size_t n=0;if(!redfield::core::checked_add_size(total,value,n))return false;total=n;return true;}
void append(char*& dst,const char* src,size_t n) noexcept {for(size_t i=0;i<n;++i)*dst++=src[i];}
size_t cstrlen(const char* s) noexcept {size_t n=0;while(s&&s[n])++n;return n;}
bool source_size(redfield::xml::TextSpan v,size_t* n) noexcept {return redfield::xml::normalize_source_attribute_value(v,nullptr,0,n)==redfield::xml::ValueCodecStatus::ok;}
bool semantic_size(redfield::xml::TextSpan v,size_t* n) noexcept {return redfield::xml::escape_attribute_value(v,nullptr,0,n)==redfield::xml::ValueCodecStatus::ok;}
bool emit_source(redfield::xml::TextSpan v,char*& dst,size_t remaining,size_t expected) noexcept {size_t actual=0;const auto st=redfield::xml::normalize_source_attribute_value(v,dst,remaining,&actual);if(st!=redfield::xml::ValueCodecStatus::ok||actual!=expected)return false;dst+=actual;return true;}
bool emit_semantic(redfield::xml::TextSpan v,char*& dst,size_t remaining,size_t expected) noexcept {size_t actual=0;const auto st=redfield::xml::escape_attribute_value(v,dst,remaining,&actual);if(st!=redfield::xml::ValueCodecStatus::ok||actual!=expected)return false;dst+=actual;return true;}
}

HandlerStatus format_get_settings_response(const redfield::xml::RequestView& request,const redfield::config::SettingsSnapshot& s,char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!output&&capacity!=0))return HandlerStatus::invalid_request;
    if(!redfield::xml::span_equals(request.command,"GetSettings"))return HandlerStatus::wrong_command;
    if(!request.id.data||request.id.size==0)return HandlerStatus::invalid_request;
    if(!s.environment.present||(!s.environment.data&&s.environment.size)||!s.language.present||(!s.language.data&&s.language.size))return HandlerStatus::unsupported;

    const auto sender=redfield::xml::response_sender_policy(request);
    const bool sender_from_source=request.recipient.data&&request.recipient.size;
    const char* manual=normalized_bool(s.launch_offline_mode);const char* avail=normalized_bool(s.igo_available);const char* enabled=normalized_bool(s.igo_enabled);
    if(!manual||!avail||!enabled||!s.telemetry_enabled.present||(!s.telemetry_enabled.data&&s.telemetry_enabled.size)||!sender.data||sender.size==0)return HandlerStatus::unsupported;
    const size_t manual_n=cstrlen(manual),avail_n=cstrlen(avail),enabled_n=cstrlen(enabled);

    size_t id_n=0,sender_n=0,environment_n=0,telemetry_n=0,language_n=0;
    if(!source_size(request.id,&id_n))return HandlerStatus::invalid_request;
    if(sender_from_source){if(!source_size(sender,&sender_n))return HandlerStatus::invalid_request;}else if(!semantic_size(sender,&sender_n))return HandlerStatus::invalid_request;
    if(!semantic_size({s.environment.data,s.environment.size},&environment_n)||
       !semantic_size({s.telemetry_enabled.data,s.telemetry_enabled.size},&telemetry_n)||
       !semantic_size({s.language.data,s.language.size},&language_n))return HandlerStatus::unsupported;

    size_t need=0;
    if(!add(need,sizeof(prefix)-1)||!add(need,id_n)||!add(need,sizeof(a)-1)||!add(need,sender_n)||!add(need,sizeof(a2)-1)||!add(need,manual_n)||!add(need,sizeof(b)-1)||!add(need,environment_n)||!add(need,sizeof(c)-1)||!add(need,telemetry_n)||!add(need,sizeof(c2)-1)||!add(need,avail_n)||!add(need,sizeof(d)-1)||!add(need,enabled_n)||!add(need,sizeof(e)-1)||!add(need,language_n)||!add(need,sizeof(suffix)-1))return HandlerStatus::response_error;
    *written=need;
    size_t with_nul=0;if(!redfield::core::checked_add_size(need,1,with_nul)||!output||capacity<with_nul)return HandlerStatus::response_error;

    char* p=output;
    append(p,prefix,sizeof(prefix)-1);if(!emit_source(request.id,p,capacity-static_cast<size_t>(p-output),id_n))return HandlerStatus::invalid_request;
    append(p,a,sizeof(a)-1);if(sender_from_source){if(!emit_source(sender,p,capacity-static_cast<size_t>(p-output),sender_n))return HandlerStatus::invalid_request;}else if(!emit_semantic(sender,p,capacity-static_cast<size_t>(p-output),sender_n))return HandlerStatus::invalid_request;
    append(p,a2,sizeof(a2)-1);append(p,manual,manual_n);
    append(p,b,sizeof(b)-1);if(!emit_semantic({s.environment.data,s.environment.size},p,capacity-static_cast<size_t>(p-output),environment_n))return HandlerStatus::response_error;
    append(p,c,sizeof(c)-1);if(!emit_semantic({s.telemetry_enabled.data,s.telemetry_enabled.size},p,capacity-static_cast<size_t>(p-output),telemetry_n))return HandlerStatus::response_error;
    append(p,c2,sizeof(c2)-1);append(p,avail,avail_n);
    append(p,d,sizeof(d)-1);append(p,enabled,enabled_n);
    append(p,e,sizeof(e)-1);if(!emit_semantic({s.language.data,s.language.size},p,capacity-static_cast<size_t>(p-output),language_n))return HandlerStatus::response_error;
    append(p,suffix,sizeof(suffix)-1);*p='\0';*written=need;return HandlerStatus::ok;
}

HandlerStatus handle_get_settings(const redfield::xml::RequestView& request,const redfield::config::SettingsProvider& provider,char* output,size_t capacity,size_t* written) noexcept {
    const auto s=redfield::config::resolve_settings(provider);
    return format_get_settings_response(request,s,output,capacity,written);
}

} }
