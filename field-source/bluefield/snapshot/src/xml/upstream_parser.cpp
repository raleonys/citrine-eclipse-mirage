// SPDX-License-Identifier: GPL-3.0-only
// Compile the exact pinned upstream implementation once through this adapter.
// Namespace lookup selects the packet's parse-time depth=100 without modifying
// third_party files, generating a fork, or changing public header definitions.
#include "third_party/tinyxml2/tinyxml2.h"
static_assert(::TINYXML2_MAX_ELEMENT_DEPTH==500,"review the adapter when upstream changes");
namespace tinyxml2 {
constexpr int TINYXML2_MAX_ELEMENT_DEPTH=100;
}
#include "third_party/tinyxml2/tinyxml2.cpp"
