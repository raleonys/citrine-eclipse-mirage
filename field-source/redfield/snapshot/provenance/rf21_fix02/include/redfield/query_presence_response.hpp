// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config.hpp"
#include "redfield/response_dispatch.hpp"
namespace redfield {
// Snapshot and owned request DOM stay pinned through validation/measure/render.
// Output is fully materialized before transport takes asynchronous ownership.
ResponseResult dispatch_query_presence_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
