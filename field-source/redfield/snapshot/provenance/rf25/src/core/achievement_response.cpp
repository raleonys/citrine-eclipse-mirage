// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/achievement_response.hpp"
#include "redfield/config_snapshot_view.hpp"
#include "redfield/config.hpp"
#include "redfield/profile_response.hpp"
#include "redfield/runtime/record_views.hpp"
#include "record_response_support.hpp"
#include "../../third_party/tinyxml2/tinyxml2.h"
namespace redfield { namespace achievement_detail { namespace {
using StreamWriter=redfield::RecordStreamWriter;
size_t i32_text(int32_t v,char* out) noexcept {
    uint32_t mag=v<0?static_cast<uint32_t>(-(int64_t)v):static_cast<uint32_t>(v); char tmp[16]{}; size_t n=0;
    do{tmp[n++]=static_cast<char>('0'+mag%10u);mag/=10u;}while(mag);
    size_t pos=0;if(v<0)out[pos++]='-';for(size_t i=0;i<n;++i)out[pos+i]=tmp[n-1u-i];pos+=n;out[pos]='\0';return pos;
}

class AchievementView final {
    const ConfigSnapshot& snapshot_;
    Span gameinfo_span(const char* key) const noexcept {
        if(std::string_view(key)!="DISPLAY_NAME")return {};
        const auto name=presence_title(snapshot_);return {name.data,name.size};
    }
    Span cfg_or(const char* key,const char* fallback) const noexcept {
        const auto value=ConfigSnapshotView{snapshot_}.or_default("Achievements",key,fallback);
        return {value.data,value.size};
    }
    size_t achievement_record_count() const noexcept {return snapshot_.achievements.size();}
    bool achievement_record_at(size_t i,redfield::runtime::AchievementRecordView* out) const noexcept {
        if(!out || i>=snapshot_.achievements.size())return false;
        const auto& a=snapshot_.achievements[i];
        *out={{a.id.data(),a.id.size()},{a.name.data(),a.name.size()},{a.description.data(),a.description.size()},{a.how_to.data(),a.how_to.size()},{a.image_id.data(),a.image_id.size()},{a.grant_date.data(),a.grant_date.size()},{a.expiration.data(),a.expiration.size()},a.total,a.count,a.progress};
        return true;
    }
public:
    explicit AchievementView(const ConfigSnapshot& snapshot) noexcept:snapshot_(snapshot){}
    HandlerStatus handle_query_achievements(const redfield::xml::RequestView&,char*,size_t,size_t*) noexcept;
};
HandlerStatus AchievementView::handle_query_achievements(const redfield::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    if(!eq(r.command,"QueryAchievements")){if(written)*written=0;return HandlerStatus::wrong_command;}
    
    if(written)*written=0;
    if(!written||(!out&&cap)||!safe_attr(r.id)||!safe_attr(redfield::xml::response_sender_policy(r)))return HandlerStatus::invalid_request;
    redfield::xml::TextSpan all_text{};bool include_all=false,all_found=false;
    // F1437-001: QueryAchievements has an exact raw direct-child text policy
    // for All. Do not reuse find_direct_child_text() here because that shared
    // helper trims leading/trailing XML text whitespace for other routes.
    size_t all_cursor=0;
    for(;;){
        redfield::xml::DirectChildView child{};
        const auto child_status=redfield::xml::next_direct_child(r.command_content,&all_cursor,&child);
        if(child_status==redfield::xml::DirectChildStatus::end)break;
        if(child_status==redfield::xml::DirectChildStatus::malformed||child_status==redfield::xml::DirectChildStatus::invalid_argument)return HandlerStatus::invalid_request;
        if(!eq(child.name,"All"))continue;
        all_found=true;all_text=child.content;
        if(child.content.element && child.content.element->FirstChildElement())return HandlerStatus::invalid_request;
        break;
    }
    // Approved read-selector policy: only semantic lowercase "true" over the
    // untrimmed direct-child text enables inclusion of zero-progress records.
    if(all_found){
        redfield::xml::ValueCodecStatus semantic_status=redfield::xml::ValueCodecStatus::ok;
        include_all=redfield::xml::source_value_equals_literal(all_text,"true",&semantic_status);
        if(semantic_status!=redfield::xml::ValueCodecStatus::ok)return HandlerStatus::invalid_request;
    }
    const Span game_name=gameinfo_span("DISPLAY_NAME"),set_name=cfg_or("AchievementsSet","");
    if(!safe_semantic_attr_value({game_name.data,game_name.size})||!safe_semantic_attr_value({set_name.data,set_name.size}))return HandlerStatus::response_error;
    StreamWriter w(out,cap); w.append(" <LSX>\n  <Response id=\"");w.append_source(r.id);w.append("\" sender=\"");if(r.recipient.data&&r.recipient.size)w.append_source(r.recipient);else w.append("EbisuSDK");w.append("\">\n    <AchievementSets>\n      <AchievementSet GameName=\"");w.append_semantic(game_name);w.append("\" Name=\"");w.append_semantic(set_name);w.append("\"");
    const size_t count=achievement_record_count();
    bool opened=false;
    for(size_t i=0;i<count;++i){
        redfield::runtime::AchievementRecordView a{};if(!achievement_record_at(i,&a))return HandlerStatus::response_error;
        if(!include_all&&a.progress==0)continue;
        const redfield::runtime::ServiceTextView vv[]={a.id,a.name,a.description,a.how_to,a.image_id,a.grant_date,a.expiration};for(const auto& v:vv)if(!safe_service_text(v))return HandlerStatus::response_error;
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
namespace redfield {
ResponseResult dispatch_achievement_response(const xml::Request& request,const ConfigSnapshot& snapshot,char* output,std::size_t capacity,std::size_t* written) noexcept {
    if(!written)return ResponseResult::rejected;
    *written=0;if(!output || !capacity)return ResponseResult::rejected;
    achievement_detail::AchievementView view(snapshot);
    std::size_t measured=0;
    const auto measure=view.handle_query_achievements(request,nullptr,0,&measured);
    if(measure==HandlerStatus::wrong_command || measure==HandlerStatus::unsupported)return ResponseResult::no_response;
    if(measure!=HandlerStatus::ok || measured>=capacity)return ResponseResult::rejected;
    std::size_t actual=0;
    const auto render=view.handle_query_achievements(request,output,capacity,&actual);
    if(render!=HandlerStatus::ok || actual!=measured || actual>=capacity)return ResponseResult::rejected;
    *written=actual;return ResponseResult::response;
}
}
