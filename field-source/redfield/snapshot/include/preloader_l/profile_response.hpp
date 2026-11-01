// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/config.hpp"
#include "preloader_l/response_dispatch.hpp"
#include "preloader_l/config/settings_values.hpp"
namespace preloader_l {
using preloader_l::config::ValueView;
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
preloader_l::handlers::HandlerStatus format_profile_response(const xml::Request&,const ProfilePolicyView&,char*,std::size_t,std::size_t*) noexcept;
preloader_l::handlers::HandlerStatus format_presence_response(const xml::Request&,ValueView,char*,std::size_t,std::size_t*) noexcept;
ResponseResult dispatch_profile_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
