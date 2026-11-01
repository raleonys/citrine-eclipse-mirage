// SPDX-License-Identifier: GPL-3.0-only
#include "src/application/environment.hpp"
#include "src/paths/encoding.hpp"
#include <fstream>
#include "src/files/atomic_io.hpp"
#include <stdexcept>
#include <vector>
#include <limits>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#include <unistd.h>
#endif
namespace preloader_l {
std::filesystem::path native_path(const Path& path){
#ifdef _WIN32
 return std::filesystem::path(path);
#else
 auto normalized=path;for(auto& c:normalized)if(c==u'\\')c=u'/';return std::filesystem::path(normalized);
#endif
}
Path model_path(const std::filesystem::path& path){return path.u16string();}
#ifndef _WIN32
Path HostPathEffects::parent(const Path& value){return model_path(native_path(value).parent_path());}
Path HostPathEffects::filename(const Path& value){return model_path(native_path(value).filename());}
Path HostPathEffects::join(const Path& left,const Path& right){return model_path(native_path(left)/native_path(right));}
Path HostPathEffects::resolve(const Path& value,const Path& executable,bool relative){
 auto p=native_path(value);if(relative&&p.is_relative())p=native_path(executable).parent_path()/p;return model_path(p);
}
#endif
Path utf8_path(std::string_view text){return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(text.data()),text.size())).u16string();}
Path filesystem_path(std::string_view text){
#ifdef _WIN32
 encoding::NativeCalls calls;encoding::PortableErrors errors;
 return encoding::filesystem_widen_006d70(text,calls,errors);
#else
 return utf8_path(text);
#endif
}
std::string path_utf8(const Path& path){
#ifdef _WIN32
 encoding::NativeCalls calls;encoding::PortableErrors errors;
 return encoding::path_utf8_01e3f0(path,calls,errors);
#else
 auto bytes=native_path(path).u8string();return {reinterpret_cast<const char*>(bytes.data()),bytes.size()};
#endif
}
status_recovered::Status file_status(const Path& path,std::uint32_t flags){
 if(flags!=3)throw std::invalid_argument("status flags");
#ifdef _WIN32
 status_recovered::NativeStats stats;
 try{return status_recovered::status_01ea20(path,flags,stats);}
 catch(const status_recovered::OutOfEvidenceStatus&){
  // The packet leaves flags=3 reparse-tag data explicitly unknown. Preserve
  // the previous adapter only in that domain; do not invent a tag or turn a
  // research-domain guard into a new production rejection policy.
 }
#endif
 std::error_code error;auto status=std::filesystem::status(native_path(path),error);
 if(status.type()==std::filesystem::file_type::not_found)return {1,0,static_cast<std::uint32_t>(error.value())};
 if(error)return {0,0,static_cast<std::uint32_t>(error.value())};
 return {std::filesystem::is_regular_file(status)?2:std::filesystem::is_directory(status)?3:9,0,0};
}
std::string read_file(const Path& path,bool required){
 std::ifstream in(native_path(path),std::ios::binary);
 if(!in){if(required)throw std::runtime_error("required file cannot be opened: "+path_utf8(path));return {};}
 in.seekg(0,std::ios::end);auto n=in.tellg();
 if(n<0)throw std::length_error("file position unavailable");
 std::string bytes(static_cast<size_t>(n),'\0');in.seekg(0);
 if(!bytes.empty())in.read(bytes.data(),static_cast<std::streamsize>(bytes.size()));return bytes;
}

void publish_environment(std::string_view name,std::string_view value){
 std::string k(name),v(value);
#ifdef _WIN32
 if(!SetEnvironmentVariableA(k.c_str(),v.c_str()))throw std::runtime_error("environment publication failed: "+k);
#else
 if(setenv(k.c_str(),v.c_str(),1))throw std::runtime_error("environment publication failed: "+k);
#endif
}
}
