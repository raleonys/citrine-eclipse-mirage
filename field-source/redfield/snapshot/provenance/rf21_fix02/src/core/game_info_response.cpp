// SPDX-License-Identifier: GPL-3.0-only
// RF07 exact selector/handler bodies with explicit snapshot context.
#include "redfield/game_info_response.hpp"
#include "redfield/profile_response.hpp"
#include "redfield/settings_response.hpp"
#include "preloader_l/handlers/selector_pipeline.hpp"
#include "preloader_l/runtime/utc_time.hpp"
#include "response_formatter_support.hpp"
namespace redfield { namespace {
using preloader_l::handlers::HandlerStatus;
using preloader_l::handlers::SelectorResolution;
constexpr char kLanguagesFallback[] =
    "ar_SA,cs_CZ,da_DK,de_DE,el_GR,en_AU,en_CA,en_GB,en_US,es_ES,es_MX,fi_FI,fr_CA,fr_FR,hu_HU,it_IT,ja_JP,ko_KR,nl_NL,no_NO,pl_PL,pt_BR,pt_PT,ru_RU,sv_SE,th_TH,tr_TR,zh_CN,zh_HK,zh_TW";
SelectorResolution empty_unrecognized() noexcept {
    return {preloader_l::config::literal_value(""), false, false};
}
class GameInfoView final {
    const ConfigSnapshot& snapshot_;
    preloader_l::config::ValueView configuration_or_default(const char* key,const char* fallback,bool* used_fallback) noexcept {
        const bool name=std::string_view(key)=="GameName",version=std::string_view(key)=="GameVersion";
        for(auto i=snapshot_.leaves.rbegin();i!=snapshot_.leaves.rend();++i)
            if(i->path.size()==1 && i->path[0]=="Game" && (i->key==key || (name && i->key=="Name") || (version && i->key=="Version"))) {
                if(used_fallback)*used_fallback=false;
                return {i->raw_value.data(),i->raw_value.size(),true};
            }
        if(used_fallback)*used_fallback=true;
        return preloader_l::config::literal_value(fallback);
    }
    preloader_l::config::ValueView installed_language_policy(bool* used_fallback) noexcept {
        return settings_language_value(snapshot_,used_fallback);
    }
    SelectorResolution resolve_get_game_info_selector(preloader_l::xml::TextSpan selector) noexcept;
    Span gameinfo_span(const char* key) noexcept;
public:
    explicit GameInfoView(const ConfigSnapshot& snapshot) noexcept:snapshot_(snapshot){}
    HandlerStatus handle_get_game_info(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
    HandlerStatus handle_get_all_game_info(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
};
SelectorResolution GameInfoView::resolve_get_game_info_selector(preloader_l::xml::TextSpan selector) noexcept {
    if (eq(selector, "DISPLAY_NAME")) {
        bool fallback = false;
        const auto value = configuration_or_default("GameName", "Game", &fallback);
        return {value, true, fallback};
    }
    if (eq(selector, "INSTALLED_VERSION") || eq(selector, "AVAILABLE_VERSION")) {
        bool fallback = false;
        // Phase x001 closes AVAILABLE_VERSION as a post-config copy of the
        // final INSTALLED_VERSION value, so both selectors share one policy.
        const auto value = configuration_or_default("GameVersion", "1.0.0.0", &fallback);
        return {value, true, fallback};
    }
    if (eq(selector, "UPTODATE"))
        return {preloader_l::config::literal_value("true"), true, false};
    if (eq(selector, "FULLGAME_IS_RELEASED"))
        return {preloader_l::config::literal_value("true"), true, false};
    if (eq(selector, "FULLGAME_RELEASE_DATE"))
        return {preloader_l::config::literal_value("2010-01-01T00:00:00"), true, false};
    if (eq(selector, "FULLGAME_PURCHASED"))
        return {preloader_l::config::literal_value("true"), true, false};
    if (eq(selector, "FREETRIAL"))
        return {preloader_l::config::literal_value("false"), true, false};
    if (eq(selector, "EXPIRATION"))
        return {preloader_l::config::literal_value("0000-00-00T00:00:00"), true, false};
    if (eq(selector, "ENTITLEMENT_SOURCE"))
        return {preloader_l::config::literal_value("ORIGIN"), true, false};
    if (eq(selector, "MAX_GROUP_SIZE"))
        return {preloader_l::config::literal_value("16"), true, false};
    if (eq(selector, "EXPIRATION_DURATION"))
        return {preloader_l::config::literal_value("36000000"), true, false};
    if (eq(selector, "LANGUAGES")) {
        bool fallback = false;
        const auto value = configuration_or_default("Languages", kLanguagesFallback, &fallback);
        return {value, true, fallback};
    }
    if (eq(selector, "INSTALLED_LANGUAGE")) {
        bool fallback = false;
        const auto value = installed_language_policy(&fallback);
        return {value, true, fallback};
    }
    return empty_unrecognized();
}

Span GameInfoView::gameinfo_span(const char* key) noexcept {
    const preloader_l::xml::TextSpan selector{key,slen(key)};
    const auto resolution=resolve_get_game_info_selector(selector);
    return resolution.recognized && resolution.value.present
        ? Span{resolution.value.data,resolution.value.size}
        : Span{nullptr,0};
}

HandlerStatus GameInfoView::handle_get_game_info(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    if(!eq(r.command,"GetGameInfo")){if(written)*written=0;return HandlerStatus::wrong_command;}
    preloader_l::xml::TextSpan selector{};
    const auto selector_status=preloader_l::xml::find_attribute(r.command_open_tag,"GameInfoId",&selector);
    if(selector_status==preloader_l::xml::AttributeStatus::malformed||selector_status==preloader_l::xml::AttributeStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
    if(selector_status!=preloader_l::xml::AttributeStatus::found){if(written)*written=0;return HandlerStatus::unsupported;}
    // Phase x002 corrects the old representation label: behavioral GetGameInfo
    // consumes the required GameInfoId XML attribute directly. Phase x001's
    // complete 14-key policy remains unchanged; true misses stay non-success.
    preloader_l::xml::SemanticValue semantic_selector;
    const auto semantic_status=semantic_selector.assign_parsed(selector);
    if(semantic_status==preloader_l::xml::ValueCodecStatus::malformed_entity||semantic_status==preloader_l::xml::ValueCodecStatus::invalid_character||semantic_status==preloader_l::xml::ValueCodecStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
    if(semantic_status!=preloader_l::xml::ValueCodecStatus::ok){if(written)*written=0;return HandlerStatus::response_error;}
    const auto resolution=resolve_get_game_info_selector(semantic_selector.view());
    if(!resolution.recognized||!resolution.value.present){if(written)*written=0;return HandlerStatus::unsupported;}
    const Span value{resolution.value.data,resolution.value.size};
    const Span p[]={lit(" <LSX>\n  <Response id=\""),source_span(r.id),lit("\" sender=\""),response_sender_span(r),lit("\">\n    <GetGameInfoResponse GameInfo=\""),semantic_span(value),lit("\"/>\n  </Response>\n </LSX>\n")};
    return format_parts(r,p,sizeof(p)/sizeof(p[0]),out,cap,written);
}

HandlerStatus GameInfoView::handle_get_all_game_info(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    if(!eq(r.command,"GetAllGameInfo")){if(written)*written=0;return HandlerStatus::wrong_command;}
    const Span free_trial=gameinfo_span("FREETRIAL");
    const Span release_date=gameinfo_span("FULLGAME_RELEASE_DATE");
    const Span langs=gameinfo_span("LANGUAGES");
    const Span up_to_date=gameinfo_span("UPTODATE");
    const Span purchased=gameinfo_span("FULLGAME_PURCHASED");
    const Span max_group=gameinfo_span("MAX_GROUP_SIZE");
    const Span installed_version=gameinfo_span("INSTALLED_VERSION");
    const Span installed_language=gameinfo_span("INSTALLED_LANGUAGE");
    const Span display_name=gameinfo_span("DISPLAY_NAME");
    const Span released=gameinfo_span("FULLGAME_IS_RELEASED");
    const Span expiration=gameinfo_span("EXPIRATION");
    const Span available_version=gameinfo_span("AVAILABLE_VERSION");
    const Span entitlement_source=gameinfo_span("ENTITLEMENT_SOURCE");
    char system_time[20]{}; size_t system_time_n=0;
    if(!preloader_l::runtime::current_utc_timestamp(system_time,sizeof(system_time),&system_time_n)){if(written)*written=0;return HandlerStatus::response_error;}
    const Span p[]={
        lit(" <LSX>\n  <Response id=\""),source_span(r.id),lit("\" sender=\""),response_sender_span(r),
        lit("\">\n    <GetAllGameInfoResponse FreeTrial=\""),semantic_span(free_trial),
        lit("\" FullGameReleaseDate=\""),semantic_span(release_date),
        lit("\" Languages=\""),semantic_span(langs),
        lit("\" UpToDate=\""),semantic_span(up_to_date),
        lit("\" FullGamePurchased=\""),semantic_span(purchased),
        lit("\" MaxGroupSize=\""),semantic_span(max_group),
        lit("\" HasExpiration=\"false\" InstalledVersion=\""),semantic_span(installed_version),
        lit("\" InstalledLanguage=\""),semantic_span(installed_language),
        lit("\" SystemTime=\""),semantic_span(system_time,system_time_n),lit("\" DisplayName=\""),semantic_span(display_name),
        lit("\" FullGameReleased=\""),semantic_span(released),
        lit("\" Expiration=\""),semantic_span(expiration),
        lit("\" AvailableVersion=\""),semantic_span(available_version),
        lit("\" EntitlementSource=\""),semantic_span(entitlement_source),
        lit("\"/>\n  </Response>\n </LSX>\n")};
    return format_parts(r,p,sizeof(p)/sizeof(p[0]),out,cap,written);
}

} // namespace
ResponseResult dispatch_game_info_response(const xml::Request& request,const ConfigSnapshot& snapshot,char* output,std::size_t capacity,std::size_t* written) noexcept {
    if(!written)return ResponseResult::rejected;
    *written=0;if(!output || !capacity)return ResponseResult::rejected;
    GameInfoView view{snapshot};HandlerStatus status;
    if(preloader_l::xml::span_equals(request.command,"GetGameInfo"))status=view.handle_get_game_info(request,output,capacity,written);
    else if(preloader_l::xml::span_equals(request.command,"GetAllGameInfo"))status=view.handle_get_all_game_info(request,output,capacity,written);
    else return ResponseResult::no_response;
    if(status==HandlerStatus::unsupported || status==HandlerStatus::no_response_required){*written=0;return ResponseResult::no_response;}
    if(status!=HandlerStatus::ok || *written>=capacity){*written=0;return ResponseResult::rejected;}
    return ResponseResult::response;
}
}
