// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/xml/reader.hpp"
#include <iostream>
#include <stdexcept>

namespace reader=citrine_eclipse_mirage::xml_reader_bridge;
namespace msg=citrine_eclipse_mirage::messages;
namespace {
unsigned checks=0;
void check(bool value,const char* reason){++checks;if(!value)throw std::runtime_error(reason);}
const msg::XmlNode& root(const msg::ParseResult& parsed) {
    check(!parsed.has_error&&parsed.returned_code==0,"parse success");
    const auto* node=parsed.document->first_element();check(node!=nullptr,"root present");return *node;
}
void reading_and_ownership() {
    reader::Backend backend;
    std::string input="\xef\xbb\xbf<?xml version=\"1.0\"?><!--outside--><LSX z=\"\" a=\"&quot;&amp;&apos;&lt;&gt;\" b=\"&#65;&#x1f600;\"><!--skip--><Child> first\r\nsecond\rthird </Child><Other/></LSX><Tail/>";
    auto parsed=backend.parse_c_string(input);input.assign(input.size(),'X');
    const auto& element=root(parsed);check(element.name()=="LSX","input copied/owned");
    check(element.attribute("z")=="","present empty attribute");
    check(!element.attribute("Z"),"attribute case sensitive");
    check(element.attribute("a")=="\"&'<>","five named entities");
    check(element.attribute("b")=="A\xf0\x9f\x98\x80","decimal/hex codepoint to UTF-8");
    const auto attrs=backend.attributes_in_order(element);
    check(attrs.size()==3&&attrs[0].first=="z"&&attrs[1].first=="a"&&attrs[2].first=="b","attribute order");
    const auto* child=element.first_element();check(child&&child->name()=="Child","comments skipped by element traversal");
    check(element.first_element()==child,"stable wrapper identity");
    check(std::string_view(backend.text_003860(*child))==" first\nsecond\nthird ","preserved text/newline normalization");
    const auto* other=child->next_element();check(other&&other->name()=="Other","element sibling");
    check(!other->first_element()&&!other->next_element(),"empty element traversal");
    check(backend.text_003860(*other)==nullptr,"empty GetText is null");
    check(backend.text_003860(element)==nullptr,"nested first noncomment is not text");
    check(element.next_element()&&element.next_element()->name()=="Tail","multiple root elements retained");
    check(parsed.document->error_name().empty(),"success has empty ErrorStr diagnostic");
    auto text=backend.parse_c_string("<R><!--a--><!--b--><![CDATA[A&B<raw>]]><C/>tail</R>");
    check(std::string_view(backend.text_003860(root(text)))=="A&B<raw>","GetText skips comments and returns one text node");
    auto mixed=backend.parse_c_string("<R>before<C/>after</R>");
    check(std::string_view(backend.text_003860(root(mixed)))=="before","GetText does not concatenate");
    auto normalization=backend.parse_c_string(" \t\n\xef\xbb\xbf<R>a\n\rb</R>");
    check(std::string_view(backend.text_003860(root(normalization)))=="a\nb","BOM after leading whitespace and LFCR normalization");
    auto emptytext=backend.parse_c_string("<R><![CDATA[]]></R>");
    const auto* cdata=backend.text_003860(root(emptytext));check(cdata&&std::string_view(cdata).empty(),"CDATA empty differs from absent text");
    std::string nul="<R/>";nul.push_back('\0');nul+="<malformed";
    check(root(backend.parse_c_string(nul)).name()=="R","C-string first NUL stops parsing");
    const std::string sliced="<R/>junk";
    check(root(backend.parse_c_string(std::string_view(sliced).substr(0,4))).name()=="R","nonterminated view slice copied safely");
    const auto* retained=child;
    for(unsigned i=0;i<20;++i){auto temporary=backend.parse_c_string("<Different/>");check(root(temporary).name()=="Different","independent docs");}
    check(retained->name()=="Child","original doc lives across later parses");
}
void errors_and_depth() {
    reader::Backend backend;
    struct Case{const char* input;int code;const char* name;};
    const Case cases[]={
        {"",13,"XML_ERROR_EMPTY_DOCUMENT"},{" \r\n\t",13,"XML_ERROR_EMPTY_DOCUMENT"},
        {"abcdef012345",8,"XML_ERROR_PARSING_TEXT"},
        {"<R a=>",7,"XML_ERROR_PARSING_ATTRIBUTE"},
        {"<R a='1' a='2'/>",7,"XML_ERROR_PARSING_ATTRIBUTE"},
        {"<R><C></R>",14,"XML_ERROR_MISMATCHED_ELEMENT"},
        {"<R><![CDATA[unclosed</R>",9,"XML_ERROR_PARSING_CDATA"},
        {"<R><!--unclosed</R>",10,"XML_ERROR_PARSING_COMMENT"}
    };
    for(const auto& test:cases) {
        const auto parsed=backend.parse_c_string(test.input);
        check(parsed.has_error&&parsed.returned_code==test.code,"error code");
        check(parsed.document->error_name().find(test.name)!=std::string_view::npos,"full ErrorStr name");
        check(parsed.document->error_name().find("ErrorID=")!=std::string_view::npos,"full diagnostic id");
        check(parsed.document->error_name().find("Line number=")!=std::string_view::npos,"full diagnostic line");
        check(!parsed.document->first_element(),"failed parse children cleaned");
    }
    for(unsigned depth=1;depth<=505;++depth) {
        std::string input;for(unsigned i=0;i<depth;++i)input+="<R>";
        for(unsigned i=0;i<depth;++i)input+="</R>";
        const auto parsed=backend.parse_c_string(input);
        // Supplied unmodified vendor uses 500; ParseDeep also counts the document.
        // This intentionally replaces the older RE copy's 100-limit profile.
        check(parsed.returned_code==(depth>=499?18:0),"established project depth boundary");
    }
}
void numeric_character_reference_bounds() {
    // C33 security applicability: exercise the actual production XML
    // reader, not an independent or synthetic numeric decoder.
    reader::Backend backend;
    const auto verify=[&](const std::string& reference,std::string_view expected) {
        const std::string input="<R number='"+reference+"'>"+reference+"</R>";
        auto parsed=backend.parse_c_string(input);
        const auto& element=root(parsed);
        const auto value=element.attribute("number");
        check(value&&*value==expected,"numeric-reference attribute must not wrap");
        const char* text=backend.text_003860(element);
        check(text&&std::string_view(text)==expected,
              "numeric-reference text must match attribute conversion");
    };
    constexpr std::string_view highest("\xF4\x8F\xBF\xBF",4);
    constexpr std::string_view below_highest("\xF4\x8F\xBF\xBE",4);
    verify("&#x10FFFF;",highest);
    verify("&#1114111;",highest);
    verify("&#x10FFFE;",below_highest);
    verify("&#x00000010FFFF;",highest);
    // The corrected project policy leaves invalid numeric references literal.
    // Do not silently change that policy.
    for(const char* raw:{"&#x110000;","&#1114112;","&#xFFFFFFFF;",
                          "&#x100000000;","&#4294967295;","&#4294967296;",
                          "&#x123456789ABCDEF;","&#xG123;","&#12E3;"}) {
        const std::string reference=raw;
        verify(reference,reference);
    }
    for(const auto length:{32U,64U,256U}) {
        const std::string reference="&#x"+std::string(length,'F')+";";
        verify(reference,reference);
    }
}
void bounded_input_and_wrapper_rehash() {
    reader::Backend backend;
    check(backend.parse_c_string(std::string_view{}).returned_code==13,"null empty view is an empty document");
    const std::string zero("\0<R/>",5);
    check(backend.parse_c_string(zero).returned_code==13,"leading NUL remains an empty document");
    std::string source="<R><C v='retained'>payload</C>";
    for(unsigned i=0;i<1024;++i)source+="<C/>";
    source+="</R>trailing-not-in-view";
    const auto length=source.find("trailing-not-in-view");
    auto parsed=backend.parse_c_string(std::string_view(source.data(),length));
    source.assign(source.size(),'X'); // The vendor, not the caller, owns parse text.
    const auto* r=&root(parsed);const auto* first=r->first_element();
    check(first&&first->attribute("v")=="retained","wrapper retains document-owned attribute bytes");
    unsigned count=0;
    for(auto* node=first;node;node=node->next_element())++count;
    check(count==1025,"all wrappers survive repeated container growth");
    check(r->first_element()==first&&first->attribute("v")=="retained","root and child identities survive wrapper-map rehash");
    check(std::string_view(backend.text_003860(*first))=="payload","text survives caller overwrite and wrapper-map rehash");
}
void reference_values_and_contexts() {
    reader::Backend backend;
    const auto verify=[&](std::string_view reference,std::string_view expected) {
        const auto input="<R a='prefix"+std::string(reference)+"suffix'>prefix"+std::string(reference)+"suffix</R>";
        auto parsed=backend.parse_c_string(input);
        if(parsed.has_error)throw std::runtime_error("reference fixture parse: "+std::string(reference)+" "+std::string(parsed.document->error_name()));
        const auto& node=root(parsed);
        const auto value="prefix"+std::string(expected)+"suffix";
        check(node.attribute("a")==value,"attribute reference preserves complete prefix/suffix");
        const auto* text=backend.text_003860(node);
        check(text&&std::string_view(text)==value,"text reference preserves complete prefix/suffix");
        const auto attrs=backend.attributes_in_order(node);
        check(attrs.size()==1&&attrs[0].second==value,"ordered attributes use the same reference policy");
    };
    for(const auto& [reference,expected]:std::vector<std::pair<std::string,std::string>>{
        {"&#9;","\t"},{"&#10;","\n"},{"&#13;","\r"},{"&#32;"," "},
        {"&#x7F;","\x7f"},{"&#x80;","\xc2\x80"},{"&#x7FF;","\xdf\xbf"},
        {"&#x800;","\xe0\xa0\x80"},{"&#xD7FF;","\xed\x9f\xbf"},
        {"&#xE000;","\xee\x80\x80"},{"&#xFFFD;","\xef\xbf\xbd"},
        {"&#x10000;","\xf0\x90\x80\x80"},{"&#1114111;","\xf4\x8f\xbf\xbf"},
        {"&amp;&#65;&x1f600;","&A&x1f600;"},{"&amp;&unknown;","&&unknown;"},
        {"&amp;#x100000000;","&#x100000000;"},{"&#38;#65;","&#65;"},
        {"&amp;amp;","&amp;"},{"&quot;&amp;&apos;&lt;&gt;","\"&'<>"}})
        verify(reference,expected);
    for(const auto* reference:{"&#0;","&#x0;","&#5;","&#6;","&#31;","&#xD800;",
            "&#xDFFF;","&#65534;","&#65535;","&#x110000;","&#x100000000;",
            "&#4294967296;","&#18446744073709551616;","&#x10000000000000000;",
            "&#;","&#x;","&#X41;","&#xGG;","&#-1;","&#43","&unknown;","&AMP;","&"})
        verify(reference,reference);
    verify("&#"+std::string(20000,'0')+"65;","A");
    verify("&#x"+std::string(20000,'F')+";","&#x"+std::string(20000,'F')+";");
    auto contexts=backend.parse_c_string("<?xml version='1.0'?><!DOCTYPE R [<!ENTITY named '&#65;'>]><R a='&named;'><!--&amp;&#0;--><![CDATA[&amp;&#65;&#0;]]><C/></R>");
    check(!contexts.has_error,"DTD context fixture parses");
    const auto& node=root(contexts);
    check(node.attribute("a")=="&named;","DTD entities remain unexpanded and literal");
    check(std::string_view(backend.text_003860(node))=="&amp;&#65;&#0;","CDATA bypasses reference decoding after comments");
    auto pi=backend.parse_c_string("<R><?pi value='&#65;'?>&#65;</R>");
    check(pi.has_error&&!pi.document->first_element(),"nested PI retains the pinned parser's rejection");
}
void decoded_value_ownership() {
    reader::Backend backend;std::string input="<R>";
    for(unsigned i=0;i<2048;++i)input+="<C v='value&amp;owned'>text&amp;owned</C>";
    input+="</R>";auto parsed=backend.parse_c_string(input);input.assign(input.size(),'X');
    const auto* first=root(parsed).first_element();check(first!=nullptr,"decoded ownership first child");
    const auto value=first->attribute("v");check(value=="value&owned","decoded attribute retained");
    const auto* text=backend.text_003860(*first);
    for(auto* node=first;node;node=node->next_element()) {
        check(node->attribute("v")=="value&owned","decode every attribute across cache rehash");
        check(std::string_view(backend.text_003860(*node))=="text&owned","decode every text across cache rehash");
    }
    check(value&&value->data()==first->attribute("v")->data()&&*value=="value&owned","borrowed decoded attribute survives cache growth");
    check(text==backend.text_003860(*first)&&std::string_view(text)=="text&owned","decoded text pointer survives cache growth");
    for(unsigned i=0;i<20;++i)(void)backend.parse_c_string("<Other a='&amp;'/>");
    check(*value=="value&owned"&&std::string_view(text)=="text&owned","decoded values remain owned by their original document");
}
}
int main(){try{reading_and_ownership();errors_and_depth();numeric_character_reference_bounds();bounded_input_and_wrapper_rehash();reference_values_and_contexts();decoded_value_ownership();
    std::cout<<"PASS: "<<checks<<" actual XML reader checks; selected upstream compatibility source, original target not executed\n";
    return 0;}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
