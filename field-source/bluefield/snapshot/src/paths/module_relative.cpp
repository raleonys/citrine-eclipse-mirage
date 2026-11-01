// SPDX-License-Identifier: GPL-3.0-only
#include "src/paths/module_relative.hpp"
#ifndef GHC_WIN_DISABLE_AUTO_PREFIXES
#define GHC_WIN_DISABLE_AUTO_PREFIXES
#endif
#include <ghc/filesystem.hpp>
#include <stdexcept>
#include <utility>
namespace preloader_l::paths {
SiblingFile module_sibling(const std::filesystem::path& module_file,const std::filesystem::path& leaf) {
 const auto& name=leaf.native();
 if(name.empty() || name==std::filesystem::path(".").native() || name==std::filesystem::path("..").native())
  throw std::invalid_argument("module sibling requires a filename");
 for(const auto c:name)if(c==0 || c=='/' || c=='\\' || c==':')throw std::invalid_argument("module sibling requires a filename");
 auto expected=module_file.parent_path()/leaf;
 const auto candidate=ghc::filesystem::path(module_file.native(),ghc::filesystem::path::native_format)
  .parent_path()/ghc::filesystem::path(name,ghc::filesystem::path::native_format);
 // Path equality can ignore spelling differences; compare exact native units.
 if(candidate.native()!=expected.native())return {std::move(expected),SiblingRoute::preserved_native_spelling};
 return {std::filesystem::path(candidate.native()),SiblingRoute::gulrak};
}
}
