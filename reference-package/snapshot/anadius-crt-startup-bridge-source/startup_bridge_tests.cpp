#include "test_fixtures.hpp"
int main(){try{for(bool present:{false,true})for(bool env_failure:{false,true})for(bool mutate:{false,true})anadius::crt_startup_test::combined(present,env_failure,mutate);std::cout<<"PASS "<<f::checks<<" CRT startup/termination/environment composition checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
