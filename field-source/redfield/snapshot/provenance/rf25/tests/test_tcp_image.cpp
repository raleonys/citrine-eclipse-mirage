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
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf09_image_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const char* configuration){if(configuration){std::ofstream f(dir/"redfield.cfg",std::ios::binary);f<<configuration;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const char* id,const std::string& command){return std::string("<LSX><Request id='")+id+"'>"+command+"</Request></LSX>";}
std::string attr(const tinyxml2::XMLElement* e,const char* key){const auto* v=e->Attribute(key);return v?v:"!MISSING";}
int main(){
    struct Cwd {std::filesystem::path old=std::filesystem::current_path();~Cwd(){std::filesystem::current_path(old);}};
    const char* custom=R"("Config2" {"User" {"Avatar" "relative & <Ω>.png"}})";
    const char* empty=R"("Config2" {"User" {"Avatar" ""}})";
    for(const char* config:{static_cast<const char*>(nullptr),custom,empty}){
        Server s;Cwd cwd;std::filesystem::current_path(s.dir);CHECK(s.start(config));const auto path=(s.dir/"surge.png").u8string();const std::string fallback(reinterpret_cast<const char*>(path.data()),path.size());
        const std::string resource=!config?fallback:config==custom?"relative & <Ω>.png":"";
        const auto data=request("skip","<QueryImage Width='1' Height='2' ImageId='x'/>")+request("chunk","<QueryChunkStatus/>")+request("image&amp;lt;","<QueryImage><Width> -417 </Width><Height>nonnumeric</Height><ImageId>A&amp;lt;:x&amp;y</ImageId></QueryImage>")+request("cdata","<QueryImage><Width><![CDATA[ &lt; ]]></Width><Height/><ImageId><![CDATA[A&amp;lt;:]]></ImageId></QueryImage>")+request("profile","<GetProfile index='0'/>");
        asio::write(s.client,asio::buffer(data.data(),13));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(data.data()+13,data.size()-13));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().frames==5 && s.service.snapshot().responses==3 && s.service.snapshot().malformed==0);
        const char* ids[]={"image&lt;","cdata","profile"};std::size_t offset=0;
        for(unsigned i=0;i<3;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument d;CHECK(d.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* response=d.RootElement()->FirstChildElement("Response");CHECK(response && attr(response,"id")==ids[i]);const auto* body=response->FirstChildElement();CHECK(body);
            if(i<2){CHECK(std::string(body->Name())=="QueryImageResponse" && attr(body,"Result")=="0");const auto* image=body->FirstChildElement("Image");CHECK(image && attr(image,"ResourcePath")==resource);CHECK(attr(image,"Width")== (i==0?"-417":"&lt;"));CHECK(attr(image,"Height")== (i==0?"nonnumeric":""));CHECK(attr(image,"ImageId")== (i==0?"A&lt;:x&y":"A&amp;lt;:"));}
            else CHECK(std::string(body->Name())=="GetProfileResponse");
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);CHECK(s.until([&]{return s.service.stop_for(1ms);}));
    }
    {Server s;const std::string configuration="\"Config2\" {\"User\" {\"Avatar\" \""+std::string(17000,'x')+"\"}}";CHECK(s.start(configuration.c_str()));const auto data=request("bounded","<QueryImage><Width>1</Width><Height>2</Height><ImageId>x</ImageId></QueryImage>");asio::write(s.client,asio::buffer(data));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().responses==0 && s.service.snapshot().malformed==1);}
    for(const std::string& bad:{request("bad","<QueryImage><Width><!--bad-->1</Width><Height>2</Height><ImageId>x</ImageId></QueryImage>"),std::string("<LSX><Request><QueryImage><Width>1</Width><Height>2</Height><ImageId>x</ImageId></QueryImage></Request></LSX>")}){Server s;CHECK(s.start(custom));asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1);}
    std::cout<<"TCP_IMAGE_DLL_FALLBACK_OVERRIDE_FIFO_ENTITY_CAPACITY_ATOMIC_PASS\n";
}
