#include "preloader_l/handlers/image_route.hpp"
#include "route_response_support.hpp"

namespace preloader_l { namespace handlers {
HandlerStatus handle_query_image(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    if(written)*written=0;
    if(!eq(r.command,"QueryImage"))return HandlerStatus::wrong_command;
    if(!written||(!out&&cap)||!safe_attr(r.id))return HandlerStatus::invalid_request;
    preloader_l::xml::TextSpan width{},height{},image_id{};
    const auto sw=preloader_l::xml::find_direct_child_text(r.command_content,"Width",&width),sh=preloader_l::xml::find_direct_child_text(r.command_content,"Height",&height),si=preloader_l::xml::find_direct_child_text(r.command_content,"ImageId",&image_id);
    if(sw==preloader_l::xml::ChildTextStatus::malformed||sh==preloader_l::xml::ChildTextStatus::malformed||si==preloader_l::xml::ChildTextStatus::malformed)return HandlerStatus::invalid_request;
    if(sw!=preloader_l::xml::ChildTextStatus::found||sh!=preloader_l::xml::ChildTextStatus::found||si!=preloader_l::xml::ChildTextStatus::found)return HandlerStatus::unsupported;
    if(!safe_attr_value(width)||!safe_attr_value(height))return HandlerStatus::invalid_request;
    preloader_l::xml::SemanticValue semantic_image_id;
    const auto image_decode=semantic_image_id.assign_parsed(image_id);
    if(image_decode==preloader_l::xml::ValueCodecStatus::malformed_entity||image_decode==preloader_l::xml::ValueCodecStatus::invalid_character||image_decode==preloader_l::xml::ValueCodecStatus::invalid_argument)return HandlerStatus::invalid_request;
    if(image_decode!=preloader_l::xml::ValueCodecStatus::ok)return HandlerStatus::response_error;
    const auto image_id_value=semantic_image_id.view();
    const auto avatar_cfg=preloader_l::config::runtime_configuration_setting("Avatar");
    HeapBytes normalized_path{};size_t normalized_path_n=0;
    Span resource{};
    if(avatar_cfg.present)resource=Span{avatar_cfg.data,avatar_cfg.size};
    else {
        (void)preloader_l::runtime::normalized_avatar_path(nullptr,0,&normalized_path_n);
        if(!normalized_path_n||normalized_path_n==static_cast<size_t>(-1)||
           !normalized_path.allocate(normalized_path_n+1u)||
           !preloader_l::runtime::normalized_avatar_path(normalized_path.data,normalized_path.capacity,&normalized_path_n))
            return HandlerStatus::response_error;
        resource=Span{normalized_path.data,normalized_path_n};
    }
    if(!safe_semantic_attr_value({resource.data,resource.size}))return HandlerStatus::response_error;
    size_t colon=image_id_value.size;for(size_t i=0;i<image_id_value.size;++i)if(image_id_value.data[i]==':'){colon=i;break;}
    if(colon!=image_id_value.size){
        const preloader_l::xml::TextSpan suffix{image_id_value.data+colon+1u,image_id_value.size-colon-1u};
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

} }
