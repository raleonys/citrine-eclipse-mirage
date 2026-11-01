// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/response_dispatch.hpp"
#include "preloader_l/runtime/utc_time.hpp"
#include "utc_test_support.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <chrono>
#include <iostream>
using namespace redfield;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
int main(){
    const auto before=utc_wall_seconds();CHECK(before>0);
    char utc[20];std::size_t n=0;CHECK(preloader_l::runtime::current_utc_timestamp(utc,sizeof(utc),&n) && n==19 && utc[19]=='\0');
    const auto seconds=preloader_l::runtime::current_unix_seconds();
    const auto after=utc_wall_seconds();CHECK(seconds>=static_cast<std::uint64_t>(before) && seconds<=static_cast<std::uint64_t>(after));
    CHECK(std::string(utc)>=utc_text(before) && std::string(utc)<=utc_text(after));
    CHECK(!preloader_l::runtime::current_utc_timestamp(utc,19,&n));
    CHECK(!preloader_l::runtime::current_utc_timestamp(nullptr,20,&n));
    xml::Request request;CHECK(xml::parse("<LSX><Request id='T&amp;lt;' recipient='Clock&amp;X'><GetUTCTime/></Request></LSX>",request)==xml::Result::ok);
    std::array<char,16384> out;const auto start=utc_wall_seconds();
    CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::response);
    tinyxml2::XMLDocument document;CHECK(document.Parse(out.data(),n)==tinyxml2::XML_SUCCESS);
    const auto* response=document.RootElement()->FirstChildElement("Response");const auto* body=response->FirstChildElement("GetUTCTimeResponse");CHECK(body && body->Attribute("Time"));const std::string stamp=body->Attribute("Time");
    CHECK(stamp>=utc_text(start) && stamp<=utc_text(utc_wall_seconds()));
    const auto exact=" <LSX>\n  <Response id=\"T&amp;lt;\" sender=\"Clock&amp;X\">\n    <GetUTCTimeResponse Time=\""+stamp+"\"/>\n  </Response>\n </LSX>\n";
    CHECK(std::string(out.data(),n)==exact && out[n]=='\0');
    out.fill('#');CHECK(dispatch_response(request,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');
    request.id={};CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::rejected && n==0);
    std::cout<<"UTC_ACTUAL_CLOCK_LITERAL_ENVELOPE_PASS\n";
}
