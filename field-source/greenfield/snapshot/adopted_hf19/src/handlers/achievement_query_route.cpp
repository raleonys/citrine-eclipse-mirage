#include "achievement_routes_internal.hpp"
#include "route_response_support.hpp"
#include "../../third_party/tinyxml2/tinyxml2.h"

namespace preloader_l { namespace handlers { namespace detail {
namespace {
struct QueryAchievementsDeliveryScope {
    bool held;
    QueryAchievementsDeliveryScope() noexcept : held(!preloader_l::runtime::achievement_watcher_callback_active()) {
        if(held)preloader_l::runtime::acquire_grant_achievement_delivery();
    }
    ~QueryAchievementsDeliveryScope() noexcept { if(held)preloader_l::runtime::release_grant_achievement_delivery(); }
    QueryAchievementsDeliveryScope(const QueryAchievementsDeliveryScope&)=delete;
    QueryAchievementsDeliveryScope& operator=(const QueryAchievementsDeliveryScope&)=delete;
};
}
HandlerStatus handle_query_achievements(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    if(!eq(r.command,"QueryAchievements")){if(written)*written=0;return HandlerStatus::wrong_command;}
    
    if(written)*written=0;
    if(!written||(!out&&cap)||!safe_attr(r.id)||!safe_attr(preloader_l::xml::response_sender_policy(r)))return HandlerStatus::invalid_request;
    preloader_l::xml::TextSpan all_text{};bool include_all=false,all_found=false;
    // F1437-001: QueryAchievements has an exact raw direct-child text policy
    // for All. Do not reuse find_direct_child_text() here because that shared
    // helper trims leading/trailing XML text whitespace for other routes.
    size_t all_cursor=0;
    for(;;){
        preloader_l::xml::DirectChildView child{};
        const auto child_status=preloader_l::xml::next_direct_child(r.command_content,&all_cursor,&child);
        if(child_status==preloader_l::xml::DirectChildStatus::end)break;
        if(child_status==preloader_l::xml::DirectChildStatus::malformed||child_status==preloader_l::xml::DirectChildStatus::invalid_argument)return HandlerStatus::invalid_request;
        if(!eq(child.name,"All"))continue;
        all_found=true;all_text=child.content;
        if(child.content.element && child.content.element->FirstChildElement())return HandlerStatus::invalid_request;
        break;
    }
    // Approved read-selector policy: only semantic lowercase "true" over the
    // untrimmed direct-child text enables inclusion of zero-progress records.
    if(all_found){
        preloader_l::xml::ValueCodecStatus semantic_status=preloader_l::xml::ValueCodecStatus::ok;
        include_all=preloader_l::xml::source_value_equals_literal(all_text,"true",&semantic_status);
        if(semantic_status!=preloader_l::xml::ValueCodecStatus::ok)return HandlerStatus::invalid_request;
    }
    // W7: wait out any already in-flight deferred GrantAchievement commit.
    // The outer-dispatch gate still spans this complete measure/render pass, so
    // a new Grant cannot enter between QueryAchievements measure and render.
    QueryAchievementsDeliveryScope delivery_scope;
    const Span game_name=gameinfo_span("DISPLAY_NAME"),set_name=cfg_or("AchievementsSet","");
    if(!safe_semantic_attr_value({game_name.data,game_name.size})||!safe_semantic_attr_value({set_name.data,set_name.size}))return HandlerStatus::response_error;
    StreamWriter w(out,cap); w.append(" <LSX>\n  <Response id=\"");w.append_source(r.id);w.append("\" sender=\"");if(r.recipient.data&&r.recipient.size)w.append_source(r.recipient);else w.append("EbisuSDK");w.append("\">\n    <AchievementSets>\n      <AchievementSet GameName=\"");w.append_semantic(game_name);w.append("\" Name=\"");w.append_semantic(set_name);w.append("\"");
    BehavioralServiceStateScope store_scope;
    const size_t count=preloader_l::runtime::achievement_record_count_locked();
    bool opened=false;
    for(size_t i=0;i<count;++i){
        preloader_l::runtime::AchievementRecordView a{};if(!preloader_l::runtime::achievement_record_at_locked(i,&a))return HandlerStatus::response_error;
        if(!include_all&&a.progress==0)continue;
        const preloader_l::runtime::ServiceTextView vv[]={a.id,a.name,a.description,a.how_to,a.image_id,a.grant_date,a.expiration};for(const auto& v:vv)if(!safe_service_text(v))return HandlerStatus::response_error;
        if(!opened){w.append(">\n");opened=true;}
        char total[32]{},cnt[32]{},prog[32]{};size_t tn=i32_text(a.total,total),cn=i32_text(a.count,cnt),pn=i32_text(a.progress,prog);
        // Shared behavioral serializer 0x18002C080 used by QueryAchievements and
        // GrantAchievement: preserve attribute order and fixed-empty metadata.
        w.append("        <Achievement Id=\"");w.append_semantic(a.id);w.append("\" Total=\"");w.append(total,tn);w.append("\" Count=\"");w.append(cnt,cn);w.append("\" Progress=\"");w.append(prog,pn);w.append("\" GrantDate=\"");w.append_semantic(a.grant_date);w.append("\" Expiration=\"");w.append_semantic(a.expiration);w.append("\" ImageId=\"");w.append_semantic(a.image_id);w.append("\" Name=\"");w.append_semantic(a.name);w.append("\" Description=\"");w.append_semantic(a.description);w.append("\" HowTo=\"");w.append_semantic(a.how_to);w.append("\"/>\n");
    }
    if(!opened)w.append("/>\n");else w.append("      </AchievementSet>\n");
    w.append("    </AchievementSets>\n  </Response>\n </LSX>\n");return w.finish(written)?HandlerStatus::ok:HandlerStatus::response_error;
}

} } }
