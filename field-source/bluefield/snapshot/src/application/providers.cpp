// SPDX-License-Identifier: GPL-3.0-only
#include "src/application/providers.hpp"
#include "src/application/avatar_assets.hpp"
#include "src/files/license_files.hpp"
#include "src/profiles/online_session.hpp"
#include "src/licensing/key_store.hpp"
#include <array>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#endif
namespace preloader_l {
status_recovered::Status StorageEffects::status_01ea20(const Path& p,std::uint32_t flags){return file_status(p,flags);}
[[noreturn]] void StorageEffects::filesystem_error_018130(std::string_view op,std::uint32_t n,const Path&){throw SessionFailure(std::string(op)+" failed: "+std::to_string(n));}
Path ConfigurationEffects::resolve_path_0192d0(std::string_view value,const Path& executable,bool relative){
 return PathEffects::resolve(utf8_path(value),executable,relative);
}
configuration::Node ConfigurationEffects::load_document(const Path& path){
 parser::State parsed;const auto bytes=read_file(path,true);(void)parser::parse(bytes,parsed);
 return config_hash::to_configuration(config_hash::from_parser(parsed.root));
}
void ConfigurationEffects::apply_license_key_02a830(std::string_view key){
 licensing::store_cipher_key(state_,key,env_.paths);
}
void ConfigurationEffects::write_embedded_png_03c020(const Path& path,configuration::EmbeddedPng image){
 (void)image;atomic_write(native_path(path),{reinterpret_cast<const char*>(assets::surge_png),sizeof assets::surge_png},&env_.paths,true);
}
std::vector<Path> ConfigurationEffects::directory_entries(const Path& root){std::vector<Path> paths;for(const auto& entry:std::filesystem::directory_iterator(native_path(root)))paths.push_back(model_path(entry.path()));return paths;}
bool ContentEffects::online_avatar_virtual_28(const Path& path,std::string_view owner,std::int32_t size){
 env_.paths.require_write(native_path(path));
 std::filesystem::create_directories(native_path(path).parent_path());
 return online_.avatar(path,owner,size);
}

const char* LicenseEffects::online_auth_virtual_30(const char* client){return online_.auth(client);}
licensing::FileStatus LicenseEffects::status_01ea20(const Path& path,std::uint32_t flags){auto status=file_status(path,flags);return {status.kind,static_cast<int32_t>(status.error)};}
[[noreturn]] void LicenseEffects::filesystem_error_018130(std::string_view op,std::int32_t n,const Path&){throw SessionFailure(std::string(op)+" failed: "+std::to_string(n));}
bool LicenseEffects::file_hash_matches_042d20(const Path& path,std::string expected){files::NativeFactory f;files::NativeTextEnvironment text;files::LicenseFiles actual(*this,f,text);return actual.file_hash_matches_042d20(path,std::move(expected));}
std::string LicenseEffects::load_token_02afa0(const Path& path){files::NativeFactory f{env_.paths};files::NativeTextEnvironment text;files::LicenseFiles actual(*this,f,text);return actual.load_token_02afa0(path);}
bool LicenseEffects::wine_detected_045010(){
#ifdef _WIN32
 auto ntdll=GetModuleHandleW(L"ntdll.dll");return ntdll&&GetProcAddress(ntdll,"wine_get_version");
#else
 return false;
#endif
}
void LicenseEffects::dialog_038bc0(std::string_view title,std::string_view body,std::uint32_t flags){
 env_.log(LogEvent::phase);
#ifdef _WIN32
 const std::string t(title),b(body);MessageBoxA(nullptr,b.c_str(),t.c_str(),flags);
#else
 (void)body;(void)flags;
#endif
}
}

namespace preloader_l {
void ContentEffects::persist_achievement(const Path& path,std::string_view xml,const content::AchievementMap& map){
 if(&map!=&state_.achievements)throw std::invalid_argument("achievement map ownership");
 std::vector<FileUpdate> writes;if(!path.empty())writes.push_back({native_path(path),std::string(xml)});
 if(!paths_.ini_path.empty())writes.push_back({native_path(paths_.ini_path),achievement_writer::serialize_ini(state_)});
 atomic_batch(writes,env_.paths);
}
}
