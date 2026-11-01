// SPDX-License-Identifier: GPL-3.0-only
#include "src/configuration/parser.hpp"
#include "tests/support/configuration/packet_parser.hpp"
#include "tests/support/configuration/parser_snapshot.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
namespace current=preloader_l::parser;
namespace packet=preloader_l::test_packet_parser;
namespace bytes=preloader_l::tests::parser_snapshot;
static std::size_t cases=0,checks=0;
static void check(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
static void field(std::string& out,std::string_view value){out+=std::to_string(value.size());out+=':';out.append(value);}
template<class Node>static void snapshot_node(std::string& out,const Node& n){
    field(out,n.name);field(out,n.value);out+='[';
    for(auto& a:n.completed_assignments)field(out,a);out+="]{";
    for(auto& c:n.children)snapshot_node(out,c);out+='}';
}
template<class Error,class State,class Parse>static std::string evaluate(std::string_view input,State& s,Parse parse){
    std::string out;
    try{out=parse(input,s)?"true;":"false;";}
    catch(const Error& e){out="parse-error;"+std::to_string(static_cast<int>(e.rule))+';'+std::to_string(e.position.byte)+';'+std::to_string(e.position.line)+';'+std::to_string(e.position.column)+';';field(out,e.what());}
    catch(const std::exception& e){out="other-error;";field(out,e.what());}
    out+=s.removed_utf8_bom?"bom;":"raw;";snapshot_node(out,s.root);out+='[';
    for(auto& f:s.open_objects)snapshot_node(out,f);out+=']';
    field(out,s.accumulator);field(out,s.key);field(out,s.value);return out;
}
static void compare(std::string_view input,current::State& c,packet::State& p){
    ++cases;const auto actual=evaluate<current::ParseError>(input,c,current::parse);
    const auto expected=evaluate<packet::ParseError>(input,p,packet::parse);
    if(actual!=expected)throw std::runtime_error("source mismatch case="+std::to_string(cases)+" input_hex="+bytes::hex(input)+" expected_hex="+bytes::hex(expected)+" actual_hex="+bytes::hex(actual));
    ++checks;
}
static void compare(std::string_view input){current::State c;packet::State p;compare(input,c,p);}
int main(){try{
    auto root=std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    std::ifstream file(root/"tests/fixtures/config2_fix09_corpus.tsv",std::ios::binary);check(bool(file),"corpus inputs missing");
    std::string line;std::size_t frozen=0;
    while(std::getline(file,line)){
        auto tab=line.find('\t');check(tab!=std::string::npos,"invalid corpus input record");
        // Only input bytes are consumed. The historical expected-output column
        // is deliberately never used by this live source oracle comparison.
        compare(bytes::unhex(std::string_view(line).substr(0,tab)));++frozen;
    }
    check(file.eof()&&frozen==8766,"incomplete corpus inputs");
    const std::array<char,12> alphabet{'"','\\','{','}','/','*',' ','\n','\r','a','\0',char(0xff)};
    // Exhaustive short byte strings, plus two grammar contexts. This bound is
    // only the finite test generator, not a runtime Config2 quota.
    for(unsigned length=0;length<=4;++length){
        std::size_t total=1;for(unsigned i=0;i<length;++i)total*=alphabet.size();
        for(std::size_t seed=0;seed<total;++seed){
            auto n=seed;std::string payload;for(unsigned i=0;i<length;++i){payload+=alphabet[n%alphabet.size()];n/=alphabet.size();}
            compare(payload);compare("\"Root\"{"+payload+"}");compare("\"Root\"{\"key\" "+payload+"}");
        }
    }
    const std::array<std::string,12> fragments{"","\"R\"{}","\"R\"{\"a\" \"1\" \"a\"{}}","\"R\"{\"a\"{} \"a\" \"2\"}","\"R\"{\"x\"/","\"R\"{\"x\"/*","\"R\"{\"x\" \"y","\"unfinished","//\r","/*bad","\xef\xbb\xbf\"R\"{}",std::string("\"R\"{\"k\" \"\0\"}",11)};
    for(auto& first:fragments)for(auto& next:fragments){
        compare(first+next);compare(std::string("\xef\xbb\xbf")+first+next);
        current::State c;packet::State p;compare(first,c,p);compare(next,c,p);
    }
    std::cout<<"PASS live packet Config2 oracle cases="<<cases<<" checks="<<checks<<"; historical output column unused; original DLL NOT_RUN\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
