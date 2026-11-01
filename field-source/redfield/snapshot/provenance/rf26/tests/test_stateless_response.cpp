// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/response_dispatch.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <cstring>
#include <iostream>
using namespace preloader_l;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
int main(){
    struct Case{const char* command;const char* body;};
    const Case cases[]={{"QueryFriends","<QueryFriendsResponse Return=\"Success\"/>"},{"GetBlockList","<GetBlockListResponse Return=\"Success\"/>"},{"GetPresenceVisibility","<GetPresenceVisibilityResponse Visible=\"false\"/>"},{"GetGroupInfo","<ErrorSuccess Description=\"User not in a party, create one.\" Code=\"-1577058296\"/>"},{"GetVoipStatus","<ErrorSuccess Description=\"The function you are calling is not implemented.\" Code=\"-1593835518\"/>"}};
    std::array<char,16384> out;
    for(const auto& c:cases){
        const auto input=std::string("<LSX><Request id='A&amp;lt;' recipient='SDK&amp;X'><")+c.command+"/></Request></LSX>";
        xml::Request request;CHECK(xml::parse(input,request)==xml::Result::ok);std::size_t n=99;
        CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::response);
        const auto expected=std::string(" <LSX>\n  <Response id=\"A&amp;lt;\" sender=\"SDK&amp;X\">\n    ")+c.body+"\n  </Response>\n </LSX>\n";
        CHECK(std::string_view(out.data(),n)==expected && out[n]=='\0');
        tinyxml2::XMLDocument doc;CHECK(doc.Parse(out.data(),n)==tinyxml2::XML_SUCCESS);const auto* response=doc.RootElement()->FirstChildElement("Response");CHECK(response && std::strcmp(response->Attribute("id"),"A&lt;")==0);
        out.fill('#');CHECK(dispatch_response(request,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');
        request.id={};CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::rejected && n==0);
    }
    xml::Request request;std::size_t n=99;CHECK(xml::parse("<LSX><Request id='chunk'><QueryChunkStatus/></Request></LSX>",request)==xml::Result::ok);
    out.fill('#');CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::no_response && n==0 && out[0]=='#');request.id={};CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::rejected && n==0);
    CHECK(xml::parse("<LSX><Request id='mutate'><BlockUser UserId='123'/></Request></LSX>",request)==xml::Result::ok);CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);
    std::cout<<"STATELESS_LITERAL_WIRE_AND_NO_RESPONSE_PASS\n";
}
