// SPDX-License-Identifier: GPL-3.0-only
#include "src/application/providers.hpp"
#include "src/configuration/defaults.hpp"
#include "src/xml/reader.hpp"
#include "tests/support/io.hpp"
#include <chrono>
#include <iostream>

// The packet's 0192d0 body converts UTF-8, optionally joins the executable's
// parent, and returns that path. It never lexically removes dot components.
// This fixture isolates that spelling contract; it is not a Windows ABI oracle.
namespace {
using namespace preloader_l;
namespace fs=std::filesystem;
unsigned checks=0;
void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
std::string utf8(const fs::path& path){const auto s=path.u8string();return {reinterpret_cast<const char*>(s.data()),s.size()};}
struct Cleanup {fs::path root;~Cleanup(){std::error_code error;fs::remove_all(root,error);}};
}
int main(){try{
 const auto root=fs::temp_directory_path()/("preloader_l_resolve_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 Cleanup cleanup{root};fs::create_directories(root/"base");
 Environment env{RuntimePaths::fixture(root/"base"/"fixture.exe",root/"base"/"surge.cfg",root/"data",root/"programdata")};
 auto state=configuration::initial_state();achievement_storage::Paths paths;
 xml_reader::Backend xml;achievement_storage::HostCrtNumbers numbers;
 timestamps::ClassicIso parser;StorageEffects storage_effects{parser,env.paths};files::NativeFactory files{env.paths};
 achievement_storage::Storage storage{paths,xml,numbers,storage_effects,files};ConfigurationEffects effects{state,env,storage};
 const auto exe=model_path(env.executable);
 for(const std::string value:{"a/./b/../target.txt","./target.txt","a/../target.txt","../outside/./target.txt","a/./",".","..","plain.txt",""}){
  const auto expected_relative=model_path(root/"base"/fs::path(value));
  require(effects.resolve_path_0192d0(value,exe,true)==expected_relative,"relative path spelling retained");
  require(effects.resolve_path_0192d0(value,exe,false)==model_path(fs::path(value)),"non-relative mode retains input spelling");
 }
 const auto absolute=root/"outside"/"."/"a"/".."/"target.txt";
 for(bool relative:{false,true})require(effects.resolve_path_0192d0(utf8(absolute),exe,relative)==model_path(absolute),"absolute path spelling retained");
 const std::string unicode="a/../\xC3\xA7\xC4\xB1\xC4\x9F/./file.txt";
 require(effects.resolve_path_0192d0(unicode,exe,false)==utf8_path(unicode),"UTF-8 spelling retained");
 const std::string nul{"a\0b/../file",11};
 require(effects.resolve_path_0192d0(nul,exe,false)==utf8_path(nul),"explicit-length input unchanged");
#ifdef _WIN32
 for(const std::string value:{"C:\\a\\.\\b\\..\\file","\\\\server\\share\\a\\..\\file","\\\\?\\C:\\a\\..\\file","C:/a/./file"})
  require(effects.resolve_path_0192d0(value,u"C:\\base\\fixture.exe",true)==utf8_path(value),"Windows absolute spelling retained");
 std::cout<<"Windows directory-link semantic fixture: NOT_RUN (no privilege assumption)\n";
#else
 // A real filesystem counterexample: link/.. follows the link target's parent.
 // Removing those components before the read selects a different existing file.
 fs::create_directories(root/"outside"/"child");fs::create_directory_symlink(root/"outside"/"child",root/"base"/"link");
 write_file(model_path(root/"outside"/"picked.txt"),"SOURCE_TARGET");
 write_file(model_path(root/"base"/"picked.txt"),"NORMALIZED_TARGET");
 const auto resolved=effects.resolve_path_0192d0("link/../picked.txt",exe,true);
 require(read_file(resolved,true)=="SOURCE_TARGET","link-parent read selects source target");
 require(read_file(model_path(native_path(resolved).lexically_normal()),true)=="NORMALIZED_TARGET","normalization counterexample is discriminating");
 require(fs::equivalent(native_path(resolved),root/"outside"/"picked.txt"),"resolved path reaches target parent");
 require(!fs::equivalent(native_path(resolved),root/"base"/"picked.txt"),"resolved path differs from normalized target");
 std::cout<<"POSIX directory-link semantic fixture: PASS; Windows native path/ABI: NOT_RUN\n";
#endif
 std::cout<<"PASS path resolution checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"FAIL path resolution: "<<error.what()<<'\n';return 1;}}
