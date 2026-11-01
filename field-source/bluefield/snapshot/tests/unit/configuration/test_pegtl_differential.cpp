#include "tests/support/configuration/parser_snapshot.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

int main() {
    using namespace preloader_l::tests::parser_snapshot;
    try {
        auto root=std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path();
        std::ifstream file(root/"tests/fixtures/config2_fix09_corpus.tsv",std::ios::binary);
        if(!file)throw std::runtime_error("missing frozen Config2 corpus");
        std::string line;std::size_t count=0;
        while(std::getline(file,line)) {
            const auto sep=line.find('\t');
            if(sep==std::string::npos)throw std::runtime_error("invalid fixture record");
            const auto input=unhex(std::string_view(line).substr(0,sep));
            const auto expected=unhex(std::string_view(line).substr(sep+1));
            const auto actual=evaluate(input);++count;
            if(actual!=expected) {
                std::cerr<<"FAIL Config2 corpus record "<<count<<" input="<<hex(input)
                         <<"\nexpected="<<hex(expected)<<"\nactual="<<hex(actual)<<'\n';return 1;
            }
        }
        if(!file.eof() || count!=8766)throw std::runtime_error("incomplete corpus read");
        std::cout<<"PASS: "<<count<<" frozen FIX09 Config2 comparisons (tree, actions, errors, BOM).\n";
        return 0;
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
