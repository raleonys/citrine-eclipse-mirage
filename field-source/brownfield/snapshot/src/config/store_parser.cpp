#include "store_internal.hpp"
#include "../../third_party/tinyxml2/tinyxml2.h"
#include <cstring>
#include <limits>

namespace preloader_l { namespace config { namespace store_entitlement_internal {
namespace {
using tinyxml2::XMLElement;
using tinyxml2::XMLNode;

bool eq(const char* a,const char* b) noexcept { return a&&b&&std::strcmp(a,b)==0; }

// TinyXML2 expands character references, including some invalid Unicode scalar
// references. Preserve the release's strict UTF-8/XML-1.0-character boundary.
// This is validation of decoded bytes, not a second XML parser or entity codec.
bool valid_semantic_xml_chars(const char* s) noexcept {
    if (!s) return true;
    const size_t n=std::strlen(s);
    for (size_t i=0;i<n;) {
        const unsigned char first=static_cast<unsigned char>(s[i]);
        uint32_t cp=0;size_t k=0;
        if (first<0x80u) { cp=first;k=1; }
        else if (first>=0xC2u&&first<=0xDFu) { cp=first&0x1Fu;k=2; }
        else if (first>=0xE0u&&first<=0xEFu) { cp=first&0x0Fu;k=3; }
        else if (first>=0xF0u&&first<=0xF4u) { cp=first&0x07u;k=4; }
        else return false;
        if (k>n-i) return false;
        if (k>=2) {
            const unsigned char b1=static_cast<unsigned char>(s[i+1]);
            if ((b1&0xC0u)!=0x80u ||
                (first==0xE0u&&b1<0xA0u) ||
                (first==0xEDu&&b1>0x9Fu) ||
                (first==0xF0u&&b1<0x90u) ||
                (first==0xF4u&&b1>0x8Fu)) return false;
        }
        for(size_t j=1;j<k;++j) {
            const unsigned char c=static_cast<unsigned char>(s[i+j]);
            if ((c&0xC0u)!=0x80u) return false;
            cp=(cp<<6)|(c&0x3Fu);
        }
        if (!(cp==9u||cp==10u||cp==13u||
             (cp>=0x20u&&cp<=0xD7FFu)||
             (cp>=0xE000u&&cp<=0xFFFDu)||
             (cp>=0x10000u&&cp<=0x10FFFFu))) return false;
        i+=k;
    }
    return true;
}

ParseCode semantic_copy(StoreState& state,const char* value,Span* result) noexcept {
    if (!result) return ParseCode::invalid;
    *result=Span{0,0,false};
    if (!value) return ParseCode::ok;
    if (!valid_semantic_xml_chars(value)) return ParseCode::invalid;
    const size_t n=std::strlen(value);
    size_t offset=0;
    if (!arena_raw(state,value,n,&offset)) return ParseCode::resource;
    *result=Span{offset,n,true};
    return ParseCode::ok;
}

ParseCode attr(StoreState& state,const XMLElement* element,const char* name,Span* result) noexcept {
    return semantic_copy(state,element->Attribute(name),result);
}
ParseCode numeric_attr(StoreState& state,const XMLElement* element,const char* name,uint64_t* result) noexcept {
    Span value{};
    auto code=attr(state,element,name,&value);
    if (code!=ParseCode::ok) return code;
    return parse_u64_span(state,value,result);
}
ParseCode text(StoreState& state,const XMLElement* element,Span* result) noexcept {
    // TinyXML2 GetText returns the first text node (skipping leading comments),
    // not recursively flattened or source-encoded XML content.
    return semantic_copy(state,element->GetText(),result);
}
ParseCode append_feature(Span** values,size_t* count,size_t* capacity,
                         Span value,FeatureRange* range) noexcept {
    if (!values||!count||!capacity||!range||*count>UINT32_MAX||range->count==UINT32_MAX)
        return ParseCode::resource;
    if (!reserve_array(values,capacity,*count,*count+1u)) return ParseCode::resource;
    if (!range->count) range->start=static_cast<uint32_t>(*count);
    (*values)[(*count)++]=value;
    ++range->count;
    return ParseCode::ok;
}
ParseCode parse_entitlement(StoreState& state,const XMLElement* e,Record* record) noexcept {
    if (!e||!record) return ParseCode::invalid;
    *record=Record{};
    ParseCode code=numeric_attr(state,e,"NumericId",&record->numeric_id);
    if (code!=ParseCode::ok) return code;
    code=numeric_attr(state,e,"OfferId",&record->offer_id);
    if (code!=ParseCode::ok) return code;
    code=numeric_attr(state,e,"ReleaseMs",&record->release_ms);
    if (code!=ParseCode::ok) return code;
    code=numeric_attr(state,e,"Flags",&record->flags);
    if (code!=ParseCode::ok) return code;
    uint64_t category=0;
    code=numeric_attr(state,e,"Category",&category);
    if (code!=ParseCode::ok) return code;
    record->category=static_cast<uint32_t>(category);

    static constexpr const char* names[]={"Tag","Icon","Title","ImgThumb","ImgCropped","ImgHero","Vendor","Key","Hash"};
    static constexpr uint8_t indices[]={0,1,2,4,5,6,7,8,9};
    for (size_t i=0;i<9;++i) {
        code=attr(state,e,names[i],&record->text[indices[i]]);
        if (code!=ParseCode::ok) return code;
    }
    bool seen_description=false;
    for (const XMLElement* child=e->FirstChildElement();child;child=child->NextSiblingElement()) {
        if (eq(child->Name(),"Description") && !seen_description) {
            Span decoded{};
            code=text(state,child,&decoded);
            if (code!=ParseCode::ok) return code;
            if (decoded.used) record->text[3]=decoded;
            seen_description=true;
        } else if (eq(child->Name(),"FeatureDesc") || eq(child->Name(),"FeatureFlag")) {
            const bool desc=eq(child->Name(),"FeatureDesc");
            Span decoded{};
            code=text(state,child,&decoded);
            if (code!=ParseCode::ok) return code;
            if (decoded.used) {
                code=desc ? append_feature(&state.feature_descs,&state.feature_desc_count,
                            &state.feature_desc_capacity,decoded,&record->feature_desc)
                          : append_feature(&state.feature_flags,&state.feature_flag_count,
                            &state.feature_flag_capacity,decoded,&record->feature_flag);
                if (code!=ParseCode::ok) return code;
            }
        }
    }
    Span encoded{};
    code=attr(state,e,"ProtoBytes",&encoded);
    if (code!=ParseCode::ok) return code;
    code=decode_quartets(state,encoded,&record->bytes[0]);
    if (code!=ParseCode::ok) return code;
    code=attr(state,e,"Field2",&encoded);
    if (code!=ParseCode::ok) return code;
    code=decode_quartets(state,encoded,&record->bytes[1]);
    if (code!=ParseCode::ok) return code;
    code=attr(state,e,"Pad",&encoded);
    if (code!=ParseCode::ok) return code;
    return decode_hex(state,encoded,&record->bytes[2]);
}
ParseCode parse_store(StoreState& state,const XMLElement* store) noexcept {
    state.present=true;
    Span value{};
    ParseCode code=attr(state,store,"PreambleB64",&value);
    if (code!=ParseCode::ok) return code;
    code=decode_quartets(state,value,&state.preamble);
    if (code!=ParseCode::ok) return code;
    code=attr(state,store,"TrailerB64",&value);
    if (code!=ParseCode::ok) return code;
    code=decode_quartets(state,value,&state.trailer);
    if (code!=ParseCode::ok) return code;
    for (const XMLElement* e=store->FirstChildElement();e;e=e->NextSiblingElement()) {
        if (!eq(e->Name(),"Entitlement")) continue;
        if (state.record_count==static_cast<size_t>(-1) ||
            !reserve_array(&state.records,&state.record_capacity,state.record_count,state.record_count+1u))
            return ParseCode::resource;
        code=parse_entitlement(state,e,&state.records[state.record_count]);
        if (code!=ParseCode::ok) return code;
        ++state.record_count;
    }
    return ParseCode::ok;
}
} // namespace

ParseCode parse_document(StoreState& state,const char* s,size_t n) noexcept {
    if ((!s&&n) || (s&&std::memchr(s,0,n))) return ParseCode::invalid;
    try {
        tinyxml2::XMLDocument doc(true,tinyxml2::PRESERVE_WHITESPACE);
        const auto status=doc.Parse(s?s:"",n);
        if (status==tinyxml2::XML_ELEMENT_DEPTH_EXCEEDED) return ParseCode::resource;
        if (status!=tinyxml2::XML_SUCCESS) return ParseCode::invalid;
        const XMLElement* root=doc.RootElement();
        if (!root || root->NextSiblingElement()) return ParseCode::invalid;
        // AE00: Root named Store is not selected; only its first direct Store child.
        for (const XMLElement* child=root->FirstChildElement();child;child=child->NextSiblingElement())
            if (eq(child->Name(),"Store")) return parse_store(state,child);
        return ParseCode::ok;
    } catch (...) { return ParseCode::resource; }
}
} } }
