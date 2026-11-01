// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/image_response.hpp"
#include "redfield/config_snapshot_view.hpp"
#include "response_formatter_support.hpp"
namespace redfield { namespace {
redfield::handlers::HandlerStatus format_query_image(const xml::Request& r,Span resource,char* out,size_t cap,size_t* written) noexcept {
    if(written)*written=0;
    if(!eq(r.command,"QueryImage"))return HandlerStatus::wrong_command;
    if(!written||(!out&&cap)||!safe_attr(r.id))return HandlerStatus::invalid_request;
    redfield::xml::TextSpan width{},height{},image_id{};
    const auto sw=redfield::xml::find_direct_child_text(r.command_content,"Width",&width),sh=redfield::xml::find_direct_child_text(r.command_content,"Height",&height),si=redfield::xml::find_direct_child_text(r.command_content,"ImageId",&image_id);
    if(sw==redfield::xml::ChildTextStatus::malformed||sh==redfield::xml::ChildTextStatus::malformed||si==redfield::xml::ChildTextStatus::malformed)return HandlerStatus::invalid_request;
    if(sw!=redfield::xml::ChildTextStatus::found||sh!=redfield::xml::ChildTextStatus::found||si!=redfield::xml::ChildTextStatus::found)return HandlerStatus::unsupported;
    if(!safe_attr_value(width)||!safe_attr_value(height))return HandlerStatus::invalid_request;
    redfield::xml::SemanticValue semantic_image_id;
    const auto image_decode=semantic_image_id.assign_parsed(image_id);
    if(image_decode==redfield::xml::ValueCodecStatus::malformed_entity||image_decode==redfield::xml::ValueCodecStatus::invalid_character||image_decode==redfield::xml::ValueCodecStatus::invalid_argument)return HandlerStatus::invalid_request;
    if(image_decode!=redfield::xml::ValueCodecStatus::ok)return HandlerStatus::response_error;
    const auto image_id_value=semantic_image_id.view();
    if(!safe_semantic_attr_value({resource.data,resource.size}))return HandlerStatus::response_error;
    size_t colon=image_id_value.size;for(size_t i=0;i<image_id_value.size;++i)if(image_id_value.data[i]==':'){colon=i;break;}
    if(colon!=image_id_value.size){
        const redfield::xml::TextSpan suffix{image_id_value.data+colon+1u,image_id_value.size-colon-1u};
        if(!safe_semantic_attr_value(suffix))return HandlerStatus::unsupported;
        // Phase 430 behavioral ordering: signed Width parsing and Width/208 belong
        // strictly to the provider-enabled branch. This normalized build has no
        // bound image provider, so the provider gate is false and the branch is
        // bypassed without parsing Width. ResourcePath remains Avatar or the
        // project fallback beside the loaded DLL: surge.png.
    }
    const Span p[]={lit(" <LSX>\n  <Response id=\""),source_span(r.id),lit("\" sender=\""),response_sender_span(r),lit("\">\n    <QueryImageResponse Result=\"0\">\n     <Image ImageId=\""),source_span(image_id),lit("\" Height=\""),source_span(height),lit("\" ResourcePath=\""),semantic_span(resource),lit("\" Width=\""),source_span(width),lit("\"/>\n    </QueryImageResponse>\n  </Response>\n </LSX>\n")};
    return format_parts(r,p,sizeof(p)/sizeof(p[0]),out,cap,written);
}
} // namespace
ResponseResult dispatch_image_response(const xml::Request& request,const ConfigSnapshot& snapshot,char* output,std::size_t capacity,std::size_t* written) noexcept {
    if(!written)return ResponseResult::rejected;
    *written=0;if(!output || !capacity)return ResponseResult::rejected;
    // Preserve direct User.Avatar, last occurrence, raw config value semantics.
    Span resource{snapshot.avatar_fallback_path.data(),snapshot.avatar_fallback_path.size()};
    const auto avatar=ConfigSnapshotView{snapshot}.find("User","Avatar");
    const bool override_present=avatar.present;
    if(override_present)resource={avatar.data,avatar.size};
    // A context-free store has no DLL path. An explicitly empty override is valid.
    if(!override_present && snapshot.avatar_fallback_path.empty())return ResponseResult::rejected;
    const auto status=format_query_image(request,resource,output,capacity,written);
    using redfield::handlers::HandlerStatus;
    if(status==HandlerStatus::unsupported || status==HandlerStatus::no_response_required || status==HandlerStatus::wrong_command){*written=0;return ResponseResult::no_response;}
    if(status!=HandlerStatus::ok || *written>=capacity){*written=0;return ResponseResult::rejected;}
    return ResponseResult::response;
}
}
