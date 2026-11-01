// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/local_service.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <fstream>
#include <iostream>
using namespace redfield;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Server {
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf08_presence_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const char* configuration){if(configuration){std::ofstream f(dir/"redfield.cfg",std::ios::binary);f<<configuration;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const char* id,const std::string& command){return std::string("<LSX><Request id='")+id+"'>"+command+"</Request></LSX>";}
std::string attr(const tinyxml2::XMLElement* e,const char* key){const auto* v=e->Attribute(key);return v?v:"!MISSING";}
int main(){
    const char* custom=R"("Config2" {"User" {"Username" "Sky & <King>"}})";
    for(const char* config:{static_cast<const char*>(nullptr),R"("Config2" {})",custom}){
        Server s;CHECK(s.start(config));const std::string persona=!config?"SURGE":config==custom?"Sky & <King>":"";
        // Over-count is an explicit unsupported/no-response result; the next
        // valid response stays FIFO and cannot reuse a previous response buffer.
        std::string over="<QueryPresence>";for(unsigned i=0;i<513;++i)over+="<u>7</u>";over+="</QueryPresence>";
        const auto data=request("empty","<QueryPresence/>")+request("chunk","<QueryChunkStatus/>")+request("over",over)+request("rows&amp;lt;","<QueryPresence><u><!--lead-->one<tail/>ignored</u><u><![CDATA[two&<]]></u></QueryPresence>")+request("profile","<GetProfile index='0'/>");
        asio::write(s.client,asio::buffer(data.data(),13));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(data.data()+13,data.size()-13));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().frames==5 && s.service.snapshot().responses==3 && s.service.snapshot().malformed==0);CHECK(s.received.find('\0')==std::string::npos);
        const char* ids[]={"empty","rows&lt;","profile"};std::size_t offset=0;
        for(unsigned i=0;i<3;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument d;CHECK(d.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* response=d.RootElement()->FirstChildElement("Response");CHECK(response && attr(response,"id")==ids[i]);const auto* body=response->FirstChildElement();CHECK(body);
            if(i==0)CHECK(std::string(body->Name())=="QueryPresenceResponse" && !body->FirstChildElement());
            else if(i==1){CHECK(std::string(body->Name())=="QueryPresenceResponse");const auto* first=body->FirstChildElement();CHECK(first && attr(first,"UserId")=="one" && attr(first,"Persona")==persona && attr(first,"Presence")=="UNKNOWN" && attr(first,"State")=="NONE");const auto* next=first->NextSiblingElement();CHECK(next && attr(next,"UserId")=="two&<" && attr(next,"Persona")==persona && !next->NextSiblingElement());}
            else CHECK(std::string(body->Name())=="GetProfileResponse" && attr(body,"Persona")==persona);
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);CHECK(s.until([&]{return s.service.stop_for(1ms);}));
    }
    // Count-valid but wire-too-large must reject before sending any prefix.
    {Server s;CHECK(s.start(custom));std::string command="<QueryPresence>";for(unsigned i=0;i<512;++i)command+="<u>7</u>";command+="</QueryPresence>";const auto data=request("bounded",command);asio::write(s.client,asio::buffer(data));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().responses==0 && s.service.snapshot().malformed==1);}
    {Server s;CHECK(s.start(custom));const auto bad=request("first_element","<QueryPresence><u><first/>text</u></QueryPresence>");asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1);}
    {Server s;CHECK(s.start(custom));const std::string bad="<LSX><Request><QueryPresence><u>7</u></QueryPresence></Request></LSX>";asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1);}
    std::cout<<"TCP_QUERY_PRESENCE_FIFO_COUNT_BOUND_CAPACITY_ATOMIC_PASS\n";
}
