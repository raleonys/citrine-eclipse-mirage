// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/port_publication.hpp"
#include <iostream>
using namespace preloader_l;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Fake {
    std::optional<std::string> value;bool fail_read=false,fail_write=false;unsigned writes=0;
    static bool read(void* p,std::optional<std::string>& out) noexcept {auto& f=*static_cast<Fake*>(p);if(f.fail_read)return false;try{out=f.value;return true;}catch(...){return false;}}
    static bool write(void* p,const char* value) noexcept {auto& f=*static_cast<Fake*>(p);++f.writes;if(f.fail_write)return false;try{if(value)f.value=value;else f.value.reset();return true;}catch(...){return false;}}
    PortBackend backend(){return {this,read,write};}
};
int main(){
    for(auto initial:{std::optional<std::string>{},std::optional<std::string>{""},std::optional<std::string>{"host-port"}}){
        Fake f;f.value=initial;PortPublication p(f.backend());CHECK(!p.publish(0));CHECK(p.publish(43123));CHECK(f.value=="43123" && p.pending());CHECK(!p.publish(5));
        f.fail_read=true;CHECK(!p.restore() && p.pending());f.fail_read=false;
        f.fail_write=true;CHECK(!p.restore() && p.pending());f.fail_write=false;
        CHECK(p.restore() && !p.pending() && f.value==initial);const auto writes=f.writes;CHECK(p.restore() && writes==f.writes);
    }
    {Fake f;PortPublication p(f.backend());f.fail_read=true;CHECK(!p.publish(2) && !p.pending() && f.writes==0);f.fail_read=false;f.fail_write=true;CHECK(!p.publish(3) && !p.pending());}
    {Fake f;PortPublication p(f.backend());CHECK(p.publish(5));f.value="later-owner";CHECK(p.restore() && f.value=="later-owner" && f.writes==1);}
    {Fake f;PortPublication p(f.backend());CHECK(p.publish(5));f.value.reset();CHECK(p.restore() && !f.value && f.writes==1);}
    {PortPublication disabled;CHECK(disabled.publish(6) && !disabled.pending() && disabled.restore());PortPublication invalid({nullptr,Fake::read,nullptr});CHECK(!invalid.publish(6));}
    std::cout<<"PORT_PUBLICATION_RETRY_RESTORE_PASS\n";
}
