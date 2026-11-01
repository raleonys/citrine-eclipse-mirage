#include "tests/support/io.hpp"
// SPDX-License-Identifier: GPL-3.0-only
#include "src/transport/service.hpp"
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <new>
#include <stdexcept>
#include <thread>
#include <vector>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using Socket=SOCKET;constexpr Socket invalid_socket=INVALID_SOCKET;
void close_socket(Socket s){closesocket(s);}
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#if defined(__linux__)
#include <sys/syscall.h>
#endif
using Socket=int;constexpr Socket invalid_socket=-1;
void close_socket(Socket s){::close(s);}
#endif
namespace {
std::atomic<bool> fail_next_background{false},session_allocations_only{true};std::atomic<unsigned> injected{0};std::atomic<std::size_t> fault_size{0};std::thread::id main_thread;
#if defined(__linux__)
std::atomic<unsigned> accept_faults_remaining{0},accept_errors{0};
#endif
int checks=0;const char* phase="initialize";
struct FrameClosed:std::runtime_error{FrameClosed():std::runtime_error("fixture EOF") {}};
void require(bool value,const char* reason){++checks;if(!value)throw std::runtime_error(reason);}
std::string request(int id,const char* command="<GetProfile index=\"0\"/>"){
 return "<LSX><Request id=\""+std::to_string(id)+"\" recipient=\"EbisuSDK\">"+command+"</Request></LSX>";
}
struct Client {
 Socket socket=invalid_socket;std::string pending;
 explicit Client(unsigned short port){
  socket=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);if(socket==invalid_socket)throw std::runtime_error("fixture socket");
#ifdef _WIN32
  DWORD timeout=3000;setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof timeout);
#else
  timeval timeout{3,0};setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof timeout);
#endif
  sockaddr_in a{};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);a.sin_port=htons(port);
  if(::connect(socket,reinterpret_cast<sockaddr*>(&a),sizeof a)){close_socket(socket);socket=invalid_socket;throw std::runtime_error("fixture connect");}
 }
 ~Client(){if(socket!=invalid_socket)close_socket(socket);}
 Client(const Client&)=delete;
 void send(std::string_view text){while(!text.empty()){
#ifdef _WIN32
  const auto n=::send(socket,text.data(),static_cast<int>(text.size()),0);
#else
  const auto n=::send(socket,text.data(),text.size(),MSG_NOSIGNAL);
#endif
  if(n<=0)throw std::runtime_error("fixture send");text.remove_prefix(static_cast<std::size_t>(n));
 }}
 std::string frame(){for(;;){auto n=pending.find('\0');if(n!=std::string::npos){auto s=pending.substr(0,n);pending.erase(0,n+1);return s;}char bytes[137];const auto got=::recv(socket,bytes,sizeof bytes,0);if(got==0)throw FrameClosed();if(got<0){
#ifdef _WIN32
 const auto code=WSAGetLastError();
#else
 const auto code=errno;
#endif
 throw std::runtime_error("fixture recv error="+std::to_string(code));}pending.append(bytes,static_cast<std::size_t>(got));if(pending.size()>2*1024*1024)throw std::runtime_error("fixture response bound");}}
 void message(std::string text){text.push_back('\0');send(text);}
};
}
#if defined(__linux__)
// Test-executable-only native boundary fault. The production source/vendor tree
// has no selector or hook for this: actual Asio completions and TCP frames run.
extern "C" int accept(int socket,sockaddr* address,socklen_t* length){
 auto remaining=accept_faults_remaining.load();
 while(remaining&&!accept_faults_remaining.compare_exchange_weak(remaining,remaining-1)){}
 if(remaining){accept_errors.fetch_add(1);errno=EMFILE;return -1;}
#if defined(SYS_accept)
 return static_cast<int>(::syscall(SYS_accept,socket,address,length));
#else
 return static_cast<int>(::syscall(SYS_accept4,socket,address,length,0));
#endif
}
#endif
void* operator new(std::size_t n){
 // Session contains a fixed 64 KiB buffer: fault that owned allocation in
 // the recovery test, not arbitrary internals of the Asio dispatcher.
 const bool target=!session_allocations_only.load()||(n>=65536 && n<69632);
 if(target&&fail_next_background.load(std::memory_order_acquire)&&std::this_thread::get_id()!=main_thread&&fail_next_background.exchange(false)){
  fault_size.store(n);injected.fetch_add(1);throw std::bad_alloc();
 }
 if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();
}
void operator delete(void* p) noexcept{std::free(p);}
void operator delete(void* p,std::size_t) noexcept{std::free(p);}
int main(){main_thread=std::this_thread::get_id();try{
 const auto root=std::filesystem::temp_directory_path()/("preloader_l_asio_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));std::filesystem::create_directories(root);
 preloader_l::Environment env{preloader_l::RuntimePaths::fixture(root/"fixture.exe",root/"surge.cfg",root,root/"programdata")} ;
 const std::string large(300000,'G');preloader_l::write_file(preloader_l::model_path(env.configuration_file),"\"Config2\"{\"Game\"{\"Name\" \""+large+"\"}}");
 {
  preloader_l::Service service(env);service.start();Client c(service.port());(void)c.frame();
  phase="fragments/FIFO";const auto split=request(1);c.send(std::string_view(split).substr(0,17));c.send(std::string_view(split).substr(17));c.send(std::string_view("\0",1));
  require(c.frame().find("Persona=\"SURGE\"")!=std::string::npos,"fragmented request and default profile");
  std::string batch;batch.push_back('\0');for(int i=2;i<32;++i){batch+=request(i);batch.push_back('\0');}c.send(batch);
  for(int i=2;i<32;++i)require(c.frame().find("id=\""+std::to_string(i)+"\"")!=std::string::npos,"coalesced frames preserve response FIFO");
  phase="source frame domain";const std::string padding(1024*1024+1,'P');c.message("<LSX><Request id=\"wide\" recipient=\"EbisuSDK\"><GetProfile index=\"0\" Padding=\""+padding+"\"/></Request></LSX>");require(c.frame().find("GetProfileResponse")!=std::string::npos,"source transport accepts frame beyond invented 1MiB cap");
  phase="large write";c.message(request(32,"<GetGameInfo GameInfoId=\"DISPLAY_NAME\"/>"));const auto response=c.frame();
  require(response.find(large)!=std::string::npos,"300k response retained across async writes and tiny receive chunks");
  phase="malformed no-response";{Client broken(service.port());(void)broken.frame();broken.message("<malformed");bool closed=false;try{(void)broken.frame();}catch(...){closed=true;}require(closed,"malformed request gives no response; listener stays available");}
  {Client next(service.port());(void)next.frame();next.message(request(33));require(next.frame().find("GetProfileResponse")!=std::string::npos,"new client after failed request");}
  // Target accepted Session allocation; arbitrary dispatch faults are separate.
  phase="owned Session allocation";
  fail_next_background.store(true,std::memory_order_release);
  try{Client fault(service.port());(void)fault.frame();}catch(...){}
  fail_next_background.store(false);require(injected.load()==1,"one background allocation failure injected");
  require(fault_size.load()>=65536 && fault_size.load()<69632 && service.is_ready(),"owned Session failure preserves actual readiness");
  {Client recovered(service.port());(void)recovered.frame();recovered.message(request(34));require(recovered.frame().find("GetProfileResponse")!=std::string::npos,"accept recovers after injected allocation failure");}
  c.send("<LSX><Request");const auto begin=std::chrono::steady_clock::now();service.stop();require(std::chrono::steady_clock::now()-begin<std::chrono::seconds(2),"stop cancels incomplete async read and drains owners");service.stop();
 }
 phase="repeated lifecycle";for(int generation=0;generation<10;++generation){preloader_l::Service service(env);service.start();Client c(service.port());(void)c.frame();c.message(request(35));require(c.frame().find("SURGE")!=std::string::npos,"repeated create/start/stop with live socket");service.stop();}
 {
  phase="source client domain";preloader_l::Service service(env);service.start();std::vector<std::unique_ptr<Client>> admitted;
  for(int i=0;i<40;++i){auto c=std::make_unique<Client>(service.port());(void)c->frame();admitted.push_back(std::move(c));}
  {Client excess(service.port());require(excess.frame().find("Challenge")!=std::string::npos,"source accepts beyond invented client cap");}
  const auto begin=std::chrono::steady_clock::now();service.stop();require(std::chrono::steady_clock::now()-begin<std::chrono::seconds(2),"stop drains live async sessions");
 }
#if defined(__linux__)
 {
  phase="source accept completion errors";preloader_l::Service service(env);service.start();const auto port=service.port();
  accept_faults_remaining.store(3);bool response=false;
  try{Client client(port);(void)client.frame();client.message(request(101));response=client.frame().find("GetProfileResponse")!=std::string::npos;}catch(...){}
  accept_faults_remaining.store(0);
  require(accept_errors.load()!=0,"native accept error was actually injected");
  require(response&&service.is_ready()&&service.port()==port,"accept completion errors keep the same listener usable");
  require(accept_errors.load()==3,"three consecutive native accept errors were rearmed");
  Client next(port);(void)next.frame();next.message(request(102));require(next.frame().find("SURGE")!=std::string::npos,"later client after native accept failures");
  service.stop();require(!service.is_ready(),"explicit stop still clears readiness after accept recovery");
  std::cout<<"Native accept fault regression injected="<<accept_errors.load()<<" platform=Linux; Win32 boundary NOT_RUN\n";
 }
#endif
 phase="arbitrary dispatch allocation";unsigned attempts=0,terminal=0,recovered=0,dispatch=0,rearm=0;
 session_allocations_only.store(false);
 // Bounded campaign; accepting a terminal failure does not mean recovery.
 // A false-ready listener that cannot serve the next client must fail.
 for(unsigned n=0;n<6 && !terminal;++n){
  preloader_l::Service service(env);service.start();
  const auto log_start=preloader_l::read_file(preloader_l::model_path(env.paths.diagnostic_log()),preloader_l::disk_diagnostics_enabled()).size();
  const auto before=injected.load();fail_next_background.store(true,std::memory_order_release);
  try{Client fault(service.port());(void)fault.frame();}catch(...){}
  fail_next_background.store(false);++attempts;require(injected.load()==before+1,"arbitrary background fault injected");
  bool response=false;try{Client next(service.port());(void)next.frame();next.message(request(99));response=next.frame().find("GetProfileResponse")!=std::string::npos;}catch(...){}
  const bool stopped=!service.is_ready();require(response||stopped,"arbitrary failure recovers or explicitly clears readiness");
  if(stopped)++terminal;else ++recovered;
  const auto log=preloader_l::read_file(preloader_l::model_path(env.paths.diagnostic_log()),preloader_l::disk_diagnostics_enabled()).substr(log_start);
  if(log.find("event=dispatch_failed")!=std::string::npos)++dispatch;
  if(log.find("event=accept_rearm_failed")!=std::string::npos)++rearm;
  service.stop();require(!service.is_ready(),"stop clears readiness");
 }
 std::cout<<"Asio fault campaign attempts="<<attempts<<" terminal="<<terminal<<" dispatch="<<dispatch<<" rearm="<<rearm<<" recovery="<<recovered<<'\n';
 std::filesystem::remove_all(root);std::cout<<"PASS Asio transport checks="<<checks<<"; fragmentation/FIFO/300k-write/fault recovery/cancel/repeated lifecycle; native IOCP requires Windows\n";return 0;
}catch(const std::exception& e){fail_next_background.store(false);std::cerr<<"FAIL Asio phase="<<phase<<" checks="<<checks<<" fault_size="<<fault_size.load()<<": "<<e.what()<<'\n';return 1;}}
