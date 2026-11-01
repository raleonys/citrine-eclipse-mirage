#include "../../tools/preloader_l_native_acceptance_host/achievement_frame_contract.hpp"
#include <cstring>
#include <cstdio>
#include <string>
#include <array>
using namespace achievement_frame_contract;
int main(){
    static constexpr char id[]="<Achievement Id=\"NATIVE_CAMPAIGN18\"";
    static constexpr char response_xml[]=" <LSX>\n  <Achievement Id=\"NATIVE_CAMPAIGN18\"/>\n </LSX>\n";
    static constexpr char event_xml[]="<LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"NATIVE_CAMPAIGN18\"/></Event></LSX>";
    static constexpr char wrong[]="<LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"WRONG\"/></Event></LSX>";
    static constexpr char duplicate[]="<LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"NATIVE_CAMPAIGN18\"/><Achievement Id=\"NATIVE_CAMPAIGN18\"/></Event></LSX>";
    static constexpr char combined[]="<LSX><Achievement Id=\"NATIVE_CAMPAIGN18\"/></LSX><LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"NATIVE_CAMPAIGN18\"/></Event></LSX>";
    int tests=0;
    auto check=[&](bool ok){++tests;if(!ok){std::fprintf(stderr,"failed case=%d\n",tests);return false;}return true;};
    const size_t m=sizeof(id)-1u;
    if(!check(response(response_xml,sizeof(response_xml)-1u,id,m)))return 1;
    if(!check(event(event_xml,sizeof(event_xml)-1u,id,m)))return 2;
    if(!check(!response(event_xml,sizeof(event_xml)-1u,id,m)))return 3;
    if(!check(!event(response_xml,sizeof(response_xml)-1u,id,m)))return 4;
    if(!check(!response(combined,sizeof(combined)-1u,id,m)))return 5;
    if(!check(!event(combined,sizeof(combined)-1u,id,m)))return 6;
    if(!check(!event(wrong,sizeof(wrong)-1u,id,m)))return 7;
    if(!check(!event(duplicate,sizeof(duplicate)-1u,id,m)))return 8;
    if(!check(!event("",0u,id,m)))return 9;
    if(!check(!response("",0u,id,m)))return 10;
    if(!check(!response(response_xml,sizeof(response_xml)-1u,nullptr,0u)))return 11;
    if(!check(!event(event_xml,sizeof(event_xml)-1u,nullptr,0u)))return 12;
    static constexpr char formatted[]=" <LSX>\n <Event sender=\"EbisuSDK\">\n   <Achievement Name=\"AB&amp;C\" Id=\"NATIVE_CAMPAIGN18\" Count=\"1\"/>\n </Event>\n </LSX>\n";
    static constexpr char wrong_sender[]="<LSX><Event sender=\"Other\"><Achievement Id=\"NATIVE_CAMPAIGN18\"/></Event></LSX>";
    static constexpr char extra[]="<LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"NATIVE_CAMPAIGN18\"/><Other/></Event></LSX>";
    static constexpr char attr_only[]="<LSX><Event sender=\"EbisuSDK\"><Other Id=\"NATIVE_CAMPAIGN18\"/></Event></LSX>";
    static constexpr char prefix[]="<LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"NATIVE_CAMPAIGN18\"/></Event></LSX><Extra/>";
    static constexpr char close_missing[]="<LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"NATIVE_CAMPAIGN18\"/></Event>";
    static constexpr char nul_tail[]="<LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"NATIVE_CAMPAIGN18\"/></Event></LSX>\0<Other/>";
    static constexpr char response_extra[]=" <LSX><Achievement Id=\"NATIVE_CAMPAIGN18\"/><Other/></LSX>";
    if(!check(event(formatted,sizeof(formatted)-1u,id,m)))return 13;
    if(!check(!event(wrong_sender,sizeof(wrong_sender)-1u,id,m)))return 14;
    if(!check(!event(extra,sizeof(extra)-1u,id,m)))return 15;
    if(!check(!event(attr_only,sizeof(attr_only)-1u,id,m)))return 16;
    if(!check(!event(prefix,sizeof(prefix)-1u,id,m)))return 17;
    if(!check(!event(close_missing,sizeof(close_missing)-1u,id,m)))return 18;
    if(!check(!event(nul_tail,sizeof(nul_tail)-1u,id,m)))return 19;
    if(!check(!response(response_extra,sizeof(response_extra)-1u,id,m)))return 20;
    // The native wire codec returns unstripped PKCS#7 decrypted blocks.
    // This exercised real 224-byte/2-padding event condition from Windows.
    std::string event_222(event_xml,sizeof(event_xml)-1u);
    if(event_222.size()>222u)return 21;
    event_222.insert(event_222.find("/>"),222u-event_222.size(),' ');
    if(!check(event_222.size()==222u))return 22;
    std::string padded_224=event_222+std::string(2,static_cast<char>(2));
    size_t plain_n=0;
    if(!check(strict_pkcs7_xml_length(padded_224.data(),padded_224.size(),&plain_n)&&plain_n==222u))return 23;
    if(!check(event(padded_224.data(),plain_n,id,m)))return 24;
    if(!check(!event(padded_224.data(),padded_224.size(),id,m)))return 25;
    // Every PKCS#7 length is legal only when *all* tail bytes match.
    for(unsigned padding=1u;padding<=16u;++padding){
        std::string block(event_xml,sizeof(event_xml)-1u);
        while(((block.size()+padding)&15u)!=0u)block.push_back(' ');
        block.append(padding,static_cast<char>(padding));
        size_t actual=0;
        if(!check(strict_pkcs7_xml_length(block.data(),block.size(),&actual)&&
                  actual==block.size()-padding && event(block.data(),actual,id,m)))return 26;
        block[block.size()-padding]=static_cast<char>(padding+1u);
        if(!check(!strict_pkcs7_xml_length(block.data(),block.size(),&actual)&&actual==0u))return 27;
    }
    std::string invalid_0(16,'A');invalid_0.back()='\0';
    if(!check(!strict_pkcs7_xml_length(invalid_0.data(),invalid_0.size(),&plain_n)))return 28;
    std::string invalid_17(32,'A');invalid_17.back()=static_cast<char>(17);
    if(!check(!strict_pkcs7_xml_length(invalid_17.data(),invalid_17.size(),&plain_n)))return 29;
    if(!check(!strict_pkcs7_xml_length(padded_224.data(),padded_224.size()-1u,&plain_n)))return 30;
    if(!check(!strict_pkcs7_xml_length(nullptr,padded_224.size(),&plain_n)))return 31;
    if(!check(!strict_pkcs7_xml_length(padded_224.data(),padded_224.size(),nullptr)))return 32;
    std::string all_padding(16,static_cast<char>(16));
    if(!check(!strict_pkcs7_xml_length(all_padding.data(),all_padding.size(),&plain_n)))return 33;
    // Matching padding is not sufficient: no trailing XML, wrong sender or
    // extra elements can bypass the owned TinyXML2 DOM schema check.
    std::string bad_extra(prefix,sizeof(prefix)-1u);bad_extra.append(1,static_cast<char>(1));
    if(!check(!event(bad_extra.data(),bad_extra.size()-1u,id,m)))return 34;
    std::string xml_extra(prefix,sizeof(prefix)-1u);
    xml_extra.append(1,static_cast<char>(1));
    if(!check(!strict_pkcs7_xml_length(xml_extra.data(),xml_extra.size(),&plain_n)))return 35;
    // Correct padding does not excuse invalid, extra, or reversed XML.
    const char* rejected[]={wrong,duplicate,combined,close_missing,wrong_sender,extra};
    for(const char* bad:rejected){
        std::string frame(bad);
        const size_t pad=16u-(frame.size()&15u);
        frame.append(pad,static_cast<char>(pad));
        size_t xml_len=0;
        if(!check(strict_pkcs7_xml_length(frame.data(),frame.size(),&xml_len)&&
                  !event(frame.data(),xml_len,id,m)))return 36;
    }
    std::string response_padded(response_xml,sizeof(response_xml)-1u);
    const size_t response_pad=16u-(response_padded.size()&15u);
    response_padded.append(response_pad,static_cast<char>(response_pad));
    size_t rsp_n=0;
    if(!check(strict_pkcs7_xml_length(response_padded.data(),response_padded.size(),&rsp_n)&&
              response(response_padded.data(),rsp_n,id,m)))return 37;
    std::printf("ACHIEVEMENT_WIRE_FRAME_CONTRACT: PASS (%d/%d)\n",tests,tests);
    return 0;
}
