// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/profile_response.hpp"
#include "response_formatter_support.hpp"
namespace redfield {
using preloader_l::handlers::HandlerStatus;
HandlerStatus format_profile_response(const xml::Request& r,const ProfilePolicyView& profile,char* out,size_t cap,size_t* written) noexcept {
    if(!eq(r.command,"GetProfile")){if(written)*written=0;return HandlerStatus::wrong_command;}

    preloader_l::xml::TextSpan request_index{};
    const auto index_status=preloader_l::xml::find_attribute(r.command_open_tag,"index",&request_index);
    if(index_status==preloader_l::xml::AttributeStatus::malformed||index_status==preloader_l::xml::AttributeStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
    if(index_status!=preloader_l::xml::AttributeStatus::found){if(written)*written=0;return HandlerStatus::unsupported;}

    // Phase x002 corrects the old Phase-131 representation label: behavioral
    // GetProfile consumes the required index XML attribute directly and echoes
    // that value as UserIndex. No child fallback and no synthetic index="0".
    if(!safe_attr_value(request_index)){if(written)*written=0;return HandlerStatus::invalid_request;}

    const Span username{profile.username.data,profile.username.size};
    const Span persona_id{profile.persona_id.data,profile.persona_id.size};
    const Span user_id{profile.user_id.data,profile.user_id.size};
    const Span country{profile.country.data,profile.country.size};
    const Span currency{profile.currency.data,profile.currency.size};
    const Span is_subscriber{profile.is_subscriber.data,profile.is_subscriber.size};
    const Span is_steam_subscriber{profile.is_steam_subscriber.data,profile.is_steam_subscriber.size};
    const Span subscriber_level{profile.subscriber_level.data,profile.subscriber_level.size};
    const Span is_under_age{profile.is_under_age.data,profile.is_under_age.size};
    
    const Span attrs[]={country,currency,username,user_id,persona_id,is_subscriber,is_steam_subscriber,subscriber_level,is_under_age};
    for(const auto& v:attrs){ preloader_l::xml::TextSpan x{v.data,v.size}; if(!safe_semantic_attr_value(x)){if(written)*written=0;return HandlerStatus::unsupported;} }

    // Preserve the curio Phase 132 field set/order. Phase x002 supplies
    // exact request-attribute policy; Phase x003 supplies persistent profile
    // policy/default/config-overlay semantics.
    const Span p[]={
        lit(" <LSX>\n  <Response id=\""),source_span(r.id),lit("\" sender=\""),response_sender_span(r),
        lit("\">\n    <GetProfileResponse Country=\""),semantic_span(country),
        lit("\" GeoCountry=\""),semantic_span(country),
        lit("\" PersonaId=\""),semantic_span(persona_id),
        lit("\" IsTrialSubscriber=\"false\" SubscriberLevel=\""),semantic_span(subscriber_level),
        lit("\" Persona=\""),semantic_span(username),
        lit("\" UserId=\""),semantic_span(user_id),
        lit("\" CommerceCountry=\""),semantic_span(country),
        lit("\" CommerceCurrency=\""),semantic_span(currency),
        lit("\" AvatarId=\"\" IsSubscriber=\""),semantic_span(is_subscriber),
        lit("\" UserIndex=\""),source_span(request_index),
        lit("\" IsUnderAge=\""),semantic_span(is_under_age),
        lit("\" IsSteamSubscriber=\""),semantic_span(is_steam_subscriber),
        lit("\"/>\n  </Response>\n </LSX>\n")};
    return format_parts(r,p,sizeof(p)/sizeof(p[0]),out,cap,written);
}

HandlerStatus format_presence_response(const xml::Request& r,ValueView title,char* out,size_t cap,size_t* written) noexcept {
    if(!eq(r.command,"GetPresence")){if(written)*written=0;return HandlerStatus::wrong_command;}
    const Span rich{title.data,title.size};
    const Span p[]={lit(" <LSX>\n  <Response id=\""),source_span(r.id),lit("\" sender=\""),response_sender_span(r),lit("\">\n    <GetPresenceResponse Title=\"\" SessionId=\"\" MultiplayerId=\"\" GroupId=\"\" TitleId=\"\" Presence=\"INGAME\" GamePresence=\"\" Group=\"\" RichPresence=\""),semantic_span(rich),lit("\" UserId=\"0\"/>\n  </Response>\n </LSX>\n")};
    return format_parts(r,p,sizeof(p)/sizeof(p[0]),out,cap,written);
}

}
