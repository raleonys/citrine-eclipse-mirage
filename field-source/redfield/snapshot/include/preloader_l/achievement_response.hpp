// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/response_dispatch.hpp"
namespace preloader_l {
// Typed records from an immutable snapshot. LocalService supplies its current
// committed AchievementSession snapshot; config-only dispatch uses its caller's.
ResponseResult dispatch_achievement_response(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
}
