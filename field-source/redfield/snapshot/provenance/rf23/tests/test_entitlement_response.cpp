// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/entitlement_response.hpp"
#include "redfield/config.hpp"
#include "redfield/auth_code_session.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <atomic>
#include <iostream>
#include <thread>
using namespace redfield;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
bool parse_fixture(const std::string& attributes,const std::string& children,xml::Request& r){std::string source="<LSX><Request id='E&amp;lt;' recipient='Store&amp;X'><QueryEntitlements"+attributes+">"+children+"</QueryEntitlements></Request></LSX>";const auto status=xml::parse(source,r);source.assign(source.size(),'!');return status==xml::Result::ok;}
ResponseResult render(const ConfigSnapshot& snapshot,const std::string& attributes,const std::string& children,std::string& wire){xml::Request r;if(!parse_fixture(attributes,children,r))return ResponseResult::rejected;std::array<char,16384> out{};std::size_t n=99;const auto result=dispatch_response_with_config(r,snapshot,out.data(),out.size(),&n);wire.assign(out.data(),n);return result;}
std::vector<std::string> ids(const std::string& wire){tinyxml2::XMLDocument d;std::vector<std::string> result;if(d.Parse(wire.data(),wire.size())!=tinyxml2::XML_SUCCESS)return {"!INVALID"};const auto* response=d.RootElement()->FirstChildElement();if(!response || !response->Attribute("id","E&lt;") || !response->Attribute("sender","Store&X"))return {"!ENVELOPE"};const auto* body=response->FirstChildElement();if(!body || std::string(body->Name())!="QueryEntitlementsResponse")return {"!BODY"};for(auto* e=body->FirstChildElement();e;e=e->NextSiblingElement()){if(!e->Attribute("EntitlementId"))return {"!ID"};result.emplace_back(e->Attribute("EntitlementId"));}return result;}
int main(){
    ConfigStore store;std::string wire;CHECK(render(*store.snapshot(),"","",wire)==ResponseResult::response && ids(wire).empty());
    CHECK(store.replace(R"("Config2" {"Emulator" {"Online" "false"} "Entitlements" {"A&" {"Group" "g1" "Version" "1" "Type" "T&" "EntitlementTag" "tag1"} "B" {"Group" "g2" "EntitlementTag" "tag2"} "C" {"Group" "" "EntitlementTag" ""}}})")==ConfigResult::published);const auto held=store.snapshot();
    struct Case{const char* attr;const char* children;std::vector<std::string> expected;};
    const std::vector<Case> cases={
        {"","",{"1000000000001","1000000000002","1000000000003"}},
        {" OfferId='A&amp;'","",{"1000000000001"}},
        {" ItemId='B'","",{"1000000000002"}},
        {" OfferId='none' ItemId='B'","",{"1000000000002"}},
        {" OfferId='A&amp;'","<FilterOffers>B</FilterOffers>",{"1000000000001","1000000000002"}},
        {" Group='g2'","",{"1000000000002"}},
        {" OfferId='A&amp;' Group='g2'","",{}},
        {" OfferId='A&amp;' Group='g2'","<FilterOffers>B</FilterOffers>",{"1000000000002"}},
        {"","<FilterItems>tag2</FilterItems>",{"1000000000002"}},
        {"","<FilterItems>B</FilterItems>",{}},
        {"","<FilterOffers>A&#38;</FilterOffers>",{"1000000000001"}},
        {"","<FilterOffers><![CDATA[A&]]></FilterOffers>",{"1000000000001"}},
        {"","<FilterOffers> A&amp; </FilterOffers>",{}},
        {"","<FilterOffers><Item>B</Item></FilterOffers>",{"1000000000001","1000000000002","1000000000003"}},
        {"","<FilterGroups/>",{"1000000000003"}},
        {"","<FilterGroups><!--comment--></FilterGroups>",{"1000000000003"}},
        {"","<FilterGroups>g1<!--comment-->tail</FilterGroups>",{"1000000000003"}},
        {"","<filteroffers>B</filteroffers>",{"1000000000001","1000000000002","1000000000003"}},
        {"","<FilterOffers>A&amp;</FilterOffers><FilterOffers>B</FilterOffers>",{"1000000000001","1000000000002"}},
        {"","<Other><FilterOffers>B</FilterOffers></Other>",{"1000000000001","1000000000002","1000000000003"}},
        {" OfferId=''","",{}},
        {" UserId='invalid'","<Other/>",{"1000000000001","1000000000002","1000000000003"}}
    };
    for(const auto& c:cases){CHECK(render(*held,c.attr,c.children,wire)==ResponseResult::response && ids(wire)==c.expected);}
    CHECK(render(*held," OfferId='A&amp;'","",wire)==ResponseResult::response);
    const std::string literal=" <LSX>\n  <Response id=\"E&amp;lt;\" sender=\"Store&amp;X\">\n    <QueryEntitlementsResponse>\n      <Entitlement ItemId=\"A&amp;\" Type=\"T&amp;\" EntitlementId=\"1000000000001\" EntitlementTag=\"tag1\" Group=\"g1\" ResourceId=\"\" GrantDate=\"0000-00-00T00:00:00\" Expiration=\"0000-00-00T00:00:00\" LastModifiedDate=\"0000-00-00T00:00:00\" Version=\"1\" UseCount=\"0\"/>\n    </QueryEntitlementsResponse>\n  </Response>\n </LSX>\n";CHECK(wire==literal);
    xml::Request r;CHECK(parse_fixture(" OfferId='A&amp;'","",r));std::array<char,16384> out;out.fill('#');std::size_t n=99;
    CHECK(dispatch_response(r,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);AuthCodeSession auth;CHECK(dispatch_response_with_session(r,*held,auth,out.data(),out.size(),&n)==ResponseResult::response && std::string(out.data(),n)==literal && !auth.ready());const auto need=n;
    out.fill('#');CHECK(dispatch_entitlement_response(r,*held,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_entitlement_response(r,*held,out.data(),need,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_entitlement_response(r,*held,out.data(),need+1,&n)==ResponseResult::response && n==need && out[n]=='\0');CHECK(dispatch_entitlement_response(r,*held,nullptr,out.size(),&n)==ResponseResult::rejected && n==0);CHECK(dispatch_entitlement_response(r,*held,out.data(),0,&n)==ResponseResult::rejected && n==0);CHECK(dispatch_entitlement_response(r,*held,out.data(),out.size(),nullptr)==ResponseResult::rejected);
    const auto id=r.id;r.id={};out.fill('#');CHECK(dispatch_entitlement_response(r,*held,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');r.id=id;
    // Validation is before filtering: unsafe metadata on excluded C still rejects.
    for(unsigned field=0;field<5;++field){ConfigSnapshot bad=*held;auto& e=bad.entitlements.back();std::string* v[]={&e.item_id,&e.group,&e.version,&e.type,&e.entitlement_tag};*v[field]=std::string(1,'\1');out.fill('#');CHECK(dispatch_entitlement_response(r,bad,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');}
    ConfigSnapshot empty;const auto tag=r.command_open_tag;r.command_open_tag={};out.fill('#');CHECK(dispatch_entitlement_response(r,empty,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');r.command_open_tag=tag;
    CHECK(parse_fixture("","non-whitespace<FilterOffers>B</FilterOffers>",r));out.fill('#');CHECK(dispatch_entitlement_response(r,empty,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');
    CHECK(store.replace(R"("Config2" {"Entitlements" {"" {"Other" ""} "raw&amp;" {"Other" ""}}})")==ConfigResult::published);CHECK(render(*store.snapshot()," OfferId=''","",wire)==ResponseResult::response && ids(wire)==std::vector<std::string>{"1000000000001"});CHECK(render(*store.snapshot()," OfferId='raw&amp;amp;'","",wire)==ResponseResult::response && ids(wire)==std::vector<std::string>{"1000000000002"});
    CHECK(store.replace(R"("Config2" {})")==ConfigResult::published);CHECK(render(*store.snapshot(),"","",wire)==ResponseResult::response && ids(wire).empty());CHECK(render(*held," ItemId='B'","",wire)==ResponseResult::response && ids(wire)==std::vector<std::string>{"1000000000002"});
    ConfigSnapshot many;for(unsigned i=0;i<600;++i)many.entitlements.push_back({std::to_string(i),"g","0","DEFAULT","ONLINE_ACCESS"});CHECK(parse_fixture("","",r));std::string large(500000,'#');CHECK(dispatch_entitlement_response(r,many,large.data(),large.size(),&n)==ResponseResult::response && n>16384);CHECK(ids(std::string(large.data(),n)).size()==600);out.fill('#');CHECK(dispatch_entitlement_response(r,many,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');
    const char* yes=R"("Config2" {"Emulator" {"Online" "true"} "Entitlements" {"A" {"Group" "gA"}}})";const char* no=R"("Config2" {"Emulator" {"Online" "false"} "Entitlements" {"B" {"Group" "gB"}}})";CHECK(store.replace(yes)==ConfigResult::published);std::atomic<bool> bad=false;std::thread writer([&]{for(unsigned i=0;i<200;++i)if(store.replace(i%2?yes:no)!=ConfigResult::published)bad=true;});
    for(unsigned i=0;i<200;++i){const auto s=store.snapshot();std::string w;if(render(*s,"","",w)!=ResponseResult::response || w.find(s->online?"ItemId=\"A\"":"ItemId=\"B\"")==std::string::npos || w.find(s->online?"Group=\"gA\"":"Group=\"gB\"")==std::string::npos)bad=true;}
    writer.join();CHECK(!bad);
    std::cout<<"QUERY_ENTITLEMENTS_FILTER_OR_AND_STABLE_SEQUENCE_DOM_MEASURE_SNAPSHOT_PASS\n";
}
