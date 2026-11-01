#include "preloader_l/config/runtime_configuration_file.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include <cstdio>
#include <cstring>
using namespace preloader_l;
static bool write_file(const char* path,const char* bytes,size_t size){std::FILE*f=std::fopen(path,"wb");if(!f)return false;bool ok=std::fwrite(bytes,1,size,f)==size;return std::fclose(f)==0&&ok;}
int main(){
 const char* good="/tmp/preloader_l_surge_good.cfg";
 const char* bad="/tmp/preloader_l_surge_bad.cfg";
 constexpr char cfg[]=R"CFG("Config2" { "Game" { "Language" "de_DE" } "User" { "Username" "SURGE" } })CFG";
 if(!write_file(good,cfg,sizeof(cfg)-1))return 1;
 if(config::load_runtime_configuration_file(good)!=config::RuntimeConfigurationFileStatus::ok)return 2;
 auto language=config::runtime_configuration_setting("Language");
 if(!language.present||language.size!=5||std::memcmp(language.data,"de_DE",5)!=0)return 3;
 constexpr char malformed[]=R"CFG("Config2" { "Game" { "Language" "fr_FR" })CFG";
 if(!write_file(bad,malformed,sizeof(malformed)-1))return 4;
 if(config::load_runtime_configuration_file(bad)!=config::RuntimeConfigurationFileStatus::invalid_cfg)return 5;
 language=config::runtime_configuration_setting("Language");
 if(!language.present||language.size!=5||std::memcmp(language.data,"de_DE",5)!=0)return 6;
 if(config::load_runtime_configuration_file("/tmp/preloader_l_missing_surge.cfg")!=config::RuntimeConfigurationFileStatus::not_found)return 7;
 std::remove(good);std::remove(bad);return 0;
}
