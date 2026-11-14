// SPDX-License-Identifier: GPL-3.0-only

// Compile the real application with an Execution observer. Only flag-publication
// timing is controlled; the Windows mutex, workers, service and factory are real.
#include "../../src/runtime/application_internal.hpp"
#include "laureondo/codec/crypto.hpp"
#include "laureondo/core/protocol_identity.hpp"
#include "laureondo/net/asio_service.hpp"
#include "laureondo/net/session_handler.hpp"
#include "laureondo/net/session_routing.hpp"
#include "laureondo/runtime/startup.hpp"
#include "laureondo/runtime/libraries.hpp"
#include "laureondo/platform/hook_unload_guard.hpp"
#include "laureondo/platform/callback_lock.hpp"
#include "laureondo/platform/hook_transaction.hpp"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <future>
#include <thread>
#include <iostream>
#include <chrono>
#include <asio.hpp>

namespace observer {
using namespace std::chrono_literals;
std::promise<void> entered,release;
std::shared_future<void> released=release.get_future().share();
std::atomic<bool> held{false};
laureondo::execution::Execution* execution=nullptr;
}
namespace laureondo::execution {
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
    using namespace laureondo;using namespace std::chrono_literals;
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
    // B48: the real listener has bound the port, but the Application
    // has not completed provider/profile publication yet.
    char port_text[32]{};
    const auto length=GetEnvironmentVariableA("EALsxPort",port_text,sizeof port_text);
    if(!length||length>=sizeof port_text)throw std::runtime_error("bound listener port not published");
    asio::io_context io;
    asio::ip::tcp::socket early(io);
    early.connect({asio::ip::address_v4::loopback(),static_cast<std::uint16_t>(std::stoul(port_text))});
    early.non_blocking(true);
    bool closed_before_ready=false,delivered_before_ready=false;
    const auto pre_deadline=std::chrono::steady_clock::now()+100ms;
    while(std::chrono::steady_clock::now()<pre_deadline){
        char bytes[2048]{};
        asio::error_code error;
        const auto count=early.read_some(asio::buffer(bytes),error);
        if(count)delivered_before_ready=true;
        if(error&&error!=asio::error::would_block&&error!=asio::error::try_again)closed_before_ready=true;
        if(closed_before_ready||delivered_before_ready)break;
        Sleep(1);
    }
    release.now();start.get();
    if(waiter.wait_for(2s)!=std::future_status::ready)throw std::runtime_error("ready publication did not release waiting callback");
    waiter.get();
    std::string greeting;
    const auto deadline=std::chrono::steady_clock::now()+3s;
    while(greeting.find('\0')==std::string::npos&&std::chrono::steady_clock::now()<deadline){
        char bytes[4096]{};
        asio::error_code error;
        const auto count=early.read_some(asio::buffer(bytes),error);
        if(count)greeting.append(bytes,count);
        if(error&&error!=asio::error::would_block&&error!=asio::error::try_again)
            throw std::runtime_error("early client disconnected after application readiness");
        if(!count)Sleep(1);
    }
    app.stop();
    if(premature)throw std::runtime_error("all-phase callback escaped before handler factory/profile publication");
    if(closed_before_ready||delivered_before_ready)
        throw std::runtime_error("early client was accepted before application readiness");
    if(greeting.find('\0')==std::string::npos)
        throw std::runtime_error("early client never received the greeting after readiness");
    std::cout<<"PASS: extra worker readiness gates callback publication and preserves early TCP clients\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
