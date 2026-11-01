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
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf16_ent_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const std::string* config){if(config){std::ofstream f(dir/"redfield.cfg",std::ios::binary);f<<*config;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const std::string& id,const std::string& command){return "<LSX><Request id='"+id+"'>"+command+"</Request></LSX>";}
std::string attr(const tinyxml2::XMLElement* e,const char* key){const auto* v=e->Attribute(key);return v?v:"!MISSING";}
int main(){
    const std::string empty=R"("Config2" {})",defaults=R"("Config2" {"Entitlements" {"A" {"Other" ""} "B" {"Other" ""}}})",data=R"("Config2" {"Emulator" {"Online" "true"} "Content" {"game" {"Name" "configured"}} "Entitlements" {"A" {"Group" "g1"} "B" {"Group" "g2" "EntitlementTag" "tag2"}}})";
    for(const auto* config:{static_cast<const std::string*>(nullptr),&empty,&defaults,&data}){Server s;CHECK(s.start(config));const auto held=s.service.configuration();CHECK(held);const auto count=held->entitlements.size();
        const auto frames=request("unbound","<GrantAchievement/>")+request("E&amp;lt;","<QueryEntitlements/>")+request("filtered","<QueryEntitlements ItemId='B'/>")+request("none","<QueryEntitlements OfferId='none'/>")+request("content","<QueryContent/>")+request("group","<QueryEntitlements Group='g2'><FilterItems>tag2</FilterItems></QueryEntitlements>");
        asio::write(s.client,asio::buffer(frames.data(),13));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(frames.data()+13,frames.size()-13));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().frames==6 && s.service.snapshot().responses==5 && s.service.snapshot().malformed==0 && s.service.configuration()==held);
        std::size_t offset=0;
        for(unsigned i=0;i<5;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument d;CHECK(d.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* response=d.RootElement()->FirstChildElement("Response");CHECK(response);const auto* body=response->FirstChildElement();CHECK(body);
            if(i==3){CHECK(attr(response,"id")=="content" && std::string(body->Name())=="QueryContentResponse");std::size_t n=0;for(auto* e=body->FirstChildElement();e;e=e->NextSiblingElement())++n;CHECK(n==held->content.size());}
            else {CHECK(std::string(body->Name())=="QueryEntitlementsResponse");const auto expected=i==0?count:i==1?(count?1u:0u):i==2?0u:(config==&data?1u:0u);std::size_t n=0;for(auto* e=body->FirstChildElement();e;e=e->NextSiblingElement()){CHECK(std::string(e->Name())=="Entitlement");if(i==1 || i==4)CHECK(attr(e,"ItemId")=="B" && attr(e,"EntitlementId")=="1000000000002");else CHECK(attr(e,"EntitlementId")==std::to_string(UINT64_C(1000000000001)+n));++n;}CHECK(n==expected);if(i==0)CHECK(attr(response,"id")=="E&lt;");}
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);CHECK(s.until([&]{return s.service.stop_for(1ms);}));CHECK(held->entitlements.size()==count);if(count)CHECK(held->entitlements[0].item_id=="A");
    }
    for(const auto* config:{&empty,&data}){Server s;CHECK(s.start(config));const auto bad=request("invalid","<QueryEntitlements>bad<FilterItems>tag2</FilterItems></QueryEntitlements>");asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
    {Server s;CHECK(s.start(nullptr));const std::string bad="<LSX><Request><QueryEntitlements/></Request></LSX>";asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1);}
    std::string huge="\"Config2\" {\"Entitlements\" {";for(unsigned i=0;i<100;++i)huge+='"'+std::to_string(i)+"\" {\"Other\" \"\"}";huge+="}}";
    {Server s;CHECK(s.start(&huge));const auto frame=request("big","<QueryEntitlements/>");asio::write(s.client,asio::buffer(frame));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0 && s.service.configuration()->entitlements.size()==100);}
    const std::string unsafe="\"Config2\" {\"Entitlements\" {\"A\" {\"Group\" \"ok\"} \"B\" {\"Type\" \"bad"+std::string(1,'\1')+"\"}}}";
    {Server s;CHECK(s.start(&unsafe));const auto frame=request("excluded","<QueryEntitlements ItemId='A'/>");asio::write(s.client,asio::buffer(frame));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
    std::cout<<"TCP_ENTITLEMENTS_FILTER_SEQUENCE_CONTENT_FIFO_EMPTY_PREVALIDATION_CAP_PASS\n";
}
