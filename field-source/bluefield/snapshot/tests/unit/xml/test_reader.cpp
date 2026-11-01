#include "src/xml/reader.hpp"
#include "third_party/tinyxml2/tinyxml2.h"
#include <iostream>
#include <stdexcept>

namespace reader=preloader_l::xml_reader;
namespace msg=preloader_l::messages;
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
    static_assert(::TINYXML2_MAX_ELEMENT_DEPTH==500,"pristine upstream depth must remain unchanged");
    for(unsigned depth=1;depth<=105;++depth) {
        std::string input;for(unsigned i=1;i<depth;++i)input+="<R>";
        input+="<R/>";for(unsigned i=1;i<depth;++i)input+="</R>";
        const auto parsed=backend.parse_c_string(input);
        check(parsed.returned_code==(depth>=100?18:0),"self-closing element does not enter source ParseDeep");
        if(depth>=100)check(parsed.has_error&&!parsed.document->first_element()&&parsed.document->error_name().find("ErrorID=18")!=std::string_view::npos&&parsed.document->error_name().find("Line number=1")!=std::string_view::npos,"native parse-time depth error hides document and preserves full ErrorStr");
    }
    for(unsigned depth=1;depth<=105;++depth) {
        std::string input;for(unsigned i=0;i<depth;++i)input+="<R>";
        for(unsigned i=0;i<depth;++i)input+="</R>";
        const auto parsed=backend.parse_c_string(input);
        // XMLNode::ParseDeep counts the document level as well. Original
        // PushDepth increments then compares equality with 100.
        check(parsed.returned_code==(depth>=99?18:0),"evidence-backed depth boundary");
    }
}
}
int main(){try{reading_and_ownership();errors_and_depth();
    std::cout<<"PASS: "<<checks<<" actual XML reader checks; selected upstream compatibility source, original target not executed\n";
    return 0;}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
