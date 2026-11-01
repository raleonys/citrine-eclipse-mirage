// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/query_presence_response.hpp"
#include "redfield/profile_response.hpp"
#include "stream_response_support.hpp"
#include "../../third_party/tinyxml2/tinyxml2.h"
#include <cstring>
namespace preloader_l { namespace handlers { namespace {
using redfield::Span;
using redfield::StreamWriter;
using redfield::safe_attr;
using redfield::safe_semantic_attr_value;
using xml::TextSpan;
// Bounded first direct text node policy: donor skips leading comments, not elements.
// Textless and element-first nodes fail closed; trailing nodes after first text/CDATA follow donor first-node precedence.
struct UserText { TextSpan value; bool cdata; };
bool direct_first_text(TextSpan content,UserText* result) noexcept {
 if(!result || !content.document || !content.element) return false;
 // No secondary text scanner: operate on the same TinyXML2 document used for
 // envelope parsing. GetText skips leading comments and honors first text.
 const auto* node=content.element->FirstChild();
 while(node && node->ToComment()) node=node->NextSibling();
 const auto* txt=node ? node->ToText():nullptr;
 if(!txt || !txt->Value() || !*txt->Value()) return false;
 result->value=TextSpan{txt->Value(),std::strlen(txt->Value()),content.document};
 result->cdata=txt->CData();
 return true;
}
HandlerStatus validate(const xml::RequestView& r,const config::ValueView& persona) noexcept {
 if(!r.id.data||r.id.size==0||!safe_attr(r.id)||!safe_attr(xml::response_sender_policy(r)))return HandlerStatus::invalid_request;
 if(!safe_semantic_attr_value({persona.data,persona.size}))return HandlerStatus::response_error;
 size_t cursor=0,count=0;
 for(;;){
  xml::DirectChildView child{};
  const auto state=xml::next_direct_child(r.command_content,&cursor,&child);
  if(state==xml::DirectChildStatus::end)break;
  if(state!=xml::DirectChildStatus::found||child.self_closing)return HandlerStatus::invalid_request;
  UserText id{};
  if(!direct_first_text(child.content,&id))return HandlerStatus::invalid_request;
  size_t needed=0;
  const auto ok=id.cdata?xml::escape_attribute_value(id.value,nullptr,0,&needed):xml::normalize_source_attribute_value(id.value,nullptr,0,&needed);
  if(ok!=xml::ValueCodecStatus::ok)return HandlerStatus::invalid_request;
  if(++count>512u)return HandlerStatus::unsupported;
 }
 return HandlerStatus::ok;
}
bool emit(const xml::RequestView& r,const config::ValueView& persona,StreamWriter& writer) noexcept {
 writer.append(" <LSX>\n  <Response id=\"");writer.append_source(r.id);
 writer.append("\" sender=\"");writer.append_source(xml::response_sender_policy(r));
 writer.append("\">\n    <QueryPresenceResponse>");
 size_t cursor=0;
 for(;;){
  xml::DirectChildView child{};
  if(xml::next_direct_child(r.command_content,&cursor,&child)==xml::DirectChildStatus::end)break;
  UserText id{};if(!direct_first_text(child.content,&id))return false;
  writer.append("<Friend AvatarId=\"\" Title=\"\" GroupId=\"\" MultiplayerId=\"\" TitleId=\"\" Presence=\"UNKNOWN\" PersonaId=\"0\" GamePresence=\"\" Group=\"\" Persona=\"");
  writer.append_semantic(Span{persona.data,persona.size});
  writer.append("\" RichPresence=\"\" UserId=\"");
  if(id.cdata)writer.append_semantic(Span{id.value.data,id.value.size});else writer.append_source(id.value);
  writer.append("\" State=\"NONE\"/>");
 }
 writer.append("</QueryPresenceResponse>\n  </Response>\n </LSX>\n");
 return writer.valid;
}
}
HandlerStatus format_query_presence(const xml::RequestView& r,const config::ValueView& persona,char* out,size_t cap,size_t* written) noexcept {
 if(written)*written=0;
 if(!xml::span_equals(r.command,"QueryPresence"))return HandlerStatus::wrong_command;
 if(!written||(!out&&cap))return HandlerStatus::invalid_request;
 const auto validation=validate(r,persona);
 if(validation!=HandlerStatus::ok){return validation;}
 StreamWriter measure(nullptr,0);
 const bool measured=emit(r,persona,measure);
 size_t required=0;measure.finish(&required);
 if(!measured||!measure.valid){return HandlerStatus::response_error;}
 *written=required;
 if(!out||cap<=required){return HandlerStatus::response_error;}
 StreamWriter writer(out,cap);
 const bool rendered=emit(r,persona,writer)&&writer.finish(written);
 
 if(!rendered){if(out&&cap)out[0]='\0';return HandlerStatus::response_error;}
 return HandlerStatus::ok;
}
} }
namespace redfield {
ResponseResult dispatch_query_presence_response(const xml::Request& request,const ConfigSnapshot& snapshot,char* output,std::size_t capacity,std::size_t* written) noexcept {
    if(!written)return ResponseResult::rejected;
    *written=0;if(!output || !capacity)return ResponseResult::rejected;
    const auto persona=profile_policy(snapshot).username;
    const auto status=preloader_l::handlers::format_query_presence(request,persona,output,capacity,written);
    using preloader_l::handlers::HandlerStatus;
    if(status==HandlerStatus::unsupported || status==HandlerStatus::no_response_required || status==HandlerStatus::wrong_command){*written=0;return ResponseResult::no_response;}
    if(status!=HandlerStatus::ok || *written>=capacity){*written=0;return ResponseResult::rejected;}
    return ResponseResult::response;
}
}
