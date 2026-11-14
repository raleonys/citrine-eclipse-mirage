// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/runtime/control.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
namespace {
struct Environment {
    std::string previous;bool present=false;
    static constexpr const char* name="LAUREONDO_TEST_FAIL_WORKER_IMAGE_LEASE";
    explicit Environment(const char* value) {
        SetLastError(ERROR_SUCCESS);const auto size=GetEnvironmentVariableA(name,nullptr,0);
        if(size) {
            previous.resize(size);SetLastError(ERROR_SUCCESS);
            const auto copied=GetEnvironmentVariableA(name,previous.data(),size);
            require(copied<size&&(copied||GetLastError()==ERROR_SUCCESS),"capture prior worker fixture environment");
            previous.resize(copied);present=true;
        }else {
            const auto error=GetLastError();
            require(error==ERROR_SUCCESS||error==ERROR_ENVVAR_NOT_FOUND,"query worker fixture environment");
            present=error==ERROR_SUCCESS;
        }
        require(SetEnvironmentVariableA(name,value)!=FALSE,"select worker module acquisition case");
    }
    ~Environment(){SetEnvironmentVariableA(name,present?previous.c_str():nullptr);}
};
void image_lease_failure(const char* path) {
    Environment fault("1");
    const auto name=std::filesystem::path(path).filename();
    require(!GetModuleHandleW(name.c_str()),"failure case starts without an earlier fixture image");
    struct Caller {
        HMODULE module;DWORD(WINAPI* stop)(DWORD)=nullptr;
        ~Caller(){if(module&&stop)for(unsigned i=0;i<3;++i)if(stop(1000)==DLL_SHUTDOWN_OK){FreeLibrary(module);break;}}
    } caller{LoadLibraryA(path)};
    require(caller.module!=nullptr,"module acquisition failure still permits fixture attach");
    caller.stop=reinterpret_cast<decltype(caller.stop)>(GetProcAddress(caller.module,"DllShutdown"));
    const auto worker=reinterpret_cast<DWORD(*)(unsigned)>(GetProcAddress(caller.module,"FixtureWorker"));
    const auto created=reinterpret_cast<int(*)()>(GetProcAddress(caller.module,"FixtureCreated"));
    const auto attempts=reinterpret_cast<int(*)()>(GetProcAddress(caller.module,"FixtureAttempts"));
    const auto destroyed=reinterpret_cast<int(*)()>(GetProcAddress(caller.module,"FixtureDestroyed"));
    const auto scan_caller=reinterpret_cast<std::uint32_t(*)()>(GetProcAddress(caller.module,"FixtureCaller"));
    require(caller.stop&&worker&&created&&attempts&&destroyed&&scan_caller,"early failure fixture methods resolved");
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(worker(0)==WAIT_TIMEOUT&&std::chrono::steady_clock::now()<deadline)Sleep(1);
    require(worker(0)==WAIT_OBJECT_0,"failed module acquisition physically ends the worker");
    require(worker(1)==ERROR_ACCESS_DENIED&&worker(2)==0&&worker(3)==1&&worker(4)==1,
        "one address-based module failure retains its worker exit error and acquires no image lease");
    require(created()==0&&attempts()==0&&destroyed()==0,"failure precedes Application construction and cleanup");
    require(caller.stop(5000)==DLL_SHUTDOWN_OK&&caller.stop(0)==DLL_SHUTDOWN_OK&&scan_caller()==0,
        "already ended early worker permits idempotent shutdown and clears caller exclusion");
    require(created()==0&&attempts()==0&&destroyed()==0,"public cleanup does not invent an Application after early failure");
    require(FreeLibrary(caller.module)!=FALSE,"release the sole early-failure caller reference");caller.module=nullptr;
    require(!GetModuleHandleW(name.c_str()),"early module failure leaves no physical image lease");
    std::cout<<"PASS: actual entry worker module-acquisition failure ends without an Application or leaked image\n";
}
}
int main(int argc,char** argv)try{
    require(argc==2,"fixture DLL path required");Environment healthy(nullptr);image_lease_failure(argv[1]);
    auto module=LoadLibraryA(argv[1]);require(module!=nullptr,"healthy reload after early worker failure");
    auto stop=reinterpret_cast<DWORD(WINAPI*)(DWORD)>(GetProcAddress(module,"DllShutdown"));
    auto attempts=reinterpret_cast<int(*)()>(GetProcAddress(module,"FixtureAttempts"));
    auto destroyed=reinterpret_cast<int(*)()>(GetProcAddress(module,"FixtureDestroyed"));
    auto created=reinterpret_cast<int(*)()>(GetProcAddress(module,"FixtureCreated"));
    auto caller=reinterpret_cast<std::uint32_t(*)()>(GetProcAddress(module,"FixtureCaller"));
    auto worker=reinterpret_cast<DWORD(*)(unsigned)>(GetProcAddress(module,"FixtureWorker"));
    require(stop&&attempts&&destroyed&&created&&caller&&worker,"fixture methods resolved");
    require(stop(250)==DLL_SHUTDOWN_CLEANUP_FAILED&&attempts()==1&&destroyed()==0,"first cleanup failure retains owner");
    require(caller()==0,"failed shutdown return removes caller exclusion");
    require(stop(250)==DLL_SHUTDOWN_CLEANUP_FAILED&&attempts()==2&&destroyed()==0,"second request retries the same owner");
    require(caller()==0,"retry failure does not leave a stale scan exclusion");
    require(stop(5000)==DLL_SHUTDOWN_OK&&attempts()==3&&destroyed()==1&&created()==1,"successful retry destroys the sole owner after worker completion");
    require(caller()==0,"successful return also clears caller identity");
    require(worker(0)==WAIT_OBJECT_0&&worker(1)==0&&worker(2)==0&&worker(3)==1&&worker(4)==1,
        "healthy reload forwards one correct acquisition and physically ends after releasing the image lease");
    require(FreeLibrary(module)!=FALSE,"fixture caller reference released");
    require(!GetModuleHandleW(std::filesystem::path(argv[1]).filename().c_str()),"healthy reload releases both worker and caller image references");
    std::cout<<"PASS: actual DLL worker retains ownership across two failed cleanup requests\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
