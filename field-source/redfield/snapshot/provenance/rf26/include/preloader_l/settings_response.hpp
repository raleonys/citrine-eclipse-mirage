// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/config.hpp"
#include "preloader_l/response_dispatch.hpp"
#include "preloader_l/config/settings_values.hpp"
namespace preloader_l {
preloader_l::config::ValueView settings_language_value(const ConfigSnapshot&,bool* used_fallback=nullptr) noexcept;
// Caller retains one immutable config snapshot through this synchronous call.
ResponseResult dispatch_settings_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
