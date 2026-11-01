// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/response_dispatch.hpp"
namespace preloader_l {
// Local Config2 projection/filtering, not remote account ownership verification.
ResponseResult dispatch_entitlement_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
