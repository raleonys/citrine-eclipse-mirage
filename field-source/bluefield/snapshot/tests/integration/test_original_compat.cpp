// SPDX-License-Identifier: GPL-3.0-only
#include "tests/support/io.hpp"
#include "src/application/application.hpp"
#include "src/transport/service.hpp"
#include "src/files/binary_io.hpp"
#include "src/crypto/crypto.hpp"
#include "src/tokens/constants.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
namespace fs=std::filesystem;
using namespace preloader_l;
namespace {int count=0;void check(bool v,const char* message){++count;if(!v)throw std::runtime_error(message);}
RuntimePaths paths(const fs::path& r){return RuntimePaths::fixture(r/"host.exe",r/"surge.cfg",r/"local",r/"program");}
void config_domain(const fs::path& root){
 auto p=paths(root/"large");std::string value(1024*1024+7,'A');write_file(p.config().u16string(),"\"Config2\"{\"Game\"{\"Name\" \""+value+"\"}}");Environment env{p};Application app(env);app.initialize();check(app.configuration().game_attributes.at("DISPLAY_NAME")==value,"source parser accepts config larger than invented 1MiB limit");
 auto q=paths(root/"deep");std::string text="\"Config2\"{";for(int i=0;i<140;++i)text+="\"Nested\"{";text+=std::string(141,'}');write_file(q.config().u16string(),text);Environment deep_env{q};Application deep(deep_env);deep.initialize();check(deep.configuration().username=="SURGE","source parser accepts deeper than invented 128 precheck");
}
void read_domain(const fs::path& root){auto p=paths(root);auto file=p.data()/"large.dat";fs::create_directories(file.parent_path());{std::ofstream f(file,std::ios::binary);f.seekp(64*1024*1024);f.put('Z');}auto bytes=read_file(file.u16string(),true);check(bytes.size()==64*1024*1024+1 && bytes.back()=='Z',"no invented general 64MiB read cap");
 auto short_file=p.data()/"short.dat";write_file(short_file.u16string(),"abcdef");files::NativeFactory factory{p};auto input=factory.open_input_039f70(short_file.u16string(),0x20,0x40);check(input->initial_state()==0,"native input opens");input->seek_03b610(0,files::Seek::end);auto n=input->position_sum_03ba50();input->seek_03b610(0,files::Seek::begin);fs::resize_file(short_file,2);std::string target(n,'\0');auto got=input->read_03b2a0({reinterpret_cast<std::uint8_t*>(target.data()),target.size()});check(got==2 && target==std::string("ab\0\0\0\0",6),"source short read preserves zero-filled tail without added throw");}
void ini_and_key_order(const fs::path& root){
 auto p=paths(root);const auto long_set=std::string(160,'a');write_file(p.config().u16string(),"\"Config2\"{\"Game\"{\"ContentId\" \"fixture\" \"KeyForLicense\" \"SYNTHETIC_TEST_ONLY\"}\"User\"{\"UserId\" \"changed-after-key\"}\"Achievements\"{\"AchievementsSet\" \""+long_set+"\" \"SteamAppId\" \"42\"}}");Environment env{p};Application app(env);app.initialize();
 check(app.configuration().user_id=="changed-after-key","User applied after Game key");auto license=p.program_data()/"Electronic Arts/EA Services/License/fixture.dlf";auto bytes=read_file(license.u16string(),true);crypto::Cipher c;c.set_key_and_iv_042760(tokens::constants::key_084a50,tokens::constants::iv_084a60);check(c.decrypt_with_offset(bytes,0x41),"synthetic license decrypts");check(bytes.find("<UserId>1000200030000</UserId>")!=std::string::npos,"source Game key side effect occurs before later User section, not delayed to provider stage");check(fs::is_regular_file(p.data()/"achievement_watcher/42/achievements.ini"),"source missing INI startup write restored");}
}
int main(){auto root=fs::temp_directory_path()/("preloader_l_original_compat_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));try{config_domain(root);read_domain(root/"read");ini_and_key_order(root/"order");fs::remove_all(root);std::cout<<"PASS original-package compatibility counterexamples="<<count<<"; not original binary execution\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';fs::remove_all(root);return 1;}}
