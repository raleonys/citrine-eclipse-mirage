// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config.hpp"
#include "redfield/response_dispatch.hpp"
namespace redfield {
// Stack context borrows one immutable snapshot until synchronous formatting
// completes. The transport owns the resulting bytes for asynchronous delivery.
ResponseResult dispatch_game_info_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
