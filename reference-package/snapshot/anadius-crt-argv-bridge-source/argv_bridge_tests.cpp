#include "test_fixtures.hpp"
int main(){try{for(bool present:{false,true})for(bool fallback:{false,true})anadius::crt_argv_test::combined(present,fallback);anadius::crt_argv_test::cleanup_order();std::cout<<"PASS "<<f::checks<<" argv/CRT/heap/PTD/classification composition checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
