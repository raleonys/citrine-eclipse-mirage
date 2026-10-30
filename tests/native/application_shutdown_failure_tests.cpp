#include "preloader_l/runtime/control.h"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
void require(bool b,const char* text){if(!b)throw std::runtime_error(text);}
struct Module {
    HMODULE module=nullptr;
    DWORD (WINAPI* stop)(DWORD)=nullptr;
    void (*mode)(unsigned)=nullptr;
    unsigned (*value)(unsigned)=nullptr;
    explicit Module(const char* path){
        module=LoadLibraryA(path);require(module!=nullptr,"fixture DLL load");
        stop=reinterpret_cast<decltype(stop)>(GetProcAddress(module,"DllShutdown"));
        mode=reinterpret_cast<decltype(mode)>(GetProcAddress(module,"FixtureMode"));
        value=reinterpret_cast<decltype(value)>(GetProcAddress(module,"FixtureValue"));
        require(stop&&mode&&value,"fixture methods");
    }
    ~Module(){if(mode&&stop){mode(0);if(stop(10000)==DLL_SHUTDOWN_OK)FreeLibrary(module);}}
};
void scenario(const char* path,unsigned fault){
    Module m(path);
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while(!m.value(0)&&std::chrono::steady_clock::now()<until)Sleep(1);
    require(m.value(0)&&m.value(6),"real application and hook binding ready");
    m.mode(fault);
    const auto result=m.stop(1000);
    require(result==DLL_SHUTDOWN_CLEANUP_FAILED,"incomplete hook cleanup must return failure for an explicit retry");
    require(m.value(1)==1&&m.value(2)==1&&m.value(3)==0&&m.value(6),"failed cleanup retains sole application and binding");
    require(m.value(4)==1,"single shutdown request must not busy-retry hook disable");
    require(m.value(7)==(fault==2?1u:0u),"real in-flight lease survives failed cleanup");
    m.mode(0);require(m.stop(10000)==DLL_SHUTDOWN_OK,"same application cleanup retry");
    require(m.value(1)==2&&m.value(2)==1&&m.value(3)==1&&!m.value(6),"retry destroys sole owner only after binding retirement");
    require(m.value(4)==(fault==1?2u:1u),"successful disable must not repeat during retry");
    require(m.value(5)==(fault==3?2u:1u),"retry releases only unfinished engine ownership");
    require(m.stop(0)==DLL_SHUTDOWN_OK,"completed shutdown is idempotent");
    std::cout<<"PASS: actual entry/Application/hook cleanup fault "<<fault<<'\n';
}
}
int main(int argc,char** argv)try{
    require(argc==2||argc==3,"fixture DLL path and optional mode required");
    bool ran=false;
    for(unsigned fault=1;fault<=4;++fault)
        if(argc==2||std::string_view(argv[2])==std::to_string(fault)){scenario(argv[1],fault);ran=true;}
    require(ran,"unknown fault mode");
    return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
