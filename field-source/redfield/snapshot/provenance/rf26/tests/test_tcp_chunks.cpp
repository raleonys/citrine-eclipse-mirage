// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/local_service.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <fstream>
#include <iostream>
using namespace preloader_l;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Server {
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf10_chunks_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const char* configuration){if(configuration){std::ofstream f(dir/"preloader_l.cfg",std::ios::binary);f<<configuration;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const char* id,const std::string& command){return std::string("<LSX><Request id='")+id+"'>"+command+"</Request></LSX>";}
std::string attr(const tinyxml2::XMLElement* e,const char* key){const auto* v=e->Attribute(key);return v?v:"!MISSING";}
int main(){
    for(const char* config:{static_cast<const char*>(nullptr),R"("Config2" {"Emulator" {"Online" "true"}})"}){
        Server s;CHECK(s.start(config));
        const auto data=request("empty","<AreChunksInstalled/>")+request("skip","<AreChunksInstalled><ChunkIds>1</ChunkIds><ChunkIds>2147483648</ChunkIds></AreChunksInstalled>")+request("chunk","<QueryChunkStatus/>")+request("rows&amp;lt;","<AreChunksInstalled ItemId='ignored'><Other><ChunkIds>bad</ChunkIds></Other><ChunkIds>+0007</ChunkIds><ChunkIds>-2147483648</ChunkIds><ChunkIds>&#43;0009</ChunkIds><ChunkIds><![CDATA[-0]]></ChunkIds><ChunkIds>+0007</ChunkIds></AreChunksInstalled>")+request("profile","<GetProfile index='0'/>");
        asio::write(s.client,asio::buffer(data.data(),13));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(data.data()+13,data.size()-13));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().frames==5 && s.service.snapshot().responses==3 && s.service.snapshot().malformed==0);
        const char* ids[]={"empty","rows&lt;","profile"};std::size_t offset=0;
        for(unsigned i=0;i<3;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument d;CHECK(d.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* response=d.RootElement()->FirstChildElement("Response");CHECK(response && attr(response,"id")==ids[i]);const auto* body=response->FirstChildElement();CHECK(body);
            if(i<2){CHECK(std::string(body->Name())=="AreChunksInstalledResponse" && attr(body,"Installed")=="true" && attr(body,"ItemId")=="");const auto* child=body->FirstChildElement();if(i==0)CHECK(!child);else {for(const char* id:{"7","-2147483648","9","0","7"}){CHECK(child && std::string(child->Name())=="ChunkIds" && child->GetText() && std::string(child->GetText())==id);child=child->NextSiblingElement();}CHECK(!child);}}
            else CHECK(std::string(body->Name())=="GetProfileResponse");
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);CHECK(s.until([&]{return s.service.stop_for(1ms);}));
    }
    // A valid <8KiB frame can exceed the 16KiB response capacity because the
    // current TinyXML2 unknown-entity literal contract expands ampersands.
    {Server s;CHECK(s.start(nullptr));std::string id;for(unsigned i=0;i<2400;++i)id+="&x;";const auto data=request(id.c_str(),"<AreChunksInstalled><ChunkIds>1</ChunkIds></AreChunksInstalled>");CHECK(data.size()<8192);asio::write(s.client,asio::buffer(data));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().responses==0 && s.service.snapshot().malformed==1);}
    for(const std::string& bad:{request("bad","<AreChunksInstalled>bad<ChunkIds>1</ChunkIds></AreChunksInstalled>"),std::string("<LSX><Request><AreChunksInstalled/></Request></LSX>")}){Server s;CHECK(s.start(nullptr));asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1);}
    std::cout<<"TCP_CHUNKS_INT32_NO_RESPONSE_FIFO_ENTITY_EXPANSION_BOUND_PASS\n";
}
