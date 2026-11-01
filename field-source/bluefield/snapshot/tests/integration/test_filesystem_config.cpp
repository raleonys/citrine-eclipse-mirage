#include "tests/support/io.hpp"
// SPDX-License-Identifier: GPL-3.0-only
#include "src/paths/module_relative.hpp"
#include "src/transport/service.hpp"
#include "src/files/binary_io.hpp"
#include "src/configuration/parser.hpp"
#include "src/profiles/online_session.hpp"
#include "third_party/tinyxml2/tinyxml2.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
namespace fs=std::filesystem;
namespace {
int checks=0;
void require(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
struct Sandbox {
 fs::path cwd=fs::current_path();fs::path root=fs::absolute(fs::temp_directory_path()/("preloader_l_fs_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())));
 Sandbox(){fs::create_directories(root/"caller");fs::current_path(root/"caller");}
 ~Sandbox(){std::error_code ec;fs::current_path(cwd,ec);fs::remove_all(root,ec);}
};
struct Profile {std::string name,country;};
Profile run(const fs::path& root,const fs::path& config) {
 preloader_l::Environment env{preloader_l::RuntimePaths::fixture(root/"application"/"fixture.exe",config,root,root/"programdata")} ;
 preloader_l::Service service(env);service.start();require(service.port()!=0,"transport binds");
 auto c=service.application().connect();const auto reply=c->process("<LSX><Request id=\"path-test\" recipient=\"EbisuSDK\"><GetProfile index=\"0\"/></Request></LSX>");
 tinyxml2::XMLDocument doc;require(doc.Parse(reply.data(),reply.size())==tinyxml2::XML_SUCCESS,"profile XML");auto* n=doc.FirstChildElement("LSX");require(n!=nullptr,"LSX envelope");n=n->FirstChildElement("Response");require(n!=nullptr,"response envelope");n=n->FirstChildElement("GetProfileResponse");require(n!=nullptr,"profile response");
 unsigned fields=0;for(auto* a=n->FirstAttribute();a;a=a->Next())++fields;require(fields==14,"complete profile");require(n->Attribute("Persona") && n->Attribute("Country"),"identity fields");
 Profile result{n->Attribute("Persona"),n->Attribute("Country")};service.stop();return result;
}
}
int main(){try{
 Sandbox s;const auto directory=s.root/fs::path(u8"module_\u00e7\u0131_\u30c6\u30b9\u30c8_\U0001f3c8");fs::create_directories(directory);
 const auto config=preloader_l::paths::module_sibling(directory/"preloader_l.dll","surge.cfg").value;const auto overlay=preloader_l::paths::module_sibling(config,"surge_override.cfg").value;
 require(config.native()==(directory/"surge.cfg").native(),"Unicode config adjacent");require(overlay.native()==(directory/"surge_override.cfg").native(),"Unicode override adjacent");
 for(const char* name:{"surge.cfg","surge_override.cfg"})preloader_l::write_file(preloader_l::model_path(s.root/"caller"/name),"\"Config2\"{\"User\"{\"Username\" \"WrongCwd\"}}");
 auto p=run(s.root,config);require(p.name=="SURGE" && p.country=="US","absent config ignores cwd decoys");
 preloader_l::write_file(preloader_l::model_path(config),"\"Config2\"{\"User\"{\"Username\" \"ModuleBase\" \"Country\" \"TR\"}}");p=run(s.root,config);require(p.name=="ModuleBase" && p.country=="TR","Unicode base config");
 preloader_l::write_file(preloader_l::model_path(overlay),"\"Config2\"{\"User\"{\"Username\" \"ModuleOverride\" \"Country\" \"JP\"}}");p=run(s.root,config);require(p.name=="ModuleOverride" && p.country=="JP","override after base");
 fs::remove(config);p=run(s.root,config);require(p.name=="ModuleOverride","override without optional base");fs::remove(overlay);
 preloader_l::write_file(preloader_l::model_path(config),"\"Config2\"{");bool malformed=false;try{(void)run(s.root,config);}catch(const preloader_l::parser::ParseError&){malformed=true;}require(malformed,"malformed remains error");
 fs::remove(config);fs::create_directory(config);bool invalid=false;try{(void)run(s.root,config);}catch(const std::invalid_argument&){invalid=true;}require(invalid,"directory remains error");fs::remove(config);
 p=run(s.root,config);require(p.name=="SURGE","no-config recovery");
 const auto policy=preloader_l::RuntimePaths::fixture(directory/"host.exe",config,s.root,s.root/"programdata");const auto binary=policy.data()/fs::path(u8"binary_\u00e7_\U0001f3c8.dat");const std::string bytes("a\0b\xff\r\n",6);preloader_l::files::NativeFactory files{policy};
 require(preloader_l::files::write_binary_03c1d0(preloader_l::model_path(binary),bytes,files),"native Unicode writer");require(preloader_l::files::read_binary_03b3f0(preloader_l::model_path(binary),files)==bytes,"native binary reader");
 const auto external=directory/"existing-external-read.bin";
 preloader_l::write_file(preloader_l::model_path(external),bytes);
 require(preloader_l::files::read_binary_03b3f0(preloader_l::model_path(external),files)==bytes,"owned input factory can read an existing external file");
 preloader_l::Environment read_env{policy};preloader_l::configuration::State read_state{};std::uint8_t online=0;
 preloader_l::OnlineSession session(read_state,read_env,online);
 require(session.read_binary_03b3f0(preloader_l::model_path(external))==bytes,"profile input does not apply write ownership to read");
 require(session.status_01ea20(preloader_l::model_path(external),3).kind==2,"profile status can query an external file without write authority");
 bool write_denied=false;try{(void)preloader_l::files::write_binary_03c1d0(preloader_l::model_path(external),"changed",files);}catch(const std::invalid_argument&){write_denied=true;}
 require(write_denied,"external write ownership remains enforced");
 require(preloader_l::read_file(preloader_l::model_path(external),true)==bytes,"external read and rejected write preserve exact original bytes");
 bool missing=false;try{(void)preloader_l::read_file(preloader_l::model_path(directory/"missing.bin"),true);}catch(const std::runtime_error&){missing=true;}require(missing,"required read error retained");
 std::cout<<"PASS filesystem/config integration checks="<<checks<<"; Unicode module/override/cwd isolation/no-config/profile/native binary IO\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
