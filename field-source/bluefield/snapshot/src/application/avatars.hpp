// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/application/environment.hpp"
namespace preloader_l {
// Existing app/module images and explicit custom paths are read-only.
// The official embedded image is a missing-file fallback at SURGE/surge.png.
class AvatarStore final {
 const RuntimePaths& paths_;
public:
 explicit AvatarStore(const RuntimePaths& paths):paths_(paths){}
 Path logical_default() const{return paths_.default_avatar().u16string();}
 Path resolve(const Path& requested);
};
}
