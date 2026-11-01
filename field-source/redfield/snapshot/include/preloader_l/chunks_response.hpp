// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/response_dispatch.hpp"
namespace preloader_l {
// Parser-local installed projection, not a filesystem/download inventory query.
ResponseResult dispatch_chunks_response(const xml::Request&,char*,std::size_t,std::size_t*) noexcept;
}
