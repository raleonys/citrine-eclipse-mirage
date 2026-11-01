#include "test_fixtures.hpp"
int main(){try{anadius::vcrt_test::combined_lifetime(true);anadius::vcrt_test::combined_lifetime(false);std::cout<<"PASS "<<anadius::vcrt_test::checks<<" VCRT/ACRT/CPU/locks/entry composition checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
