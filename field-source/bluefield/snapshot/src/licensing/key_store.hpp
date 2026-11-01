// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/runtime/runtime_paths.hpp"
#include "src/configuration/state.hpp"
namespace preloader_l::licensing {
// Normal-flow reconstruction of input function 0x18002a830. The application
// commits after final profile/config assignment, before accepting requests.
void store_cipher_key(const configuration::State&,std::string_view key,const RuntimePaths&);
}
