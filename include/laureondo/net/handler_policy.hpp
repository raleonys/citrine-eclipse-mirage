// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstddef>
namespace laureondo::net::handler_policy {
inline constexpr std::size_t workers=2;
inline constexpr std::size_t queued_jobs=128;
} // namespace laureondo::net::handler_policy
