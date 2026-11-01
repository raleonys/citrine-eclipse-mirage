// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/config.hpp"
#include "preloader_l/response_dispatch.hpp"
namespace preloader_l {
// Snapshot and owned request DOM stay pinned through validation/measure/render.
// Output is fully materialized before transport takes asynchronous ownership.
ResponseResult dispatch_query_presence_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
