// SPDX-License-Identifier: GPL-3.0-only
#include "src/runtime/runtime_paths.hpp"
#include "src/paths/module_relative.hpp"
#include <stdexcept>
#include "src/application/environment.hpp"
#ifdef _WIN32
#include <windows.h>
#endif
namespace preloader_l {
namespace fs=std::filesystem;
namespace {
void validate_root(const fs::path& p){if(p.empty()||!p.is_absolute()||p.native().find(fs::path::value_type(0))!=fs::path::string_type::npos)throw std::invalid_argument("invalid runtime root");}
bool within(const fs::path& p,const fs::path& base){auto n=p.lexically_normal(),r=base.lexically_normal();auto a=n.begin(),b=r.begin();for(;b!=r.end();++b,++a)if(a==n.end()||*a!=*b)return false;return true;}
bool reparse(const fs::path& p){
#ifdef _WIN32
 const auto a=GetFileAttributesW(p.c_str());return a!=INVALID_FILE_ATTRIBUTES&&(a&FILE_ATTRIBUTE_REPARSE_POINT)!=0;
#else
 std::error_code ec;return fs::is_symlink(fs::symlink_status(p,ec));
#endif
}
}
RuntimePaths::RuntimePaths(fs::path module,fs::path exe,fs::path local,fs::path program,fs::path config):module_(std::move(module)),executable_(std::move(exe)),local_(std::move(local)),data_(local_/"SURGE"),program_(std::move(program)),config_(std::move(config)){
 validate_root(module_);validate_root(executable_);validate_root(local_);validate_root(program_);validate_root(config_);
 std::error_code ec;const auto status=fs::status(local_,ec);
 if(ec&&status.type()!=fs::file_type::not_found)throw fs::filesystem_error("runtime local root",local_,ec);
 if(fs::exists(status)&&!fs::is_directory(status))throw std::invalid_argument("runtime local root is not a directory");
 const auto data_status=fs::status(data_,ec);if(fs::exists(data_status)&&!fs::is_directory(data_status))throw std::invalid_argument("runtime data root is not a directory");
 if(reparse(data_))throw std::invalid_argument("runtime data root cannot redirect");
}
RuntimePaths RuntimePaths::from_system(fs::path m,fs::path e,fs::path l,fs::path p){auto c=paths::module_sibling(m,"surge.cfg").value;return RuntimePaths(std::move(m),std::move(e),std::move(l),std::move(p),std::move(c));}
RuntimePaths RuntimePaths::fixture(fs::path e,fs::path c,fs::path l,fs::path p){auto m=c.parent_path()/"preloader_l.dll";return RuntimePaths(std::move(m),std::move(e),std::move(l),std::move(p),std::move(c));}
fs::path RuntimePaths::override_config()const{return paths::module_sibling(config_,"surge_override.cfg").value;}
fs::path RuntimePaths::provider()const{return paths::module_sibling(module_,provider_filename).value;}
fs::path RuntimePaths::sidecar_avatar()const{return paths::module_sibling(module_,"surge.png").value;}
fs::path RuntimePaths::default_avatar()const{return data_/"surge.png";}
fs::path RuntimePaths::profile_cache()const{return data_;}
fs::path RuntimePaths::diagnostic_log()const{return data_/"surge.log";}
fs::path RuntimePaths::file(RuntimeFile f)const{switch(f){case RuntimeFile::data_root:return data_;case RuntimeFile::base_config:return config_;case RuntimeFile::override_config:return override_config();case RuntimeFile::cookie:return profile_cache()/"remid_cookie";case RuntimeFile::last_profile:return profile_cache()/"last_profile";case RuntimeFile::last_avatar:return profile_cache()/"last_avatar";case RuntimeFile::default_avatar:return default_avatar();}throw std::logic_error("runtime file kind");}
fs::path RuntimePaths::avatar_cache(std::string_view owner,std::string_view width)const{
 // Preserve the source filename spelling, including the original Width text.
 const auto component=filesystem_path("_AVATAR_"+std::string(owner)+"_"+std::string(width));
 return data_/fs::path(component);
}
bool RuntimePaths::owns(const fs::path& p)const{return p.is_absolute()&&within(p,data_);}
bool RuntimePaths::external_contract(const fs::path& p)const{
 const auto license=program_/"Electronic Arts"/"EA Services"/"License";
 if(within(p,license))return p.extension()==fs::path(".dlf");
 const auto name=p.filename().native();
 return p.parent_path().lexically_normal()==executable_.parent_path().lexically_normal()
     &&name.starts_with(fs::path("Denuvo_ticket_").native())&&name.ends_with(fs::path(".txt").native());
}

void RuntimePaths::require_write(const fs::path& original)const{
 if(original.native().find(fs::path::value_type(0))!=fs::path::string_type::npos)throw std::invalid_argument("NUL runtime path");
 const auto p=original.lexically_normal();
 if(!owns(p)&&!external_contract(p))throw std::invalid_argument("unowned runtime write");
 if(owns(p)){auto c=data_.lexically_normal();for(;;){if(reparse(c))throw std::invalid_argument("runtime path redirects");if(c==p)break;const auto relative=p.lexically_relative(c);if(relative.empty())throw std::invalid_argument("runtime ancestry");c/=*relative.begin();}}
 else if(reparse(p))throw std::invalid_argument("external target redirects");
}
}
