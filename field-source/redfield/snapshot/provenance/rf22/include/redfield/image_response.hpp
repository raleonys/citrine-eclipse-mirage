// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config.hpp"
#include "redfield/response_dispatch.hpp"
namespace redfield {
// Completed response bytes own the path after synchronous snapshot formatting.
ResponseResult dispatch_image_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
