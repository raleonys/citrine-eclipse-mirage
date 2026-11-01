// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/response_dispatch.hpp"
namespace preloader_l {
// Config-owned typed records, not live filesystem/download/ownership discovery.
ResponseResult dispatch_content_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
