// SPDX-License-Identifier: GPL-3.0-only
#include "src/application/avatars.hpp"
#include "src/application/avatar_assets.hpp"
#include "src/files/atomic_io.hpp"
namespace preloader_l {
Path AvatarStore::resolve(const Path& requested){
 if(requested!=logical_default())return requested;
 const auto file=paths_.default_avatar();
 // Keep module-sidecar priority from the prior resolver; remove only its rejected byte gate.
 if(std::filesystem::is_regular_file(paths_.sidecar_avatar()))return paths_.sidecar_avatar().u16string();
 if(std::filesystem::is_regular_file(file))return file.u16string();
 paths_.require_write(file);
 // Never replace an existing file. The embedded official bytes are a missing-file fallback.
 try{atomic_write(file,{reinterpret_cast<const char*>(assets::surge_png),sizeof assets::surge_png},&paths_,true);}
 catch(...){if(!std::filesystem::is_regular_file(file))throw;}
 return file.u16string();
}
}
