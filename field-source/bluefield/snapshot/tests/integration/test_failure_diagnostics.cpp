// SPDX-License-Identifier: GPL-3.0-only
#include "src/application/environment.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <type_traits>
namespace fs=std::filesystem;
using namespace preloader_l;
static unsigned checks=0;
static void check(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
int main(){try{
    static_assert(!std::is_invocable_v<decltype(&Environment::log),Environment*,std::string_view>);
    static_assert(static_cast<int>(LogEvent::unknown_field)==27,"existing event identities must not shift");
    auto root=fs::temp_directory_path()/("preloader_l_failure_events_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    struct Cleanup{fs::path p;~Cleanup(){std::error_code e;fs::remove_all(p,e);}}cleanup{root};
    Environment env(RuntimePaths::from_system(root/"module/preloader_l.dll",root/"game/game.exe",root/"local",root/"program"));
    check(std::string_view(log_event_name(LogEvent::startup_failed))=="startup_failed","startup failure event lost");
    check(std::string_view(log_event_name(LogEvent::shutdown_failed))=="shutdown_failed","shutdown failure event lost");
    check(std::string_view(log_event_name(LogEvent::startup))=="startup","ordinary startup event drift");
    check(!fs::exists(env.data_root),"event names must not create data");
    env.log(LogEvent::startup_failed);env.log(LogEvent::shutdown_failed);
    if(disk_diagnostics_enabled()){
        std::ifstream f(env.paths.diagnostic_log(),std::ios::binary);std::string contents{std::istreambuf_iterator<char>(f),{}};
        check(contents.find(" event=startup_failed\n")!=std::string::npos,"Debug startup failure missing");
        check(contents.find(" event=shutdown_failed\n")!=std::string::npos,"Debug shutdown failure missing");
        check(contents.find(root.string())==std::string::npos,"path leaked to fixed-event log");
        for(std::size_t begin=0;begin<contents.size();){auto end=contents.find('\n',begin);check(end!=std::string::npos,"partial log line");auto line=contents.substr(begin,end-begin);check(line.starts_with("utc_ms="),"unexpected diagnostic payload");check(line.ends_with(" event=startup_failed")||line.ends_with(" event=shutdown_failed"),"unclassified diagnostic text");begin=end+1;}
    }else check(!fs::exists(env.data_root),"Production failure event created disk output");
    std::cout<<"PASS fixed failure diagnostics checks="<<checks<<" disk="<<disk_diagnostics_enabled()<<"; Windows bootstrap execution NOT_RUN\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
