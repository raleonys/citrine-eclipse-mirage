#include "preloader_l/handlers/integration_routes.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/xml/response_builder.hpp"
#include "preloader_l/xml/value_codec.hpp"
#include "error_success_support.hpp"
#include "route_response_support.hpp"

namespace preloader_l { namespace handlers {
namespace {
constexpr char get_presence_visibility_body[] = "<GetPresenceVisibilityResponse Visible=\"false\"/>";
constexpr char get_block_list_body[] = "<GetBlockListResponse Return=\"Success\"/>";

HandlerStatus fixed_response(const preloader_l::xml::RequestView& request,const char* expected,const char* body,size_t body_size,char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!output&&capacity!=0))return HandlerStatus::invalid_request;
    if(!preloader_l::xml::span_equals(request.command,expected))return HandlerStatus::wrong_command;
    if(!request.id.data||!request.id.size)return HandlerStatus::invalid_request;
    const auto sender=preloader_l::xml::response_sender_policy(request);
    const auto status=preloader_l::xml::build_response_envelope_from_source_attributes(request.id,sender,{body,body_size},output,capacity,written);
    return status==preloader_l::xml::ResponseStatus::ok?HandlerStatus::ok:HandlerStatus::response_error;
}

bool parse_chunk_id(preloader_l::xml::TextSpan source,int32_t* value) noexcept {
    if(!value)return false;
    preloader_l::xml::SemanticValue semantic;
    if(semantic.assign_parsed(source)!=preloader_l::xml::ValueCodecStatus::ok)return false;
    const auto v=semantic.view();
    if(!v.data||v.size==0)return false;
    size_t i=0;bool negative=false;
    if(v.data[i]=='-'||v.data[i]=='+'){negative=v.data[i]=='-';if(++i==v.size)return false;}
    uint64_t magnitude=0;
    for(;i<v.size;++i){
        const unsigned char c=static_cast<unsigned char>(v.data[i]);
        if(c<'0'||c>'9')return false;
        magnitude=magnitude*10u+static_cast<uint64_t>(c-'0');
        const uint64_t limit=negative?UINT64_C(2147483648):UINT64_C(2147483647);
        if(magnitude>limit)return false;
    }
    *value=negative?static_cast<int32_t>(-static_cast<int64_t>(magnitude)):static_cast<int32_t>(magnitude);
    return true;
}

HandlerStatus render_show_igo_response(const preloader_l::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    preloader_l::xml::RequestView normalized=request;
    constexpr char canonical_command[]="ShowIGO";
    if(preloader_l::xml::span_equals(request.command,"ShowIGOWindow"))
        normalized.command={canonical_command,sizeof(canonical_command)-1u};
    if(!preloader_l::xml::span_equals(normalized.command,"ShowIGO")){if(written)*written=0;return HandlerStatus::wrong_command;}
    return detail::render_error_success(
        normalized,route_error_contracts::show_igo_success,output,capacity,written);
}
}

HandlerStatus handle_show_igo(const preloader_l::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    const auto status=render_show_igo_response(request,output,capacity,written);
    // Direct-dispatch semantics remain unchanged: publish exactly one DOWN
    // event only after a real successful render; measure passes are side-effect free.
    if(status==HandlerStatus::ok&&output&&!preloader_l::runtime::publish_show_igo_down_event())return HandlerStatus::response_error;
    return status;
}

HandlerStatus prepare_show_igo_delivery(const preloader_l::xml::RequestView& request,char* output,size_t capacity,size_t* written,preloader_l::runtime::PreparedQueuedEvent** candidate) noexcept {
    if(candidate)*candidate=nullptr;
    if(!candidate){if(written)*written=0;return HandlerStatus::invalid_request;}
    const auto status=render_show_igo_response(request,output,capacity,written);
    if(status!=HandlerStatus::ok)return status;
    if(!preloader_l::runtime::prepare_show_igo_down_event(candidate))return HandlerStatus::response_error;
    return HandlerStatus::ok;
}
HandlerStatus handle_start_download(const preloader_l::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(!preloader_l::xml::span_equals(request.command,"StartDownload")){if(written)*written=0;return HandlerStatus::wrong_command;}
    return detail::render_error_success(
        request,route_error_contracts::start_download_success,output,capacity,written);
}

HandlerStatus handle_set_presence(const preloader_l::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(!preloader_l::xml::span_equals(request.command,"SetPresence")){if(written)*written=0;return HandlerStatus::wrong_command;}
    // Consumer + Donor1 convergence: SetPresence has no route-local
    // connectivity gate. The observable contract is always Success(0).
    return detail::render_error_success(
        request,route_error_contracts::set_presence_success,output,capacity,written);
}

HandlerStatus handle_get_presence_visibility(const preloader_l::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    return fixed_response(request,"GetPresenceVisibility",get_presence_visibility_body,sizeof(get_presence_visibility_body)-1u,output,capacity,written);
}

HandlerStatus handle_are_chunks_installed(const preloader_l::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!output&&capacity)||!preloader_l::xml::span_equals(request.command,"AreChunksInstalled"))
        return preloader_l::xml::span_equals(request.command,"AreChunksInstalled")?HandlerStatus::invalid_request:HandlerStatus::wrong_command;
    if(!safe_attr(request.id)||!safe_attr(preloader_l::xml::response_sender_policy(request)))return HandlerStatus::invalid_request;

    StreamWriter w(output,capacity);
    w.append(" <LSX>\n  <Response id=\"");
    w.append_source(request.id);
    w.append("\" sender=\"");
    w.append_source(preloader_l::xml::response_sender_policy(request));
    w.append("\">\n    <AreChunksInstalledResponse ItemId=\"\" Installed=\"true\">");

    size_t cursor=0;
    for(;;){
        preloader_l::xml::DirectChildView child{};
        const auto status=preloader_l::xml::next_direct_child(request.command_content,&cursor,&child);
        if(status==preloader_l::xml::DirectChildStatus::end)break;
        if(status!=preloader_l::xml::DirectChildStatus::found){if(written)*written=0;return HandlerStatus::invalid_request;}
        if(!preloader_l::xml::span_equals(child.name,"ChunkIds"))continue;
        int32_t chunk_id=0;
        if(!parse_chunk_id(child.content,&chunk_id)){if(written)*written=0;return HandlerStatus::unsupported;}
        char text[16]{};const size_t text_n=i32_text(chunk_id,text);
        w.append("<ChunkIds>");w.append(text,text_n);w.append("</ChunkIds>");
    }
    w.append("</AreChunksInstalledResponse>\n  </Response>\n </LSX>\n");
    return w.finish(written)?HandlerStatus::ok:HandlerStatus::response_error;
}

HandlerStatus handle_restart_game(const preloader_l::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(!preloader_l::xml::span_equals(request.command,"RestartGame")){if(written)*written=0;return HandlerStatus::wrong_command;}
    return detail::render_error_success(
        request,route_error_contracts::restart_game_pending,output,capacity,written);
}

HandlerStatus handle_get_voip_status(const preloader_l::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(!preloader_l::xml::span_equals(request.command,"GetVoipStatus")){if(written)*written=0;return HandlerStatus::wrong_command;}
    return detail::render_error_success(
        request,route_error_contracts::get_voip_status_not_implemented,output,capacity,written);
}

HandlerStatus handle_get_block_list(const preloader_l::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    return fixed_response(request,"GetBlockList",get_block_list_body,sizeof(get_block_list_body)-1u,output,capacity,written);
}

HandlerStatus handle_query_chunk_status(const preloader_l::xml::RequestView& request,char*,size_t,size_t* written) noexcept {
    if(written)*written=0;
    if(!preloader_l::xml::span_equals(request.command,"QueryChunkStatus"))return HandlerStatus::wrong_command;
    if(!safe_attr(request.id))return HandlerStatus::invalid_request;
    // Donor1's NoResponseRequiredError is an internal control-flow mechanism.
    // The project contract preserves its observable result explicitly: this is
    // a recognized route, emits zero response bytes, and keeps the session alive.
    return HandlerStatus::no_response_required;
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
