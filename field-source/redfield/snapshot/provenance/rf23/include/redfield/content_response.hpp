// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/response_dispatch.hpp"
namespace redfield {
// Config-owned typed records, not live filesystem/download/ownership discovery.
ResponseResult dispatch_content_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
