// Synthetic non-XML grammar; not a surge.cfg/VDF or production grammar decision.
#include <tao/pegtl.hpp>
#include <iostream>

namespace grammar_fixture {
struct decimal : tao::pegtl::seq<tao::pegtl::plus<tao::pegtl::ascii::digit>,tao::pegtl::eof> {};
}

int main() {
    tao::pegtl::view_input valid("12345");
    tao::pegtl::view_input invalid("12x");
    tao::pegtl::view_input empty("");
    if(!tao::pegtl::parse<grammar_fixture::decimal>(valid)) return 10;
    if(tao::pegtl::parse<grammar_fixture::decimal>(invalid)) return 11;
    if(tao::pegtl::parse<grammar_fixture::decimal>(empty)) return 12;
    std::cout << "PASS: supplied PEGTL 4 public-input fixture; production grammar undecided\n";
}
