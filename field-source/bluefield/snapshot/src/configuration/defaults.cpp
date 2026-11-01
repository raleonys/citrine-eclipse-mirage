// SPDX-License-Identifier: GPL-3.0-only
#include "src/configuration/defaults.hpp"
namespace preloader_l::configuration {
State initial_state() {
    State state{};
    state.content_id="0";
    // User-selected display name; keep the source numeric IDs stable.
    state.username="SURGE";
    state.persona_id="1144668899";
    state.user_id="1000200030000";
    state.fake_auth=1;
    state.pretend_connected=1;
    state.sdk="10,5,96,47512";
    state.country="US";
    state.currency="USD";
    state.is_subscriber="true";
    state.is_steam_subscriber="false";
    state.subscriber_level="2";
    state.is_under_age="false";
    state.emulator_attributes={
        {"ENVIRONMENT","production"},
        {"IS_IGO_AVAILABLE","false"},
        {"IS_TELEMETRY_ENABLED","false"},
        {"IS_MANUAL_OFFLINE","false"},
        {"LANGUAGE","en_US"},
        {"IS_IGO_ENABLED","false"}
    };
    state.game_attributes={
        {"DISPLAY_NAME","Game"},
        {"INSTALLED_VERSION","1.0.0.0"},
        {"AVAILABLE_VERSION","1.0.0.0"},
        {"UPTODATE","true"},
        {"FULLGAME_IS_RELEASED","true"},
        {"FULLGAME_RELEASE_DATE","2010-01-01T00:00:00"},
        {"FULLGAME_PURCHASED","true"},
        {"FREETRIAL","false"},
        {"EXPIRATION","0000-00-00T00:00:00"},
        {"ENTITLEMENT_SOURCE","ORIGIN"},
        {"MAX_GROUP_SIZE","16"},
        {"EXPIRATION_DURATION","36000000"},
        {"LANGUAGES","ar_SA,cs_CZ,da_DK,de_DE,el_GR,en_AU,en_CA,en_GB,en_US,es_ES,es_MX,fi_FI,fr_CA,fr_FR,hu_HU,it_IT,ja_JP,ko_KR,nl_NL,no_NO,pl_PL,pt_BR,pt_PT,ru_RU,sv_SE,th_TH,tr_TR,zh_CN,zh_HK,zh_TW"},
        {"INSTALLED_LANGUAGE","en_US"}
    };
    return state;
}
}
