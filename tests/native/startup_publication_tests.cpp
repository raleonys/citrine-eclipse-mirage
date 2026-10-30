// Compile the real application with an Execution observer. Only flag-publication
// timing is controlled; the Windows mutex, workers, service and factory are real.
#include "../../src/runtime/application_internal.hpp"
#include "preloader_l/codec/crypto.hpp"
#include "preloader_l/core/protocol_identity.hpp"
#include "preloader_l/net/asio_service.hpp"
#include "preloader_l/net/session_handler.hpp"
#include "preloader_l/net/session_routing.hpp"
#include "preloader_l/runtime/startup.hpp"
#include "preloader_l/runtime/libraries.hpp"
#include "preloader_l/platform/hook_unload_guard.hpp"
#include "preloader_l/platform/callback_lock.hpp"
#include "preloader_l/platform/hook_transaction.hpp"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <future>
#include <thread>
#include <iostream>
#include <chrono>

namespace observer {
using namespace std::chrono_literals;
std::promise<void> entered,release;
std::shared_future<void> released=release.get_future().share();
std::atomic<bool> held{false};
preloader_l::execution_recovered::Execution* execution=nullptr;
}
namespace preloader_l::execution_recovered {
class AuditedExecution:public Execution {
    FlagReferences flags_;
public:
    AuditedExecution(MutexState& mutex,FlagReferences flags,std::int64_t& frequency,Platform& platform)
        :Execution(mutex,flags,frequency,platform),flags_(flags){observer::execution=this;}
    void finish_flag_036820(std::uint8_t& flag){
        Execution::finish_flag_036820(flag);
        if(&flag!=&flags_.first_0a3c3d&&&flag!=&flags_.second_0a3c3c&&!observer::held.exchange(true)){
            observer::entered.set_value();observer::released.wait();
        }
    }
};
}
#define Execution AuditedExecution
#include "../../src/runtime/application.cpp"
#undef Execution

int main()try{
    using namespace preloader_l;using namespace std::chrono_literals;
    const auto directory=std::filesystem::temp_directory_path()/("surge-publication-"+std::to_string(GetCurrentProcessId()));
    std::filesystem::remove_all(directory);std::filesystem::create_directories(directory);
    struct Remove {std::filesystem::path path;~Remove(){std::error_code error;std::filesystem::remove_all(path,error);}} remove{directory};
    application::Application app({(directory/"host.exe").u16string(),(directory/"data").u16string(),false});
    auto entered=observer::entered.get_future();
    auto start=std::async(std::launch::async,[&]{app.start();});
    struct Release {bool done=false;void now(){if(!done){done=true;observer::release.set_value();}}~Release(){now();}} release;
    if(entered.wait_for(5s)!=std::future_status::ready)throw std::runtime_error("extra completion observer not reached");
    auto waiter=std::async(std::launch::async,[]{observer::execution->wait_steps_036880(-1);});
    const bool premature=waiter.wait_for(150ms)==std::future_status::ready;
    release.now();start.get();
    if(waiter.wait_for(2s)!=std::future_status::ready)throw std::runtime_error("ready publication did not release waiting callback");
    waiter.get();app.stop();
    if(premature)throw std::runtime_error("all-phase callback escaped before handler factory/profile publication");
    std::cout<<"PASS: paused extra worker cannot publish callback readiness before the real application factory\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
