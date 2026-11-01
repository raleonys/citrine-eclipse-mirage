// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/config.hpp"
#include "preloader_l/response_dispatch.hpp"
namespace preloader_l {
// Completed response bytes own the path after synchronous snapshot formatting.
ResponseResult dispatch_image_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
