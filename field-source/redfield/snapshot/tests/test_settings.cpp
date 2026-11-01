// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/config/settings_values.hpp"
using namespace preloader_l::config;
int main(){
 const auto none=missing_value();
 if(!online_enabled(none))return 1;
 if(online_enabled(literal_value("false")))return 2;
 if(!online_enabled(literal_value("true")))return 3;
 if(!online_enabled(literal_value("0")))return 4; // not an alias; default-enabled
 if(!online_enabled(literal_value("1")))return 5; // not an alias; default-enabled
 if(!online_enabled(literal_value("garbage")))return 6;
 if(behavioral_config_boolean(none,true))return 8;
 if(behavioral_config_boolean(literal_value(""),true))return 9;
 if(!behavioral_config_boolean(literal_value("true"),true))return 10;
 if(!behavioral_config_boolean(literal_value("1"),true))return 11;
 if(behavioral_config_boolean(literal_value("false"),true))return 12;
 if(behavioral_config_boolean(literal_value("0"),true))return 13;
 if(!behavioral_config_boolean(literal_value("garbage"),true))return 14;
 if(behavioral_config_boolean(literal_value("garbage"),false))return 15;
 return 0;
}
