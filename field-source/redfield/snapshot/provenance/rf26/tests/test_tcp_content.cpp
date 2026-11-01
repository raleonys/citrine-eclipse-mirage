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
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf15_content_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const std::string* configuration){if(configuration){std::ofstream f(dir/"preloader_l.cfg",std::ios::binary);f<<*configuration;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const std::string& id,const std::string& command){return "<LSX><Request id='"+id+"'>"+command+"</Request></LSX>";}
std::string attr(const tinyxml2::XMLElement* e,const char* key){const auto* v=e->Attribute(key);return v?v:"!MISSING";}
int main(){
    const std::string empty=R"("Config2" {})",defaults=R"("Config2" {"Content" {"id" {"Unknown" ""}}})",data=R"("Config2" {"Emulator" {"Online" "true"} "Content" {"A&" {"Name" "First" "Version" "3"} "B" {"State" "CUSTOM"} "A&" {"Name" "Last"}}})";
    for(const auto* config:{static_cast<const std::string*>(nullptr),&empty,&defaults,&data}){
        Server s;CHECK(s.start(config));const auto held=s.service.configuration();CHECK(held);const auto count=held->content.size();
        const auto frames=request("unbound","<GrantAchievement/>")+request("C&amp;lt;","<QueryContent UserId='invalid'><Other/></QueryContent>")+request("select","<SelectStore StoreId='ignored'/>")+request("again","<QueryContent/>")+request("wallet","<GetWalletBalance/>");
        asio::write(s.client,asio::buffer(frames.data(),13));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(frames.data()+13,frames.size()-13));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().frames==5 && s.service.snapshot().responses==4 && s.service.snapshot().malformed==0 && s.service.configuration()==held);
        std::size_t offset=0;
        for(unsigned i=0;i<4;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument d;CHECK(d.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* response=d.RootElement()->FirstChildElement("Response");CHECK(response);const auto* body=response->FirstChildElement();CHECK(body);
            if(i==0 || i==2){CHECK(attr(response,"id")== (i==0?"C&lt;":"again") && std::string(body->Name())=="QueryContentResponse");std::size_t n=0;for(const auto* game=body->FirstChildElement();game;game=game->NextSiblingElement()){CHECK(n<count);const auto& rec=held->content[n++];CHECK(std::string(game->Name())=="Game" && attr(game,"contentID")==rec.content_id && attr(game,"displayName")==rec.display_name && attr(game,"state")==rec.state && attr(game,"availableVersion")==rec.version && attr(game,"installedVersion")==rec.version && attr(game,"progressValue")=="0");}CHECK(n==count);}
            else if(i==1)CHECK(std::string(body->Name())=="ErrorSuccess" && attr(body,"Code")=="0");
            else CHECK(std::string(body->Name())=="GetWalletBalanceResponse" && attr(body,"Balance")=="0");
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);CHECK(s.until([&]{return s.service.stop_for(1ms);}));CHECK(held->content.size()==count);
    }
    {Server s;CHECK(s.start(nullptr));const std::string bad="<LSX><Request><QueryContent/></Request></LSX>";asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
    std::string huge="\"Config2\" {\"Content\" {";for(unsigned i=0;i<180;++i)huge+='"'+std::to_string(i)+"\" {\"Name\" \"long content name\"}";huge+="}}";
    {Server s;CHECK(s.start(&huge));const auto frame=request("big","<QueryContent/>");asio::write(s.client,asio::buffer(frame));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0 && s.service.configuration()->content.size()==180);}
    // A syntactically accepted raw Config2 value can still be invalid XML text.
    const std::string invalid="\"Config2\" {\"Content\" {\"x\" {\"Name\" \"bad"+std::string(1,'\1')+"\"}}}";
    {Server s;CHECK(s.start(&invalid));const auto frame=request("invalid","<QueryContent/>");asio::write(s.client,asio::buffer(frame));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
    std::cout<<"TCP_QUERY_CONTENT_CFG_TYPED_DATA_FIFO_HALF_CLOSE_CAP_INVALID_TEXT_PASS\n";
}
