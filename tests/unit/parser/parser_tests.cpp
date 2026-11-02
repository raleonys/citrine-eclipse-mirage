// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/config/parser.hpp"
#include <iostream>
#include <utility>

using namespace preloader_l::parser;
namespace {
int checks=0;
void require(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
State parsed(std::string_view text){State s;require(parse(text,s),"normal parse success");return s;}
const Node& child(const Node& node,std::string_view key){const auto* n=node.find(key);require(n!=nullptr,"node exists");return *n;}
void expect_error(std::string_view text,ErrorRule rule){
    State s;try{parse(text,s);require(false,"syntax must fail");}
    catch(const ParseError& e){require(e.rule==rule,"error rule");}
}
void syntax(){
    for(const auto* text:{""," \t\r\n\v\f","// eof","/* comment */","// \r\n/*x*/\n"}){
        const auto s=parsed(text);require(s.root.children.empty(),"separators only empty tree");
    }
    auto s=parsed("\"Root\"{\"k\" \"v\"\"next\" \"x\"\"obj\"{}}\"Second\"{}");
    const auto& r=child(s.root,"Root");
    require(child(r,"k").value=="v" && child(r,"next").value=="x","adjacent pairs accepted");
    require(child(r,"obj").children.empty() && s.root.children.size()==2,"adjacent objects");
    for(const auto* separator:{" ","\t","\n","\r","\v","\f","/**/","//comment\n"}){
        const auto raw=std::string("\"R\"{\"k\"")+separator+"\"v\"}";
        const auto out=parsed(raw);require(child(child(out.root,"R"),"k").value=="v","all observed separators");
    }
    expect_error("Root{}",ErrorRule::grammar);
    expect_error("\"key\" \"value\"",ErrorRule::grammar); // root accepts only objects
    expect_error("\"R\"{k \"v\"}",ErrorRule::object_members);
    expect_error("\"R\"{\"k\"\"v\"}",ErrorRule::object_members);
    expect_error("\"R\"{",ErrorRule::object_members);
    expect_error("}",ErrorRule::grammar);
    expect_error("/*unfinished",ErrorRule::grammar);
    expect_error("\"R\"{/*unfinished",ErrorRule::object_members);
    expect_error("\"unterminated",ErrorRule::quoted_character);
    expect_error("\"R\"{\"k\" \"line\nnext\"}",ErrorRule::quoted_character);
    expect_error("\"R\"{\"k\" \"line\rnext\"}",ErrorRule::quoted_character);
    auto empty=parsed("\"\"{\"\" \"\"}");
    require(child(child(empty.root,""),"").value.empty(),"empty quoted names and values accepted");
    auto comment=parsed("/* outer /* inner */\"R\"{}//tail");
    require(child(comment.root,"R").name=="R","block comment ends at first terminator without nesting");
}
void bytes_and_escapes(){
    const auto s=parsed(R"CFG("R"{"quote" "a\"b" "slash" "a\\b" "unknown" "a\nb\tc\q\u1234"})CFG");
    const auto& r=child(s.root,"R");
    require(child(r,"quote").value=="a\"b","escaped quote decodes");
    require(child(r,"slash").value=="a\\b","escaped backslash decodes");
    require(child(r,"unknown").value==R"(a\nb\tc\q\u1234)","unknown C/Unicode escapes retained literally");
    std::string nul="\"R\"{\"k\" \"a";nul.push_back('\0');nul+="b\"}";
    const auto ns=parsed(nul);require(child(child(ns.root,"R"),"k").value==std::string("a\0b",3),"embedded NUL is an ordinary quoted byte");
    std::string invalid="\"R\"{\"k\" \"";invalid.push_back(static_cast<char>(0xff));invalid+="\"}";
    const auto is=parsed(invalid);require(static_cast<unsigned char>(child(child(is.root,"R"),"k").value[0])==0xff,"no UTF-8 validator inserted");
    auto bom=parsed(std::string("\xef\xbb\xbf",3)+"\"R\"{}");require(bom.removed_utf8_bom,"one exact UTF-8 BOM removed");
    expect_error(std::string("\xef\xbb\xbf\xef\xbb\xbf",6)+"\"R\"{}",ErrorRule::grammar);
    expect_error(std::string("\xff\xfe",2)+"\"R\"{}",ErrorRule::grammar);
}
void duplicates_and_partial_state(){
    auto s=parsed("\"R\"{\"k\" \"first\" \"k\" \"last\" \"o\"{\"old\" \"1\"} \"o\"{\"new\" \"2\"}}");
    const auto& r=child(s.root,"R");require(r.children.size()==2 && child(r,"k").value=="last","duplicate scalar last assignment wins");
    const auto& o=child(r,"o");require(!o.find("old") && child(o,"new").value=="2","duplicate object replaces entire subtree");
    require(r.completed_assignments==std::vector<std::string>{"k","k","o","o"},"action chronology keeps duplicate assignments");
    auto scalar=parsed("\"R\"{\"o\"{\"a\" \"1\"} \"o\" \"scalar\"}");
    require(child(child(scalar.root,"R"),"o").children.empty() && child(child(scalar.root,"R"),"o").value=="scalar","scalar replaces object");
    State partial;
    try{parse("\"completed\"{}\n\"broken\"{\"x\" \"good\" \"orphan\"}",partial);require(false,"partial input must fail");}
    catch(const ParseError& e){require(e.rule==ErrorRule::object_members,"partial parse members error");}
    require(partial.root.children.size()==1 && child(partial.root,"completed").name=="completed","completed root objects survive later error");
    require(partial.open_objects.size()==3,"failed ordered alternatives retain two orphan frame actions");
    require(partial.open_objects[0].name=="broken" && child(partial.open_objects[0],"x").value=="good","completed earlier member retained in open object");
    require(partial.open_objects[1].name=="orphan" && partial.open_objects[2].name=="orphan","input rewind does not roll back object-name actions");
    State line;
    try{parse("\r\n//line\n!",line);require(false,"position input must fail");}
    catch(const ParseError& e){require(e.position.byte==9 && e.position.line==3 && e.position.column==1,"LF/CRLF byte line column position");}
    State unfinished;
    try{parse("\"R\"{/*unfinished",unfinished);require(false,"unfinished block must fail");}
    catch(const ParseError& e){require(e.position.byte==16,"direct object-body block attempt consumes to EOF before throw");}
    State slash;
    try{parse("\"R\"{/}",slash);require(false,"lone slash must fail");}
    catch(const ParseError& e){require(e.position.byte==5,"direct failed block opener consumes slash before throw");}
}
std::string nested_objects(std::size_t count) {
    std::string text;
    for(std::size_t i=0;i<count;++i)
        text+=i==0?"\"Config2\"{":"\"node\"{";
    text.append(count,'}');
    return text;
}
void bounded_config_depth(){
    State accepted;
    require(parse(nested_objects(128),accepted),"128 nested objects remain accepted");
    require(accepted.root.children.size()==1,"bounded document produces a root object");
    const Node* leaf=&child(accepted.root,"Config2");
    for(std::size_t i=1;i<128;++i)leaf=&child(*leaf,"node");
    require(leaf->children.empty()&&accepted.open_objects.empty(),"all 128 objects complete without truncation");
    struct DepthOrder final:BinaryIterationOrder {
        std::vector<std::string> order_for(const Node& node,const std::vector<std::string>&)const override {
            std::vector<std::string> names;
            for(const auto& item:node.children)names.push_back(item.name);
            return names; // One child per level: no hash-order ambiguity.
        }
    } order;
    auto converted=to_configuration_node(accepted.root,order);
    const auto* converted_leaf=&converted;
    std::size_t converted_depth=0;
    for(std::size_t i=0;i<128;++i) {
        const auto found=converted_leaf->children.find(i?"node":"Config2");
        if(found==converted_leaf->children.end())break;
        converted_leaf=&found->second;++converted_depth;
    }
    require(converted_depth==128&&converted_leaf->children.empty(),
            "accepted boundary also converts every level and destroys normally");
    State sequential;
    require(parse(nested_objects(128)+nested_objects(128),sequential)&&
            sequential.root.completed_assignments==std::vector<std::string>{"Config2","Config2"},
            "depth is maximum simultaneous nesting, not total object count");
    const auto too_deep=nested_objects(129);
    State rejected;
    try {
        (void)parse(too_deep,rejected);
        require(false,"129 nested objects must be rejected");
    } catch(const ParseError& error) {
        require(error.rule==ErrorRule::grammar,"depth check uses existing grammar error");
        std::size_t cursor=0;
        for(std::size_t i=0;i<129;++i)cursor=too_deep.find('{',cursor)+1;
        require(error.position.byte==cursor-1,"depth check points at the offending brace");
        require(error.position.line==1&&error.position.column==cursor,"depth check column");
    }
    require(rejected.root.children.empty()&&rejected.open_objects.empty(),
            "depth rejection occurs before any parser action");
    State giant;
    try {
        (void)parse(nested_objects(22000),giant);
        require(false,"very deep document must not enter recursive PEGTL");
    } catch(const ParseError& error) {
        require(error.rule==ErrorRule::grammar&&error.position.byte==905,
                "deep input rejects at the same 129th brace before recursive actions");
    }
    const auto quoted=std::string("\"Config2\"{\"value\" \"")+
        std::string(1000,'{')+"\\\""+std::string(1000,'}')+"\"}";
    State quoted_state;
    require(parse(quoted,quoted_state),"literal and escaped quoted braces are ignored");
    const auto commented=std::string("/*")+std::string(1000,'{')+
        "*/ \"Config2\"{ //"+std::string(1000,'{')+"\n\"key\" \"ok\"}";
    State commented_state;
    require(parse(commented,commented_state),"commented braces are ignored");
    State bom;
    try {
        (void)parse(std::string("\xef\xbb\xbf",3)+"\n"+too_deep,bom);
        require(false,"over-depth document with BOM must reject");
    } catch(const ParseError& error) {
        require(error.position.byte==906&&error.position.line==2&&error.position.column==906,
                "one BOM is excluded from byte offsets and LF resets the column");
    }
    State mixed_lines;
    const std::string prefix="\r\n/*x\r\n*/ //end\r\n";
    try{(void)parse(prefix+too_deep,mixed_lines);require(false,"mixed-line depth must reject");}
    catch(const ParseError& error){
        require(error.position.byte==prefix.size()+905&&error.position.line==4&&error.position.column==906,
                "depth position counts LF consistently through CRLF and comments");
    }
    std::string decorated="\"Config2\"{/* \" { } */";
    for(unsigned i=1;i<128;++i)decorated+="// { } \"\r\n\"node\"/**/{";
    decorated+=R"VDF("value" "brace{quote\"slash\\unknown\q}")VDF";
    decorated.append(128,'}');
    State decorated_state;
    require(parse(decorated,decorated_state),"comments and escaped quote/backslash bytes do not inflate boundary depth");
    const Node* decorated_leaf=&child(decorated_state.root,"Config2");
    for(unsigned i=1;i<128;++i)decorated_leaf=&child(*decorated_leaf,"node");
    require(child(*decorated_leaf,"value").value==R"VDF(brace{quote"slash\unknown\q})VDF",
            "preflight preserves PEGTL byte decoding at the accepted boundary");
    State retained;
    retained.root.name="root";retained.root.completed_assignments={"old"};
    retained.root.children.push_back({"old","value",{}, {}});
    retained.open_objects.push_back({"orphan","open",{}, {}});
    retained.accumulator="acc";retained.key="key";retained.value="value";
    try{(void)parse(std::string("\xef\xbb\xbf",3)+too_deep,retained);require(false,"retained-State depth must reject");}
    catch(const ParseError& error){require(error.position.byte==905,"retained-State rejection position");}
    require(retained.root.name=="root"&&retained.root.children.size()==1&&
            retained.root.children[0].name=="old"&&retained.root.children[0].value=="value"&&
            retained.root.completed_assignments==std::vector<std::string>{"old"},
            "preflight does not clear or append existing AST/assignment state");
    require(retained.open_objects.size()==1&&retained.open_objects[0].name=="orphan"&&
            retained.accumulator=="acc"&&retained.key=="key"&&retained.value=="value"&&retained.removed_utf8_bom,
            "rejection preserves action fields while retaining existing BOM bookkeeping");
    std::string nul_prefix="\"literal\"{\"value\" \"a";nul_prefix.push_back('\0');nul_prefix+="b\"}";
    State after_nul;
    try{(void)parse(nul_prefix+too_deep,after_nul);require(false,"depth scan must continue after quoted NUL");}
    catch(const ParseError& error){require(error.position.byte==nul_prefix.size()+905,"depth scan uses the full byte view");}
    require(after_nul.root.children.empty()&&after_nul.open_objects.empty(),
            "whole-document depth preflight precedes earlier valid object actions");
}
class SyntheticOrder final:public BinaryIterationOrder {
public:
    bool invalid=false;
    std::vector<std::string> order_for(const Node& node,const std::vector<std::string>&) const override {
        if(invalid)return {};
        // MOCK ONLY: deliberately reverse syntax order, not a binary layout claim.
        std::vector<std::string> result;
        for(auto i=node.children.rbegin();i!=node.children.rend();++i)result.push_back(i->name);
        return result;
    }
};
void conversion_contract(){
    const auto s=parsed("\"Config2\"{\"Content\"{\"z\"{}\"a\"{}}}");
    SyntheticOrder order;auto config=to_configuration_node(s.root,order);
    const auto& content=config.child("Config2").child("Content");
    require(content.iteration_order==std::vector<std::string>{"a","z"},"external binary order mandatory, independent of AST order");
    require(s.root.children[0].children[0].children[0].name=="z","logical source/action order retained separately");
    order.invalid=true;
    try{(void)to_configuration_node(s.root,order);require(false,"bad external order must fail");}
    catch(const std::logic_error&){require(true,"incomplete provider order rejected");}
}
}
int main(){try{syntax();bytes_and_escapes();duplicates_and_partial_state();bounded_config_depth();conversion_contract();
 std::cout<<"PASS: "<<checks<<" synthetic parser assertions. No real CFG, original DLL/EXE, file I/O or environment actions.\n";return 0;}
 catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
