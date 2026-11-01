#pragma once
// Native test host: validate two decrypted LSX documents through the *same*
// vendored TinyXML2 grammar as production, not an ad-hoc XML byte scanner.
// This header is not part of the shipping DLL or public native ABI.
#include "../../third_party/tinyxml2/tinyxml2.h"
#include <stddef.h>
#include <string.h>

namespace achievement_frame_contract {
constexpr size_t npos=static_cast<size_t>(-1);
// decode_payload deliberately returns the full AES block plaintext, including
// PKCS#7 tail padding. Check all padding bytes before passing XML to TinyXML2.
// This is a test-host wire/XML boundary, not a production codec change.
inline bool strict_pkcs7_xml_length(const char* decoded,size_t padded_n,size_t* xml_n) noexcept {
    if(xml_n)*xml_n=0;
    if(!decoded||!xml_n||padded_n==0u||(padded_n&15u)!=0u)return false;
    const unsigned pad=static_cast<unsigned char>(decoded[padded_n-1u]);
    if(pad<1u||pad>16u||pad>padded_n||pad==padded_n)return false;
    for(size_t i=padded_n-pad;i<padded_n;++i)
        if(static_cast<unsigned char>(decoded[i])!=pad)return false;
    *xml_n=padded_n-pad;
    return true;
}
inline size_t find(const char* data,size_t n,const char* token,size_t t,size_t start=0) noexcept {
    if(!data||!token||!t||start>n||n-start<t)return npos;
    for(size_t i=start;i+t<=n;++i){size_t j=0;for(;j<t&&data[i+j]==token[j];++j){}if(j==t)return i;}
    return npos;
}
inline size_t count(const char* data,size_t n,const char* token,size_t t) noexcept {
    if(!data||!token||!t)return 0;
    size_t hits=0,start=0;
    while(start<=n){const size_t at=find(data,n,token,t,start);if(at==npos)break;++hits;start=at+t;}
    return hits;
}
inline bool exact(const char* a,const char* b) noexcept {return a&&b&&strcmp(a,b)==0;}
inline bool only_element(const tinyxml2::XMLNode* container,const tinyxml2::XMLElement* required) noexcept {
    if(!container||!required)return false;
    unsigned found=0;
    for(const auto* n=container->FirstChild();n;n=n->NextSibling()){
        if(n->ToElement()){if(n!=required||++found!=1u)return false;}
        else if(n->ToText()){
            // Allow only formatting whitespace between elements.
            const char* s=n->Value();if(!s)return false;
            for(;*s;++s)if(*s!=' '&&*s!='\t'&&*s!='\r'&&*s!='\n')return false;
        }else return false; // comments / declarations inside the envelope are not emitted
    }
    return found==1u;
}
inline bool parse_owned(const char* data,size_t n,tinyxml2::XMLDocument& doc,const tinyxml2::XMLElement** root) noexcept {
    if(!data||!root||n==0||n>static_cast<size_t>(-1)-1u)return false;
    *root=nullptr;
    // Fail closed on internal NULs (a 0-terminated parse must not ignore a tail).
    if(memchr(data,0,n))return false;
    if(doc.Parse(data,n)!=tinyxml2::XML_SUCCESS)return false;
    const auto* r=doc.RootElement();
    if(!r||!exact(r->Name(),"LSX")||r->NextSibling())return false;
    *root=r;return true;
}
inline bool id_matches(const tinyxml2::XMLElement* a,const char* token,size_t token_n) noexcept {
    static constexpr char prefix[]="<Achievement Id=\"";
    constexpr size_t p=sizeof(prefix)-1u;
    if(!a||!token||token_n<=p||token_n< p+2u)return false;
    if(memcmp(token,prefix,p)!=0||token[token_n-1u]!='\"')return false;
    const char* id=a->Attribute("Id");
    const size_t id_n=token_n-p-1u;
    return id&&strlen(id)==id_n&&memcmp(id,token+p,id_n)==0;
}
inline bool response(const char* data,size_t n,const char* token,size_t token_n) noexcept {
    if(!token||!token_n)return false;
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLElement* root=nullptr;
    if(!parse_owned(data,n,doc,&root))return false;
    const auto* achievement=root->FirstChildElement();
    return achievement&&exact(achievement->Name(),"Achievement")&&
        only_element(root,achievement)&&!achievement->FirstChild()&&
        id_matches(achievement,token,token_n);
}
inline bool event(const char* data,size_t n,const char* token,size_t token_n) noexcept {
    if(!token||!token_n)return false;
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLElement* root=nullptr;
    if(!parse_owned(data,n,doc,&root))return false;
    const auto* e=root->FirstChildElement();
    if(!e||!exact(e->Name(),"Event")||!exact(e->Attribute("sender"),"EbisuSDK")||!only_element(root,e))return false;
    const auto* a=e->FirstChildElement();
    return a&&exact(a->Name(),"Achievement")&&only_element(e,a)&&
        !a->FirstChild()&&id_matches(a,token,token_n);
}
} // namespace achievement_frame_contract
