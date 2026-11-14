// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/runtime/application.hpp"

namespace laureondo::application {
configuration::State initial_state() {
    // Documented RE constructor FUN_180036d20 (43881ff), verified
    // against the corresponding assembly. These are application values only;
    // no private-CRT data/initializer/container ABI is imported.
    configuration::State s{};
    // Zero means not yet bound; the listener always selects its owned range.
    s.server_port=0;
    s.content_id="0";
    s.username="SURGE"; // Explicit owner branding over original constructor name.
    s.persona_id="1144668899";s.user_id="1000200030000";
    s.pretend_connected=1;s.fake_auth=1;
    s.country="US";s.currency="USD";
    s.is_subscriber="true";s.is_steam_subscriber="false";
    s.subscriber_level="2";s.is_under_age="false";
    s.emulator_attributes.emplace("ENVIRONMENT","production");
    s.emulator_attributes.emplace("IS_IGO_AVAILABLE","false");
    s.emulator_attributes.emplace("IS_TELEMETRY_ENABLED","false");
    s.emulator_attributes.emplace("IS_MANUAL_OFFLINE","false");
    s.emulator_attributes.emplace("LANGUAGE","en_US");
    s.emulator_attributes.emplace("IS_IGO_ENABLED","false");
    s.game_attributes.emplace("DISPLAY_NAME","Game");
    s.game_attributes.emplace("INSTALLED_VERSION","1.0.0.0");
    s.game_attributes.emplace("AVAILABLE_VERSION","1.0.0.0");
    s.game_attributes.emplace("UPTODATE","true");
    s.game_attributes.emplace("FULLGAME_IS_RELEASED","true");
    s.game_attributes.emplace("FULLGAME_RELEASE_DATE","2010-01-01T00:00:00");
    s.game_attributes.emplace("FULLGAME_PURCHASED","true");
    s.game_attributes.emplace("FREETRIAL","false");
    s.game_attributes.emplace("EXPIRATION","0000-00-00T00:00:00");
    s.game_attributes.emplace("ENTITLEMENT_SOURCE","ORIGIN");
    s.game_attributes.emplace("MAX_GROUP_SIZE","16");
    s.game_attributes.emplace("EXPIRATION_DURATION","36000000");
    s.game_attributes.emplace("LANGUAGES","ar_SA,cs_CZ,da_DK,de_DE,el_GR,en_AU,en_CA,en_GB,en_US,es_ES,es_MX,fi_FI,fr_CA,fr_FR,hu_HU,it_IT,ja_JP,ko_KR,nl_NL,no_NO,pl_PL,pt_BR,pt_PT,ru_RU,sv_SE,th_TH,tr_TR,zh_CN,zh_HK,zh_TW");
    s.game_attributes.emplace("INSTALLED_LANGUAGE","en_US");
    return s;
}
}
