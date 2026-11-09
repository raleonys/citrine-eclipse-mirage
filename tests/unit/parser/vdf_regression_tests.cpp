// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/config/parser.hpp"
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

// Golden observations come from the previous RE-derived parser,
// not from another product line. The old implementation stays outside the DLL
// and this source package. See docs/VDF_PARSER.md for baseline hashes and scope.
namespace {
using namespace citrine_eclipse_mirage::parser;
struct Corpus {
    std::uint64_t signature=14695981039346656037ULL;
    std::uint64_t cases=0,accepted=0,rejected=0;
    void byte(unsigned char b) {signature^=b;signature*=1099511628211ULL;}
    void number(std::uint64_t n) {for(unsigned i=0;i<8;++i){byte(n&255);n>>=8;}}
    void text(std::string_view value) {number(value.size());for(unsigned char b:value)byte(b);}
    void node(const Node& value) {
        text(value.name);text(value.value);number(value.children.size());
        for(const auto& child:value.children)node(child);
        number(value.completed_assignments.size());
        for(const auto& assignment:value.completed_assignments)text(assignment);
    }
    void observe(std::string_view input) {
        ++cases;text(input);State state;
        try {byte(parse(input,state)?1:0);++accepted;}
        catch(const ParseError& error) {
            ++rejected;byte(2);number(static_cast<unsigned>(error.rule));
            number(error.position.byte);number(error.position.line);number(error.position.column);
        }
        node(state.root);number(state.open_objects.size());
        for(const auto& frame:state.open_objects)node(frame);
        text(state.accumulator);text(state.key);text(state.value);byte(state.removed_utf8_bom);
    }
};
void short_inputs(Corpus& corpus,std::string& input,unsigned remaining) {
    corpus.observe(input);
    if(!remaining)return;
    for(char ch:std::array<char,9>{'"','{','}','/','*',' ','\\','\n','a'}) {
        input.push_back(ch);short_inputs(corpus,input,remaining-1);input.pop_back();
    }
}
void all_byte_contexts(Corpus& corpus) {
    for(unsigned value=0;value<256;++value) {
        const std::string byte(1,static_cast<char>(value));
        corpus.observe("\""+byte+"\"{}");
        corpus.observe("\"R\"{\"k\" \""+byte+"\"}");
        corpus.observe("\"R\"{\"k\" \"\\"+byte+"\"}");
        corpus.observe("//"+byte+"\n\"R\"{}");
        corpus.observe("/*"+byte+"*/\"R\"{}");
    }
}
void document_mutations(Corpus& corpus) {
    const std::array<std::string,10> documents={
        "", "\"R\"{}", "\"R\"{\"a\" \"1\"\"b\" \"2\"}",
        "\"R\"{\"k\" \"old\" \"k\"{\"x\" \"new\"}}",
        "\"R\"{\"o\"{\"x\" \"1\"} \"o\" \"scalar\"}",
        "/*intro*/\r\n\"R\"{\"k\"/**/\"v\"}//tail",
        "\"done\"{}\n\"broken\"{\"x\" \"good\" \"orphan\"}",
        R"VDF("R"{"k" "a\"b\\c\q\n"})VDF",
        std::string("\xef\xbb\xbf",3)+"\"Config2\"{\"User\"{\"Username\" \"sample\"}}",
        "\"R\"{/*unfinished"
    };
    const std::array<char,12> replacements={'"','{','}','/','*',' ','\t','\r','\n','\\','\0',static_cast<char>(0xff)};
    for(const auto& document:documents) {
        corpus.observe(document);
        for(std::size_t length=0;length<=document.size();++length)
            corpus.observe(std::string_view(document).substr(0,length));
        for(std::size_t position=0;position<document.size();++position) {
            auto removed=document;removed.erase(position,1);corpus.observe(removed);
            for(char ch:replacements) {
                auto changed=document;changed[position]=ch;corpus.observe(changed);
                auto inserted=document;inserted.insert(position,1,ch);corpus.observe(inserted);
            }
        }
    }
}
void generated_documents(Corpus& corpus) {
    for(unsigned i=0;i<512;++i) {
        const auto value=std::to_string(i);
        corpus.observe("\"Config2\"{\"User\"{\"Username\" \"user"+value+
            "\"\"UserId\" \""+value+"\"}\"nested\"{\"x\" \"old\"}\"nested\"{\"y\" \"new\"}}");
    }
    const std::array<std::string_view,16> tokens={
        "\"R\"","\"key\"","\"value\"","{","}"," ","\t","\r\n",
        "//text\n","/**/","/*","*/","\"","\\","!","\"a\\q\""
    };
    std::uint32_t random=0x91c4a2b7U;
    auto next=[&] {random^=random<<13;random^=random>>17;random^=random<<5;return random;};
    for(unsigned i=0;i<8192;++i) {
        std::string input;
        const auto count=next()%20;
        for(unsigned j=0;j<count;++j)input+=tokens[next()%tokens.size()];
        corpus.observe(input);
    }
}
}
int main() {
    try {
        Corpus corpus;std::string input;short_inputs(corpus,input,4);
        all_byte_contexts(corpus);document_mutations(corpus);generated_documents(corpus);
        std::cout<<"VDF corpus: "<<corpus.cases<<" cases; "<<corpus.accepted<<" accepted; "
            <<corpus.rejected<<" rejected; full-state FNV64="<<std::hex<<corpus.signature<<std::dec<<'\n';
        // Filled from the frozen pre-PEGTL parser before evaluating the new backend.
        if(corpus.signature!=0xea70f5d56d33f404ULL)
            throw std::runtime_error("VDF baseline observations changed");
        std::cout<<"PASS: acceptance, error position, AST, assignment chronology and partial action state match RE-derived baseline\n";
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
