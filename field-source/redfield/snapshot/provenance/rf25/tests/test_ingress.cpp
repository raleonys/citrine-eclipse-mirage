// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/net/frame_ingress.hpp"
#include "redfield/xml.hpp"
#include <iostream>
#include <string>
using namespace redfield::net;
using redfield::xml::TextSpan;
#define CHECK(x) do{if(!(x)){std::cerr<<"line "<<__LINE__<<": " #x "\n";return 1;}}while(false)
int main(){
    IngressState state;reset_ingress(&state);TextSpan frame;
    const std::string a="<LSX><Request id='a&amp;lt;'><A><Value><![CDATA[</LSX>]]></Value></A></Request></LSX>";
    for(std::size_t i=0;i<a.size();++i){
        auto result=feed_ingress(&state,reinterpret_cast<const std::uint8_t*>(a.data()+i),1,&frame);
        CHECK(result==(i+1==a.size()?IngressStatus::frame_ready:IngressStatus::need_more));
    }
    redfield::xml::Request request;CHECK(redfield::xml::parse({frame.data,frame.size},request)==redfield::xml::Result::ok);
    CHECK(redfield::xml::span_equals(request.id,"a&lt;"));CHECK(consume_ingress_frame(&state,frame.size)==IngressStatus::ok);
    CHECK(state.receive.size==0 && state.frames_consumed==1);
    const auto both=a+a;CHECK(feed_ingress(&state,reinterpret_cast<const std::uint8_t*>(both.data()),both.size(),&frame)==IngressStatus::frame_ready);
    CHECK(consume_ingress_frame(&state,frame.size-1)==IngressStatus::invalid_frame);
    CHECK(consume_ingress_frame(&state,frame.size)==IngressStatus::ok);
    CHECK(peek_ingress_frame(&state,&frame)==IngressStatus::frame_ready && frame.size==a.size());
    CHECK(consume_ingress_frame(&state,frame.size)==IngressStatus::ok);
    reset_ingress(&state);const std::string too_many(receive_buffer_capacity+1,'x');
    CHECK(feed_ingress(&state,reinterpret_cast<const std::uint8_t*>(too_many.data()),too_many.size(),&frame)==IngressStatus::capacity && state.receive.size==0);
    const std::string bad="BAD";CHECK(feed_ingress(&state,reinterpret_cast<const std::uint8_t*>(bad.data()),bad.size(),&frame)==IngressStatus::invalid_frame);
    std::cout<<"INGRESS_PASS\n";
}
