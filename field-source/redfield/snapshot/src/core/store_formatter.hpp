// SPDX-License-Identifier: GPL-3.0-only
// RF14 exact pure store replies; no runtime store/global provider dependency.
#pragma once
#include "response_formatter_support.hpp"
#include "../handlers/error_success_support.hpp"
namespace preloader_l { namespace store_detail { namespace {
namespace xml=preloader_l::xml;
namespace protocol=preloader_l::protocol;
namespace detail=preloader_l::handlers::detail;
HandlerStatus simple_route_result(const xml::RequestView& r,const char* name, protocol::OriginError code,const char* description,char* out,size_t cap,size_t* written) noexcept {
 if(!xml::span_equals(r.command,name)){if(written)*written=0;return HandlerStatus::wrong_command;}
 return detail::render_error_success(r,code,description,out,cap,written);
}
HandlerStatus fixed_route_response(const xml::RequestView& r,const char* name,const char* body,size_t len,char* out,size_t cap,size_t* written) noexcept {
 if(written)*written=0;
 if(!xml::span_equals(r.command,name))return HandlerStatus::wrong_command;
 if(!written||(!out&&cap)||!r.id.data||r.id.size==0)return HandlerStatus::invalid_request;
 const auto status=xml::build_response_envelope_from_source_attributes(r.id,xml::response_sender_policy(r),{body,len},out,cap,written);
 return status==xml::ResponseStatus::ok?HandlerStatus::ok:HandlerStatus::response_error;
}
HandlerStatus handle_query_offers(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    if(!eq(r.command,"QueryOffers")){if(written)*written=0;return HandlerStatus::wrong_command;}
    
    // Phase 18 exact cross-arch closure: QueryOffers is request/config/store
    // independent. It creates QueryOffersResponse, finalizes it, and returns
    // true with no attributes or children.
    const Span p[]={lit(" <LSX>\n  <Response id=\""),source_span(r.id),lit("\" sender=\""),response_sender_span(r),lit("\">\n    <QueryOffersResponse/>\n  </Response>\n </LSX>\n")};
    return format_parts(r,p,sizeof(p)/sizeof(p[0]),out,cap,written);
}
HandlerStatus handle_select_store(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
 return simple_route_result(r,"SelectStore",protocol::OriginError::success,"",o,c,w);
}

HandlerStatus handle_get_wallet_balance(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
 constexpr char body[]="<GetWalletBalanceResponse Balance=\"0\"/>";
 return fixed_route_response(r,"GetWalletBalance",body,sizeof(body)-1u,o,c,w);
}
} } }
