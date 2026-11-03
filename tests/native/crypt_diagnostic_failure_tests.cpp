// SPDX-License-Identifier: GPL-3.0-only

#include "src/runtime/application_internal.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <new>
#include <stdexcept>

namespace fault {thread_local bool next=false;unsigned injected=0;}
void* operator new(std::size_t n){
    if(fault::next){fault::next=false;++fault::injected;throw std::bad_alloc();}
    if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();
}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}
void operator delete[](void* p,std::size_t)noexcept{std::free(p);}
#include "src/runtime/application.cpp"

namespace {
using namespace preloader_l;
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
struct Fixture {
    configuration::State state=application::initial_state();application::Options options;
    file::NativeFactory files;xml_reader_bridge::Backend xml;
    achievement_storage::Paths paths;achievement_runtime::HostClock clock;achievement_runtime::HostFormatter formatter;
    application::detail::Environment env{state,options,files,xml,paths,clock,formatter};
    execution::MutexState mutex{};std::uint8_t a=0,b=0,c=0;
    execution::WindowsPlatform platform{+[](int){throw std::runtime_error("mutex error");}};
    std::int64_t frequency=platform.query_performance_frequency();
    execution::Execution execution{mutex,{a,b,c},frequency,platform};
    application::Hooks hooks{state,env,execution};
    Fixture(){execution::initialize_mutex_04a698(mutex,2);}
};
}
int main()try {
    Fixture f;
    // With no configured directory the real Debug logger queues its first
    // record. Fail that allocation after the observer sets its reentry guard.
    fault::next=true;
    f.hooks.crypt_observed(hooks::Selection{});
#ifdef NDEBUG
    require(fault::next&&fault::injected==0,"Release observer must not allocate for diagnostics");
    fault::next=false;
#else
    require(!fault::next&&fault::injected==1,"actual Debug logger must consume the allocation fault");
    f.hooks.crypt_observed(hooks::Selection{});
    const auto directory=std::filesystem::temp_directory_path()/
        ("surge-crypt-diagnostic-"+std::to_string(GetCurrentProcessId()));
    require(std::filesystem::create_directory(directory),"isolated diagnostic directory");
    struct Remove {std::filesystem::path p;~Remove(){std::error_code e;std::filesystem::remove_all(p,e);}} remove{directory};
    f.state.user_directory=application::detail::utf16(directory.c_str());
    f.env.log(std::initializer_list<std::string_view>{});
    std::ifstream input(directory/"surge.log",std::ios::binary);
    const std::string records((std::istreambuf_iterator<char>(input)),{});
    require(records=="phase\n","observer guard must recover and record the next callback exactly once");
#endif
    std::cout<<"PASS: actual crypt diagnostic exception boundary and recovery\n";
    return 0;
}catch(const std::exception& e){fault::next=false;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
