// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/xml.hpp"
namespace preloader_l {
enum class ResponseResult { response, no_response, rejected };
// Already parsed owned DOM. The context-free API preserves RF03 bindings.
ResponseResult dispatch_response(const xml::Request&,char*,std::size_t,std::size_t*) noexcept;
class AuthCodeSession;
struct ConfigSnapshot;
// Explicit session context is required for GetAuthCode, including its offline branch.
ResponseResult dispatch_response_with_session(const xml::Request&,const ConfigSnapshot&,AuthCodeSession&,char*,std::size_t,std::size_t*) noexcept;
ResponseResult dispatch_response_with_config(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
