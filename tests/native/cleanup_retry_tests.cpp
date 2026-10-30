#include "preloader_l/runtime/control.h"
#include <iostream>
#include <stdexcept>
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
int main(int argc,char** argv)try{
    require(argc==2,"fixture DLL path required");auto module=LoadLibraryA(argv[1]);require(module!=nullptr,"fixture loaded");
    auto stop=reinterpret_cast<DWORD(WINAPI*)(DWORD)>(GetProcAddress(module,"DllShutdown"));
    auto attempts=reinterpret_cast<int(*)()>(GetProcAddress(module,"FixtureAttempts"));
    auto destroyed=reinterpret_cast<int(*)()>(GetProcAddress(module,"FixtureDestroyed"));
    auto created=reinterpret_cast<int(*)()>(GetProcAddress(module,"FixtureCreated"));
    auto caller=reinterpret_cast<std::uint32_t(*)()>(GetProcAddress(module,"FixtureCaller"));
    require(stop&&attempts&&destroyed&&created&&caller,"fixture methods resolved");
    require(stop(250)==DLL_SHUTDOWN_CLEANUP_FAILED&&attempts()==1&&destroyed()==0,"first cleanup failure retains owner");
    require(caller()==0,"failed shutdown return removes caller exclusion");
    require(stop(250)==DLL_SHUTDOWN_CLEANUP_FAILED&&attempts()==2&&destroyed()==0,"second request retries the same owner");
    require(caller()==0,"retry failure does not leave a stale scan exclusion");
    require(stop(5000)==DLL_SHUTDOWN_OK&&attempts()==3&&destroyed()==1&&created()==1,"successful retry destroys the sole owner after worker completion");
    require(caller()==0,"successful return also clears caller identity");
    require(FreeLibrary(module)!=FALSE,"fixture caller reference released");
    std::cout<<"PASS: actual DLL worker retains ownership across two failed cleanup requests\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
