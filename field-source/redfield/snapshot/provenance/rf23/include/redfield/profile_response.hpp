// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/handlers/handler_status.hpp"
#include "redfield/config.hpp"
#include "redfield/response_dispatch.hpp"
#include "redfield/config/settings_values.hpp"
namespace redfield {
using redfield::config::ValueView;
struct ProfilePolicyView {
    ValueView username;
    ValueView persona_id;
    ValueView user_id;
    ValueView country;
    ValueView currency;
    ValueView is_subscriber;
    ValueView is_steam_subscriber;
    ValueView subscriber_level;
    ValueView is_under_age;
};
ProfilePolicyView profile_policy(const ConfigSnapshot&) noexcept;
ValueView presence_title(const ConfigSnapshot&) noexcept;
redfield::handlers::HandlerStatus format_profile_response(const xml::Request&,const ProfilePolicyView&,char*,std::size_t,std::size_t*) noexcept;
redfield::handlers::HandlerStatus format_presence_response(const xml::Request&,ValueView,char*,std::size_t,std::size_t*) noexcept;
ResponseResult dispatch_profile_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
