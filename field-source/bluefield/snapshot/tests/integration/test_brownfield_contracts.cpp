#include "tests/support/io.hpp"
// SPDX-License-Identifier: GPL-3.0-only
// HF19 field-ownership/nesting regression cases adapted to the adopted backend.
// Original fixture identities and changed expectations are recorded separately.
#include "src/application/application.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
namespace {
int checks=0;
void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
template<class Check>void run(const std::filesystem::path& root,const char* name,const char* config,Check check){
 auto directory=root/name;std::filesystem::create_directories(directory);
 preloader_l::write_file(preloader_l::model_path(directory/"surge.cfg"),config);
 preloader_l::Environment env{preloader_l::RuntimePaths::fixture(directory/"fixture.exe",directory/"surge.cfg",directory,directory/"programdata")} ;
 preloader_l::Application app(env);app.initialize();check(app.configuration());
}
}
int main(){try{
 auto root=std::filesystem::temp_directory_path()/("preloader_l_contracts_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 run(root,"nested",R"("Config2"{"User"{"Username" "DIRECT" "Profile"{"Username" "NESTED" "Country" "XX"}}"Achievements"{"AchievementsSet" "set-a" "AchievementNames"{"1" "Winner"}}})",[](const auto& state){
  require(state.username=="DIRECT","HF19 direct User leaf ownership");
  require(state.country=="US","HF19 nested leaf cannot replace direct section policy; source constructor retained");
  require(state.achievements_set=="set-a"&&state.achievements.at("1").display_name=="Winner","collection does not publish unrelated scalar fields");
 });
 run(root,"wrong_section",R"("Config2"{"Game"{"Name" "RIGHT" "Language" "tr_TR"}"User"{"GameName" "WRONG" "Language" "xx_XX" "Username" "RIGHT_USER"}})",[](const auto& state){
  require(state.game_attributes.at("DISPLAY_NAME")=="RIGHT","HF19 wrong-section alias poisoning prevented");
  require(state.game_attributes.at("INSTALLED_LANGUAGE")=="tr_TR","HF19 wrong-section language poisoning prevented");
  require(state.username=="RIGHT_USER","correct section leaf remains active");
 });
 run(root,"root_leaf",R"("Config2"{"Username" "WRONG" "Bogus"{"Username" "WRONG" "Country" "XX"}})",[](const auto& state){
  require(state.username=="SURGE"&&state.country=="US","HF19 root and unknown sections are non-authoritative");
 });
 run(root,"comments",R"("Config2"{/* valid */"User"{"Username" "VALID"}}/* tail */)",[](const auto& state){require(state.username=="VALID","HF19 terminated comments accepted");});
 bool rejected=false;try{run(root,"unterminated",R"("Config2"{"User"{"Username" "BROKEN"}}/* trailing)",[](const auto&){});}catch(const std::exception&){rejected=true;}
 require(rejected,"HF19 unterminated comment rejected");
 std::filesystem::remove_all(root);std::cout<<"PASS HF19-derived field ownership/nesting/comment contracts checks="<<checks<<"; no claim of all-HF19 equivalence\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL HF19-derived contracts: "<<error.what()<<'\n';return 1;}}
