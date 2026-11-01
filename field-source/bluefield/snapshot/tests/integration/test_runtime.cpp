#include "tests/support/io.hpp"
// SPDX-License-Identifier: GPL-3.0-only
#include "src/transport/service.hpp"
#include "src/crypto/crypto.hpp"
#include "src/protocol/messages.hpp"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using TestSocket=SOCKET;constexpr TestSocket invalid_socket=INVALID_SOCKET;
static void close_test_socket(TestSocket s){closesocket(s);}
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using TestSocket=int;constexpr TestSocket invalid_socket=-1;
static void close_test_socket(TestSocket s){::close(s);}
#endif
namespace {
int checks=0;
void require(bool b,const char* label){++checks;if(!b)throw std::runtime_error(label);}
std::string request(std::string_view command,std::string_view id="test"){return "<LSX><Request id=\""+std::string(id)+"\" recipient=\"EbisuSDK\">"+std::string(command)+"</Request></LSX>";}
struct Client {
 TestSocket socket=invalid_socket;std::string pending;
 explicit Client(std::uint16_t port){socket=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);if(socket==invalid_socket)throw std::runtime_error("test socket");sockaddr_in a{};a.sin_family=AF_INET;a.sin_port=htons(port);a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);if(::connect(socket,reinterpret_cast<sockaddr*>(&a),sizeof a))throw std::runtime_error("test connect");
#ifdef _WIN32
 DWORD timeout=3000;setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof timeout);
#else
 timeval timeout{3,0};setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof timeout);
#endif
 }
 ~Client(){if(socket!=invalid_socket)close_test_socket(socket);}
 void send_bytes(std::string_view bytes){while(!bytes.empty()){int n=static_cast<int>(::send(socket,bytes.data(),static_cast<int>(bytes.size()),0));if(n<=0)throw std::runtime_error("test send");bytes.remove_prefix(static_cast<size_t>(n));}}
 void send(std::string bytes){bytes.push_back('\0');send_bytes(bytes);}
 std::string frame(){for(;;){auto n=pending.find('\0');if(n!=std::string::npos){auto result=pending.substr(0,n);pending.erase(0,n+1);return result;}char data[4096];int count=static_cast<int>(::recv(socket,data,sizeof data,0));if(count<=0)throw std::runtime_error("test receive");pending.append(data,static_cast<size_t>(count));if(pending.size()>2*1024*1024)throw std::length_error("test frame bound");}}
 std::string exchange(std::string_view command){send(request(command));return frame();}
};
const char* fixture=R"CFG("Config2" {
 "Game" { "Name" "BLUEFIELD synthetic fixture" "ContentId" "424242" "Version" "1.0" "Language" "en_US" }
 "Emulator" { "ServerPort" "random" "PretendConnected" "true" "FakeAuth" "false" "SDK" "9.9.0.0" }
 "User" { "Username" "FixturePlayer" "PersonaId" "10001" "UserId" "20001" "Country" "US" "Currency" "USD" "IsSubscriber" "false" "IsSteamSubscriber" "false" "IsUnderAge" "false" }
 "Content" { "fixture_game" {"Name" "GAME" "Version" "1.0" "State" "INSTALLED"} }
 "Entitlements" { "fixture_offer" {"Group" "fixture_group"} }
 "Achievements" {"AchievementsSet" "fixture_set" "AchievementIDs" "a1" "SteamAppId" "424242" "SteamIdPrefix" "fixture_" }
})CFG";
}
int main(){try{
 auto root=std::filesystem::temp_directory_path()/("preloader_l_runtime_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 std::filesystem::create_directories(root);preloader_l::write_file(preloader_l::model_path(root/"surge.cfg"),fixture);
 preloader_l::Environment env{preloader_l::RuntimePaths::fixture(root/"fixture.exe",root/"surge.cfg",root,root/"programdata")} ;
 {
  preloader_l::Service service(env);service.start();require(service.port()!=0,"bound port published");
  Client client(service.port());require(client.frame().find("Challenge")!=std::string::npos,"real TCP greeting");
  auto message=request("<GetConfig/>");message.push_back('\0');
  client.send_bytes(message.substr(0,7));std::this_thread::sleep_for(std::chrono::milliseconds(2));client.send_bytes(message.substr(7));
  require(client.frame().find("GetConfigResponse")!=std::string::npos,"fragmented GetConfig");
  require(client.exchange("<GetProfile index=\"0\"/>").find("FixturePlayer")!=std::string::npos,"live profile binding");
  std::string a=request("<QueryContent/>","a"),b=request("<QueryEntitlements/>","b");a.push_back('\0');b.push_back('\0');client.send_bytes(a+b);
  require(client.frame().find("fixture_game")!=std::string::npos,"coalesced content request");
  require(client.frame().find("fixture_offer")!=std::string::npos,"coalesced entitlement request");
  require(client.exchange("<GetInternetConnectedState/>").find("connected=\"1\"")!=std::string::npos,"connected flag source behavior");
  require(client.exchange("<GetAuthCode ClientId=\"fixture\"/>").find("-1577058293")!=std::string::npos,"offline authentication denied");
  require(client.exchange("<QueryImage Width=\"208\" Height=\"208\" ImageId=\"avatar:20001\"/>").find("ResourcePath")!=std::string::npos,"avatar path response");
  require(client.exchange("<GrantAchievement AchievementId=\"a1\" Progress=\"1\"/>").find("Achievement")!=std::string::npos,"achievement mutation");
  require(std::filesystem::is_regular_file(root/"SURGE/achievement-fixture_set.xml"),"real XML persistence");
  require(std::filesystem::is_regular_file(root/"SURGE/achievement_watcher/424242/achievements.ini"),"real INI persistence");
  require(client.exchange("<ChallengeResponse key=\"synthetic\" version=\"2\"/>").find("ChallengeAccepted")!=std::string::npos,"native crypto handshake");
  preloader_l::crypto::Cipher cipher;cipher.set_fixed_key_084c98();auto encrypted=request("<GetConfig/>","encrypted");cipher.encrypt_042610(encrypted);preloader_l::messages::hex_encode_03d190(encrypted);client.send(encrypted);
  auto response=client.frame();preloader_l::messages::hex_decode_03ce30(response);require(cipher.decrypt_042480(response)&&response.find("GetConfigResponse")!=std::string::npos,"encrypted live protocol roundtrip");
  std::atomic<bool> okay=true;std::vector<std::thread> clients;
  for(int i=0;i<8;++i)clients.emplace_back([&]{try{Client c(service.port());(void)c.frame();for(int j=0;j<20;++j)if(c.exchange("<GetProfile index=\"0\"/>").find("FixturePlayer")==std::string::npos)okay=false;}catch(...){okay=false;}});
  for(auto& t:clients)t.join();require(okay.load(),"8 concurrent real clients, 160 requests");
  service.stop();
 }
 {
  preloader_l::Service restarted(env);restarted.start();Client client(restarted.port());(void)client.frame();
  require(client.exchange("<QueryAchievements All=\"true\"/>").find("Count=\"1\"")!=std::string::npos,"achievement reload after restart");
  restarted.stop();
 }
 {
  preloader_l::write_file(preloader_l::model_path(root/"bad.cfg"),"\"Config2\"{ broken }");
  preloader_l::Environment bad{preloader_l::RuntimePaths::fixture(env.executable,root/"bad.cfg",root,env.program_data)} ;
  bool rejected=false;try{preloader_l::Service invalid(bad);invalid.start();}catch(...){rejected=true;}require(rejected,"bad config startup rejected");
 }
 std::filesystem::remove_all(root);
 std::cout<<"PASS full runtime integration checks="<<checks<<"; concurrent clients=8; profile requests=160\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL runtime: "<<e.what()<<'\n';return 1;}}
