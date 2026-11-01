#include "defaults.hpp"

namespace anadius::configuration_defaults {
configuration::State make_configuration_defaults_036d20(){
    // All modeled empty paths/strings/containers and zero scalar fields are
    // backed by writes within036d20. This is not a guessed blanket zero seed.
    configuration::State state{};
    state.content_id="0";                         // a3af8, ctor+118
    state.username="anadius";                     // a3bd8, ctor+1f8
    state.persona_id="1144668899";                // a3bf8, ctor+218
    state.user_id="1000200030000";                // a3c18, ctor+238
    state.fake_auth=1;state.pretend_connected=1;    // packed+258 =00010100
    state.sdk="10,5,96,47512";                    // a3c98, ctor+2b8
    state.country="US";state.currency="USD";
    state.is_subscriber="true";state.is_steam_subscriber="false";
    state.subscriber_level="2";state.is_under_age="false";
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
} // namespace anadius::configuration_defaults
