// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/handlers/utc_time_route.hpp"
#include "preloader_l/xml/response_builder.hpp"
#include "preloader_l/runtime/utc_time.hpp"

namespace preloader_l { namespace handlers {
namespace {
HandlerStatus fixed_route_response(const xml::RequestView& r,const char* name,const char* body,size_t len,char* out,size_t cap,size_t* written) noexcept {
 if(written)*written=0;
 if(!xml::span_equals(r.command,name))return HandlerStatus::wrong_command;
 if(!written||(!out&&cap)||!r.id.data||r.id.size==0)return HandlerStatus::invalid_request;
 const auto status=xml::build_response_envelope_from_source_attributes(r.id,xml::response_sender_policy(r),{body,len},out,cap,written);
 return status==xml::ResponseStatus::ok?HandlerStatus::ok:HandlerStatus::response_error;
}
}

HandlerStatus handle_get_utc_time(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
 if(w)*w=0;
 if(!xml::span_equals(r.command,"GetUTCTime"))return HandlerStatus::wrong_command;
 char utc[20]{};size_t n=0;
 if(!runtime::current_utc_timestamp(utc,sizeof(utc),&n))return HandlerStatus::response_error;
 constexpr char a[]="<GetUTCTimeResponse Time=\"";
 constexpr char b[]="\"/>";
 char body[sizeof(a)+sizeof(b)+20]{};
 size_t pos=0;
 for(size_t i=0;i<sizeof(a)-1;++i)body[pos++]=a[i];
 for(size_t i=0;i<n;++i)body[pos++]=utc[i];
 for(size_t i=0;i<sizeof(b)-1;++i)body[pos++]=b[i];
 return fixed_route_response(r,"GetUTCTime",body,pos,o,c,w);
}
} }
