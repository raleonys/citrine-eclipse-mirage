#pragma once
#include <string_view>

namespace preloader_l::protocol_identity {
// Advertised LSX Challenge identity only. Not a compiler/Windows SDK selector.
// Change only on an explicit owner request; CFG values cannot override it.
inline constexpr std::string_view sdk_version="10,6,2,31";
}
