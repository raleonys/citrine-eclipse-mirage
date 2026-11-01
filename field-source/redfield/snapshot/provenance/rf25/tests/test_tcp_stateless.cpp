// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/local_service.hpp"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <fstream>
#include <iostream>
using namespace redfield;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Server {
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf05_stateless_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(bool online){std::ofstream f(dir/"redfield.cfg",std::ios::binary);f<<"\"Config2\" {\"Emulator\" {\"Online\" \""<<(online?"true":"false")<<"\"}}";f.close();if(!f || !service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const char* id,const char* command){return std::string("<LSX><Request id='")+id+"'><"+command+"/></Request></LSX>";}
std::string response(const char* id,const char* body){return std::string(" <LSX>\n  <Response id=\"")+id+"\" sender=\"EbisuSDK\">\n    "+body+"\n  </Response>\n </LSX>\n";}
int main(){
    std::string offline_wire;
    for(bool online:{false,true}){Server s;CHECK(s.start(online));
        const auto initial=request("chunk0","QueryChunkStatus");asio::write(s.client,asio::buffer(initial));
        CHECK(s.until([&]{return s.service.snapshot().frames==1;}));CHECK(s.received.empty() && s.service.snapshot().active==1 && s.service.snapshot().responses==0);
        const auto input=request("friends","QueryFriends")+request("chunk1","QueryChunkStatus")+request("group","GetGroupInfo")+request("voip","GetVoipStatus")+request("block","GetBlockList")+request("visible","GetPresenceVisibility");
        asio::write(s.client,asio::buffer(input.data(),11));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(input.data()+11,input.size()-11));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));
        const auto want=response("friends","<QueryFriendsResponse Return=\"Success\"/>")+response("group","<ErrorSuccess Description=\"User not in a party, create one.\" Code=\"-1577058296\"/>")+response("voip","<ErrorSuccess Description=\"The function you are calling is not implemented.\" Code=\"-1593835518\"/>")+response("block","<GetBlockListResponse Return=\"Success\"/>")+response("visible","<GetPresenceVisibilityResponse Visible=\"false\"/>");
        CHECK(s.received==want && s.received.find('\0')==std::string::npos);CHECK(s.service.snapshot().frames==7 && s.service.snapshot().responses==5 && s.service.snapshot().malformed==0);
        if(!online)offline_wire=s.received;else CHECK(s.received==offline_wire);
        CHECK(s.until([&]{return s.service.stop_for(1ms);}));
    }
    {Server s;CHECK(s.start(false));const std::string bad="<LSX><Request><QueryChunkStatus/></Request></LSX>";asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1);}
    std::cout<<"TCP_STATELESS_NO_RESPONSE_THEN_FIFO_ERRORS_PASS\n";
}
