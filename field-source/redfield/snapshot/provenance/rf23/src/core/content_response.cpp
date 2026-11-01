// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/content_response.hpp"
#include "redfield/config.hpp"
#include "redfield/runtime/record_views.hpp"
#include "record_response_support.hpp"
namespace redfield { namespace content_detail { namespace {
// Preserve the original streaming serializer, adding a validated size-query
// success result and adapting the original record view to the shared writer.
using StreamWriter=redfield::RecordStreamWriter;

class ContentView final {
    const ConfigSnapshot& snapshot_;
    size_t content_record_count() const noexcept {return snapshot_.content.size();}
    bool content_record_at(size_t i,redfield::runtime::ContentRecordView* out) const noexcept {
        if(!out || i>=snapshot_.content.size())return false;
        const auto& r=snapshot_.content[i];
        *out={{r.content_id.data(),r.content_id.size()},{r.display_name.data(),r.display_name.size()},{r.version.data(),r.version.size()},{r.state.data(),r.state.size()}};
        return true;
    }
public:
    explicit ContentView(const ConfigSnapshot& snapshot) noexcept:snapshot_(snapshot){}
    HandlerStatus handle_query_content(const redfield::xml::RequestView&,char*,size_t,size_t*) noexcept;
};
HandlerStatus ContentView::handle_query_content(const redfield::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    if(!eq(r.command,"QueryContent")){if(written)*written=0;return HandlerStatus::wrong_command;}
    if(written)*written=0;
    if(!written||(!out&&cap)||!safe_attr(r.id)||!safe_attr(redfield::xml::response_sender_policy(r)))return HandlerStatus::invalid_request;
    StreamWriter w(out,cap); w.append(" <LSX>\n  <Response id=\""); w.append_source(r.id); w.append("\" sender=\""); if(r.recipient.data&&r.recipient.size)w.append_source(r.recipient);else w.append("EbisuSDK"); w.append("\">\n    <QueryContentResponse");
    const size_t count=content_record_count();
    if(!count){ w.append("/>\n  </Response>\n </LSX>\n"); return w.finish(written)?HandlerStatus::ok:HandlerStatus::response_error; }
    w.append(">\n");
    for(size_t i=0;i<count;++i){ redfield::runtime::ContentRecordView rec{}; if(!content_record_at(i,&rec)||!safe_service_text(rec.content_id)||!safe_service_text(rec.display_name)||!safe_service_text(rec.version)||!safe_service_text(rec.state))return HandlerStatus::response_error;
        w.append("      <Game progressValue=\"0\" contentID=\""); w.append_semantic(rec.content_id); w.append("\" availableVersion=\""); w.append_semantic(rec.version); w.append("\" displayName=\""); w.append_semantic(rec.display_name); w.append("\" state=\""); w.append_semantic(rec.state); w.append("\" installedVersion=\""); w.append_semantic(rec.version); w.append("\"/>\n"); }
    w.append("    </QueryContentResponse>\n  </Response>\n </LSX>\n"); return w.finish(written)?HandlerStatus::ok:HandlerStatus::response_error;
}
} } }
namespace redfield {
ResponseResult dispatch_content_response(const xml::Request& request,const ConfigSnapshot& snapshot,char* output,std::size_t capacity,std::size_t* written) noexcept {
    if(!written)return ResponseResult::rejected;
    *written=0;if(!output || !capacity)return ResponseResult::rejected;
    content_detail::ContentView view(snapshot);
    std::size_t measured=0;
    const auto measure=view.handle_query_content(request,nullptr,0,&measured);
    if(measure==HandlerStatus::wrong_command || measure==HandlerStatus::unsupported)return ResponseResult::no_response;
    if(measure!=HandlerStatus::ok || measured>=capacity)return ResponseResult::rejected;
    std::size_t actual=0;
    const auto render=view.handle_query_content(request,output,capacity,&actual);
    if(render!=HandlerStatus::ok || actual!=measured || actual>=capacity)return ResponseResult::rejected;
    *written=actual;return ResponseResult::response;
}
}
