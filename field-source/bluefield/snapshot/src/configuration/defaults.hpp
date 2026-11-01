// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/configuration/state.hpp"
namespace preloader_l::configuration {
// Logical state from input constructor 0x180036d20, before CLI and Config2.
// Owns its strings; no original MSVC object layout or runtime addresses escape.
State initial_state();
}
