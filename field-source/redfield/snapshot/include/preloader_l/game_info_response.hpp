// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/config.hpp"
#include "preloader_l/response_dispatch.hpp"
namespace preloader_l {
// Stack context borrows one immutable snapshot until synchronous formatting
// completes. The transport owns the resulting bytes for asynchronous delivery.
ResponseResult dispatch_game_info_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
