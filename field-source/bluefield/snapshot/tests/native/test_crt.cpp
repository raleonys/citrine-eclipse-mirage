#include "tests/support/io.hpp"
// SPDX-License-Identifier: GPL-3.0-only
#include "src/transport/service.hpp"
#include <crtdbg.h>
#include <filesystem>
#include <chrono>
#include <iostream>
int main(){
 auto root=std::filesystem::temp_directory_path()/("preloader_l_crt_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 std::filesystem::create_directories(root);
 preloader_l::write_file(preloader_l::model_path(root/"surge.cfg"),"\"Config2\"{\"Emulator\"{\"ServerPort\" \"random\" \"SDK\" \"9.9.0.0\"}\"User\"{\"Username\" \"Fixture\" \"PersonaId\" \"1\" \"UserId\" \"1\"}}");
 preloader_l::Environment env{preloader_l::RuntimePaths::fixture(root/"fixture.exe",root/"surge.cfg",root,root/"programdata")} ;
 try{
  {preloader_l::Service warmup(env);warmup.start();warmup.stop();}
  _CrtMemState before{},after{},difference{};_CrtMemCheckpoint(&before);
  for(int n=0;n<20;++n){preloader_l::Service service(env);service.start();auto session=service.application().connect();(void)session->greeting();session.reset();service.stop();}
  _CrtMemCheckpoint(&after);const int leaked=_CrtMemDifference(&difference,&before,&after);
  std::filesystem::remove_all(root);
  if(leaked){_CrtMemDumpStatistics(&difference);std::cerr<<"FAIL CRT object lifetime imbalance\n";return 1;}
  std::cout<<"PASS CRT core object lifetime: 20 start/connect/stop cycles\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
