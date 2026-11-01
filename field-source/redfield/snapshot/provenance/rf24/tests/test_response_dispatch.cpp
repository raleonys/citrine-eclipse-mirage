// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/response_dispatch.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <cstring>
#include <iostream>
using namespace redfield;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
int main(){
    std::array<char,16384> out{};std::size_t n=99;xml::Request request;
    CHECK(xml::parse("<LSX><Request id='A&amp;lt;' recipient='S&amp;quot;'><GetConfig/></Request></LSX>",request)==xml::Result::ok);
    CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::response);
    tinyxml2::XMLDocument response;CHECK(response.Parse(out.data(),n)==tinyxml2::XML_SUCCESS);
    auto* r=response.RootElement()->FirstChildElement("Response");CHECK(r && std::strcmp(r->Attribute("id"),"A&lt;")==0 && std::strcmp(r->Attribute("sender"),"S&quot;")==0);
    CHECK(r->FirstChildElement("GetConfigResponse"));
    out.fill('#');CHECK(dispatch_response(request,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');
    CHECK(xml::parse("<LSX><Request id='2'><IsProgressiveInstallationAvailable ItemId='ignored'/></Request></LSX>",request)==xml::Result::ok);
    CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::response);
    CHECK(std::string_view(out.data(),n).find("Available=\"false\"")!=std::string_view::npos);
    for(const char* command:{"GetProfile","UnknownRF03"}){
        const auto text=std::string("<LSX><Request id='3'><")+command+"/></Request></LSX>";
        CHECK(xml::parse(text,request)==xml::Result::ok);n=99;CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);
    }
    CHECK(xml::parse("<LSX><Request><GetConfig/></Request></LSX>",request)==xml::Result::ok);
    CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::rejected && n==0);
    CHECK(xml::parse("<LSX><Request id='bad&bogus;'><GetConfig/></Request></LSX>",request)==xml::Result::ok);
    CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::response);
    CHECK(std::string_view(out.data(),n).find("bad&amp;bogus;")!=std::string_view::npos);
    CHECK(dispatch_response(request,out.data(),out.size(),nullptr)==ResponseResult::rejected);
    std::cout<<"PARSED_RESPONSE_DISPATCH_PASS\n";
}
