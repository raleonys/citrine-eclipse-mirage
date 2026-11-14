// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <string_view>

namespace laureondo::protocol_identity {
// Advertised LSX Challenge identity only. Not a compiler/Windows SDK selector.
// Change only on an explicit owner request; CFG values cannot override it.
inline constexpr std::string_view sdk_version="10,6,2,31";
}
