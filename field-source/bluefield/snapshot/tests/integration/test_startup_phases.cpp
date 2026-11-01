// SPDX-License-Identifier: GPL-3.0-only
#include "src/transport/service.hpp"
#include "src/runtime/startup_steps.hpp"
#include "tests/support/io.hpp"
#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using Socket=SOCKET;constexpr Socket bad_socket=INVALID_SOCKET;
void close_socket(Socket s){closesocket(s);}
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
using Socket=int;constexpr Socket bad_socket=-1;
void close_socket(Socket s){::close(s);}
#endif
#if defined(__linux__)
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
namespace {
std::atomic<bool> gate_armed{false};std::mutex gate_mutex;std::condition_variable gate_condition;
bool gate_entered=false,gate_released=false;
}
// Executable-only libc boundary: delay the real provider-file stat, then
// forward it unchanged. No provider search override or production test seam.
extern "C" int stat(const char* path,struct stat* output) noexcept {
 if(gate_armed.load()&&std::string_view(path).ends_with("/preloader_l_online.dll")){
  std::unique_lock lock(gate_mutex);gate_entered=true;gate_condition.notify_all();
  gate_condition.wait(lock,[]{return gate_released;});
 }
 return static_cast<int>(::syscall(SYS_newfstatat,AT_FDCWD,path,output,0));
}
#endif
namespace {
using namespace std::chrono_literals;
int checks=0;
void require(bool b,const char* reason){++checks;if(!b)throw std::runtime_error(reason);}
struct Client {
 Socket socket=bad_socket;std::string pending;
 explicit Client(std::uint16_t port){
  socket=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);if(socket==bad_socket)throw std::runtime_error("test socket");
  sockaddr_in a{};a.sin_family=AF_INET;a.sin_port=htons(port);a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  if(::connect(socket,reinterpret_cast<sockaddr*>(&a),sizeof a)){close_socket(socket);socket=bad_socket;throw std::runtime_error("test connect");}
#ifdef _WIN32
  DWORD timeout=3000;setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof timeout);
#else
  timeval timeout{3,0};setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof timeout);
#endif
 }
 ~Client(){if(socket!=bad_socket)close_socket(socket);}
 std::string frame(){for(;;){auto n=pending.find('\0');if(n!=std::string::npos){auto result=pending.substr(0,n);pending.erase(0,n+1);return result;}char b[1024];const auto got=::recv(socket,b,sizeof b,0);if(got<=0)throw std::runtime_error("test receive");pending.append(b,static_cast<std::size_t>(got));}}
 std::string profile(){std::string text="<LSX><Request id=\"phase\" recipient=\"EbisuSDK\"><GetProfile index=\"0\"/></Request></LSX>";text.push_back('\0');
  while(!text.empty()){
#ifdef _WIN32
   auto n=::send(socket,text.data(),static_cast<int>(text.size()),0);
#else
   auto n=::send(socket,text.data(),text.size(),MSG_NOSIGNAL);
#endif
   if(n<=0)throw std::runtime_error("test send");text.erase(0,static_cast<std::size_t>(n));
  }return frame();
 }
};
struct Waiter {
 preloader_l::Service& service;std::promise<void> entered;std::promise<bool> done;
 std::future<void> entrance=entered.get_future();std::future<bool> completion=done.get_future();std::thread thread;
 explicit Waiter(preloader_l::Service& s):service(s),thread([this]{entered.set_value();done.set_value(service.wait_steps(-1));}){entrance.wait();}
 ~Waiter(){if(thread.joinable()){service.request_stop();thread.join();}}
 bool pending(){return completion.wait_for(50ms)==std::future_status::timeout;}
 bool result(){require(completion.wait_for(2s)==std::future_status::ready,"phase waiter released");auto value=completion.get();thread.join();return value;}
};
struct Gate {
#if defined(__linux__)
 Gate(){std::lock_guard lock(gate_mutex);gate_entered=false;gate_released=false;gate_armed.store(true);}
 void entered(){std::unique_lock lock(gate_mutex);require(gate_condition.wait_for(lock,2s,[]{return gate_entered;}),"real provider-file boundary reached");}
 void release(){gate_armed.store(false);{std::lock_guard lock(gate_mutex);gate_released=true;}gate_condition.notify_all();}
 ~Gate(){release();}
#endif
};
preloader_l::RuntimePaths paths(const std::filesystem::path& root){std::filesystem::create_directories(root);return preloader_l::RuntimePaths::fixture(root/"fixture.exe",root/"surge.cfg",root/"local",root/"programdata");}
void predicate(){for(unsigned bits=0;bits<8;++bits)for(int count:{-1,0,1,2,3}){
 const auto need=count==-1?7u:count==0?0u:(1u<<count)-1;
 require(preloader_l::StartupSteps::finished(count,bits&1,bits&2,bits&4)==((bits&need)==need),"source 40-case step predicate");
}}
void basic(const std::filesystem::path& root){
 preloader_l::Environment env(paths(root));preloader_l::Service service(env);service.prepare();
 require(service.wait_steps(1)&&!service.is_ready(),"source server stage precedes provider and extras");
 {Client c(service.port());require(c.frame().find("Challenge")!=std::string::npos,"IO serves greeting before provider launch");require(c.profile().find("Persona=\"SURGE\"")!=std::string::npos,"configured local profile available before provider");}
 const auto bootstrap=std::this_thread::get_id();std::thread::id worker;bool main_seen=false;
 service.activate_async([&](bool main){if(main){require(std::this_thread::get_id()==bootstrap,"true phase synchronous bootstrap");main_seen=true;}else{require(main_seen,"false phase follows true phase");worker=std::this_thread::get_id();}});
 service.wait_startup();require(worker!=bootstrap&&service.is_ready(),"false phase uses owned worker and all-step readiness");service.stop();require(!service.wait_steps(-1)&&!service.is_ready(),"stop is cancellation, not successful phase completion");
}
#if defined(__linux__)
void gated(const std::filesystem::path& root){
 preloader_l::Environment env(paths(root));preloader_l::Service service(env);service.prepare();Gate gate;
 const auto bootstrap=std::this_thread::get_id();std::promise<void> worker_entered,worker_release;auto entered=worker_entered.get_future();auto release=worker_release.get_future().share();
 // Ensure a failed assertion cannot strand the owned extra worker during unwind.
 struct Release {std::promise<void>& p;bool released=false;void finish(){if(!released){p.set_value();released=true;}}~Release(){finish();}} extra_release{worker_release};
 service.activate_async([&,release](bool main){
  if(main){require(std::this_thread::get_id()==bootstrap,"main selector stays on bootstrap caller");gate.entered();Client client(service.port());(void)client.frame();require(client.profile().find("Persona=\"SURGE\"")!=std::string::npos,"provider wait does not hold request-state lock");}
  else{worker_entered.set_value();release.wait();}
 });
 Waiter all(service);require(all.pending()&&!service.is_ready(),"all-step hook-style wait includes blocked provider");
 require(entered.wait_for(50ms)==std::future_status::timeout,"worker extras do not start before provider completes");
 gate.release();require(entered.wait_for(2s)==std::future_status::ready,"worker extras start after provider completion");
 require(service.wait_steps(2),"first two phases completed");require(all.pending()&&!service.is_ready(),"third extra-worker phase remains required");
 extra_release.finish();service.wait_startup();require(all.result()&&service.is_ready(),"all phases publish honest readiness");service.stop();
}
void cancelled(const std::filesystem::path& root){
 preloader_l::Environment env(paths(root));preloader_l::Service service(env);service.prepare();Gate gate;std::atomic<bool> extra_called=false;
 service.activate_async([&](bool main){if(main)gate.entered();else extra_called=true;});Waiter all(service);
 service.request_stop();require(!all.result()&&!service.is_ready(),"shutdown releases incomplete provider wait as cancellation");
 gate.release();service.stop();require(!extra_called.load(),"cancelled dependent extra phase does not run");
}
void provider_failure(const std::filesystem::path& root){
 preloader_l::Environment env(paths(root));preloader_l::write_file(preloader_l::model_path(env.paths.provider()),"synthetic regular file");preloader_l::Service service(env);service.prepare();Gate gate;
 service.activate_async([&](bool main){if(main)gate.entered();});Waiter all(service);gate.release();
 require(!all.result()&&!service.is_ready(),"provider failure cancels completion wait");bool failed=false;
 try{service.wait_startup();}catch(const std::exception& e){failed=std::string(e.what())=="optional native provider requires Windows";}
 require(failed,"provider exception transported to startup owner");service.stop();
}
#endif
void extra_failure(const std::filesystem::path& root){
 preloader_l::Environment env(paths(root));preloader_l::Service service(env);service.prepare();
 service.activate_async([](bool main){if(!main)throw std::runtime_error("fixture extra failure");});bool failed=false;
 try{service.wait_startup();}catch(const std::exception& e){failed=std::string(e.what())=="fixture extra failure";}
 require(failed&&!service.is_ready()&&!service.wait_steps(-1),"extra-worker error is not readiness success");service.stop();
}
}
int main(){try{
 const auto root=std::filesystem::temp_directory_path()/("preloader_l_phase_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 predicate();basic(root/"basic");extra_failure(root/"extra-error");
#if defined(__linux__)
 gated(root/"gated");cancelled(root/"cancelled");provider_failure(root/"provider-error");
#endif
 std::filesystem::remove_all(root);std::cout<<"PASS startup phases checks="<<checks<<"; source predicate, actual IO/provider ordering, extra-phase completion, cancellation/failure. Windows provider/hook boundary NOT_RUN unless native fixture executed\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL startup phases: "<<e.what()<<'\n';return 1;}}
