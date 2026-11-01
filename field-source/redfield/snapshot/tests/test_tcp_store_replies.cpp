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
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf14_store_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const char* configuration){if(configuration){std::ofstream f(dir/"surge.cfg",std::ios::binary);f<<configuration;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const std::string& id,const std::string& command){return "<LSX><Request id='"+id+"' recipient='Store&amp;X'>"+command+"</Request></LSX>";}
std::string attr(const tinyxml2::XMLElement* e,const char* key){const auto* v=e->Attribute(key);return v?v:"!MISSING";}
int main(){
    const char* configs[]={nullptr,R"("Config2" {})",R"("Config2" {"Emulator" {"Online" "1"}})",R"("Config2" {"Emulator" {"Online" "false"}})",R"("Config2" {"Emulator" {"Online" "true"} "Store" {"Balance" "999"}})"};
    for(const char* config:configs){Server s;CHECK(s.start(config));const auto snapshot=s.service.configuration();CHECK(snapshot);
        const auto data=request("unbound","<GrantAchievement/>")+request("offers","<QueryOffers UserId='invalid'><FilterOffers>arbitrary</FilterOffers></QueryOffers>")+request("select","<SelectStore StoreId='ignored'/>")+request("S&amp;lt;","<GetWalletBalance Currency='XYZ'/>")+request("auth","<GetAuthCode ClientId=''/>")+request("wallet-again","<GetWalletBalance/>")+request("permission","<CheckPermission/>")+request("still-unbound","<GrantAchievement/>");
        asio::write(s.client,asio::buffer(data.data(),17));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(data.data()+17,data.size()-17));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().frames==8 && s.service.snapshot().responses==6 && s.service.snapshot().malformed==0 && s.service.configuration()==snapshot);
        std::size_t offset=0;
        for(unsigned i=0;i<6;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument d;CHECK(d.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* response=d.RootElement()->FirstChildElement("Response");CHECK(response && attr(response,"sender")=="Store&X");const auto* body=response->FirstChildElement();CHECK(body);
            if(i==0)CHECK(attr(response,"id")=="offers" && std::string(body->Name())=="QueryOffersResponse" && !body->FirstAttribute() && !body->FirstChild());
            else if(i==1)CHECK(attr(response,"id")=="select" && std::string(body->Name())=="ErrorSuccess" && attr(body,"Code")=="0" && attr(body,"Description")=="");
            else if(i==2 || i==4)CHECK(attr(response,"id")== (i==2?"S&lt;":"wallet-again") && std::string(body->Name())=="GetWalletBalanceResponse" && attr(body,"Balance")=="0");
            else if(i==3){CHECK(attr(response,"id")=="auth");if(snapshot->online)CHECK(std::string(body->Name())=="AuthCode" && attr(body,"value").size()==8);else CHECK(std::string(body->Name())=="ErrorSuccess" && attr(body,"Code")=="-1577058293");}
            else {CHECK(attr(response,"id")=="permission");if(snapshot->online)CHECK(std::string(body->Name())=="CheckPermissionResponse");else CHECK(attr(body,"Code")=="-1576468080");}
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);CHECK(s.until([&]{return s.service.stop_for(1ms);}));
    }
    for(const char* name:{"QueryOffers","SelectStore","GetWalletBalance"}){
        {Server s;CHECK(s.start(nullptr));const std::string bad="<LSX><Request><"+std::string(name)+"/></Request></LSX>";asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
        {Server s;CHECK(s.start(nullptr));std::string id;for(unsigned i=0;i<2400;++i)id+="&x;";const auto bad=request(id,"<"+std::string(name)+"/>");CHECK(bad.size()<8192);asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
    }
    std::cout<<"TCP_STORE_PURE_REPLIES_FIFO_CONFIG_AUTH_PERMISSION_CAPACITY_PASS\n";
}
