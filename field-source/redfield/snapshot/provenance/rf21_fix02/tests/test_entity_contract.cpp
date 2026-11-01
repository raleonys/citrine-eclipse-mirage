// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/response_dispatch.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <cstring>
#include <iostream>
using namespace redfield;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
int main(){
    struct Case{const char* source;const char* semantic;};
    const Case cases[]={{"bad&bogus;","bad&bogus;"},{"bad&amp;bogus;","bad&bogus;"},{"&amp;lt;","&lt;"},{"&lt;","<"},{"&#38;","&"},{"&#x26;","&"},{"&quot;&apos;","\"'"},{"plain","plain"}};
    std::string raw_unknown_reply,encoded_unknown_reply;
    for(const auto& c:cases){
        const auto source=std::string("<LSX><Request id='")+c.source+"' recipient='E&amp;ALS'><GetConfig/></Request></LSX>";
        xml::Request request;CHECK(xml::parse(source,request)==xml::Result::ok);
        CHECK(request.id.decoded() && preloader_l::xml::span_equals(request.id,c.semantic));
        std::array<char,16384> output;std::size_t n=0;CHECK(dispatch_response(request,output.data(),output.size(),&n)==ResponseResult::response);
        tinyxml2::XMLDocument response;CHECK(response.Parse(output.data(),n)==tinyxml2::XML_SUCCESS);
        const auto* envelope=response.RootElement()->FirstChildElement("Response");CHECK(envelope && std::strcmp(envelope->Attribute("id"),c.semantic)==0 && std::strcmp(envelope->Attribute("sender"),"E&ALS")==0);
        if(std::string_view(c.source)=="bad&bogus;")raw_unknown_reply.assign(output.data(),n);
        if(std::string_view(c.source)=="bad&amp;bogus;")encoded_unknown_reply.assign(output.data(),n);
    }
    CHECK(raw_unknown_reply==encoded_unknown_reply);
    std::cout<<"ENTITY_DOM_RESPONSE_MATRIX_PASS:8; original legacy GetConfig diagnostic remains separately FAIL_OBSERVED\n";
}
