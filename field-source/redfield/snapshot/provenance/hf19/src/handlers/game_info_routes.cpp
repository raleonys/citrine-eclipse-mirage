#include "preloader_l/handlers/game_info_routes.hpp"
#include "route_response_support.hpp"

namespace preloader_l { namespace handlers {
HandlerStatus handle_get_game_info(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
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

HandlerStatus handle_get_all_game_info(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
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

} }
