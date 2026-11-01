// SPDX-License-Identifier: GPL-3.0-only
// RF05: exact HF19 stateless fragments; see DERIVED_SOURCES.json.
#include "redfield/handlers/integration_routes.hpp"
#include "redfield/xml/response_builder.hpp"
#include "redfield/xml/value_codec.hpp"
#include "error_success_support.hpp"
namespace redfield { namespace handlers {
namespace {
constexpr char get_presence_visibility_body[] = "<GetPresenceVisibilityResponse Visible=\"false\"/>";
constexpr char get_block_list_body[] = "<GetBlockListResponse Return=\"Success\"/>";
bool safe_attr(redfield::xml::TextSpan s) noexcept {
    if(!s.data || s.size==0) return false;
    size_t encoded=0;
    return redfield::xml::normalize_source_attribute_value(s,nullptr,0,&encoded)==redfield::xml::ValueCodecStatus::ok;
}
HandlerStatus fixed_response(const redfield::xml::RequestView& request,const char* expected,const char* body,size_t body_size,char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!output&&capacity!=0))return HandlerStatus::invalid_request;
    if(!redfield::xml::span_equals(request.command,expected))return HandlerStatus::wrong_command;
    if(!request.id.data||!request.id.size)return HandlerStatus::invalid_request;
    const auto sender=redfield::xml::response_sender_policy(request);
    const auto status=redfield::xml::build_response_envelope_from_source_attributes(request.id,sender,{body,body_size},output,capacity,written);
    return status==redfield::xml::ResponseStatus::ok?HandlerStatus::ok:HandlerStatus::response_error;
}
}
HandlerStatus handle_get_presence_visibility(const redfield::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    return fixed_response(request,"GetPresenceVisibility",get_presence_visibility_body,sizeof(get_presence_visibility_body)-1u,output,capacity,written);
}
HandlerStatus handle_get_block_list(const redfield::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    return fixed_response(request,"GetBlockList",get_block_list_body,sizeof(get_block_list_body)-1u,output,capacity,written);
}
HandlerStatus handle_get_voip_status(const redfield::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(!redfield::xml::span_equals(request.command,"GetVoipStatus")){if(written)*written=0;return HandlerStatus::wrong_command;}
    return detail::render_error_success(
        request,route_error_contracts::get_voip_status_not_implemented,output,capacity,written);
}
HandlerStatus handle_query_chunk_status(const redfield::xml::RequestView& request,char*,size_t,size_t* written) noexcept {
    if(written)*written=0;
    if(!redfield::xml::span_equals(request.command,"QueryChunkStatus"))return HandlerStatus::wrong_command;
    if(!safe_attr(request.id))return HandlerStatus::invalid_request;
    // Donor1's NoResponseRequiredError is an internal control-flow mechanism.
    // The project contract preserves its observable result explicitly: this is
    // a recognized route, emits zero response bytes, and keeps the session alive.
    return HandlerStatus::no_response_required;
}
HandlerStatus handle_start_download(const redfield::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(!redfield::xml::span_equals(request.command,"StartDownload")){if(written)*written=0;return HandlerStatus::wrong_command;}
    return detail::render_error_success(
        request,route_error_contracts::start_download_success,output,capacity,written);
}

HandlerStatus handle_set_presence(const redfield::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(!redfield::xml::span_equals(request.command,"SetPresence")){if(written)*written=0;return HandlerStatus::wrong_command;}
    // Consumer + Donor1 convergence: SetPresence has no route-local
    // connectivity gate. The observable contract is always Success(0).
    return detail::render_error_success(
        request,route_error_contracts::set_presence_success,output,capacity,written);
}

HandlerStatus handle_restart_game(const redfield::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(!redfield::xml::span_equals(request.command,"RestartGame")){if(written)*written=0;return HandlerStatus::wrong_command;}
    return detail::render_error_success(
        request,route_error_contracts::restart_game_pending,output,capacity,written);
}

namespace {
HandlerStatus simple_route_result(const xml::RequestView& r,const char* name, protocol::OriginError code,const char* description,char* out,size_t cap,size_t* written) noexcept {
 if(!xml::span_equals(r.command,name)){if(written)*written=0;return HandlerStatus::wrong_command;}
 return detail::render_error_success(r,code,description,out,cap,written);
}
}


HandlerStatus handle_go_online(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
 // Donor bounded parser-local Success(0), NOT a connectivity/auth state transition.
 return simple_route_result(r,"GoOnline",protocol::OriginError::success,"",o,c,w);
}

HandlerStatus handle_enable_voip(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
 return simple_route_result(r,"EnableVoip",protocol::OriginError::not_implemented,"The function you are calling is not implemented.",o,c,w);
}

HandlerStatus handle_set_downloader_utilization(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
 return simple_route_result(r,"SetDownloaderUtilization",protocol::OriginError::success,"",o,c,w);
}

HandlerStatus handle_set_presence_visibility(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
 return simple_route_result(r,"SetPresenceVisibility",protocol::OriginError::success,"",o,c,w);
}
} }
