// SPDX-License-Identifier: GPL-3.0-only
#include "src/hooks/crypt_query.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
namespace {
unsigned checks=0;
void expect(std::uint32_t type,bool null_object,std::u16string_view path,bool readable,bool companion){
    ++checks;
    if((preloader_l::crypt_query::select_object(type,null_object,path,readable)==preloader_l::crypt_query::Selection::companion)!=companion)
        throw std::runtime_error("crypt-query selection mismatch "+std::to_string(checks));
}
}
int main(){try{
    for(auto type:{0u,2u,3u,0xffffffffu})for(bool null_object:{false,true})for(bool readable:{false,true})
        expect(type,null_object,u"preloader_l.dll",readable,false);
    expect(1,true,{},false,true);expect(1,true,u"other.dll",true,true);
    expect(1,false,{},true,false);expect(1,false,u"preloader_l.dll",false,false);
    for(auto path:{u"preloader_l.dll",u"xpreloader_l.dll",u"C:\\fixture\\preloader_l.dll",u"\u00e7\u0131\u011f/preloader_l.dll",u"\U0001f3c8preloader_l.dll"})expect(1,false,path,true,true);
    for(auto path:{u"PRELOADER_L.DLL",u"preloader_L.dll",u"preloader_l.dll.bak",u"preloader_l.dl",u"other.dll",u"preloader_l.dll/"})expect(1,false,path,true,false);
    const std::u16string embedded(u"preloader_l.dll\0suffix",22);expect(1,false,embedded,true,true);
    const std::u16string before(u"other.dll\0preloader_l.dll",24);expect(1,false,before,true,false);
    for(char16_t unit:{char16_t(0xd800),char16_t(0xdbff),char16_t(0xdc00),char16_t(0xdfff)}){
        expect(1,false,std::u16string(1,unit)+u"preloader_l.dll",true,false);
        expect(1,false,std::u16string(1,unit),true,false);
    }
    // A paired non-ASCII scalar must not change the ASCII final-byte decision.
    for(std::uint32_t high=0xd800;high<=0xdbff;high+=31)for(std::uint32_t low=0xdc00;low<=0xdfff;low+=47){
        std::u16string path{static_cast<char16_t>(high),static_cast<char16_t>(low)};
        expect(1,false,path+u"preloader_l.dll",true,true);expect(1,false,path+u"PRELOADER_L.DLL",true,false);
    }
    expect(1,false,std::u16string(40000,u'a')+u"preloader_l.dll",true,true);
    std::cout<<"PASS crypt-query UTF16/null/type/case/suffix selection checks="<<checks<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
