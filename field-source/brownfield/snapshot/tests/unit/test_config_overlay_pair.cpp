#include "preloader_l/config/runtime_configuration_file.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#include "config2_pegtl.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <vector>
using namespace preloader_l;
static std::map<std::string,std::string> files,settings;
static std::string opened;
static unsigned long err=2;
static int commits=0,allocs=0,frees=0,read_error=0,close_error=0;
static int fail_alloc=0;
static void* bad(){return reinterpret_cast<void*>(static_cast<intptr_t>(-1));}
static std::string narrow(const uint16_t* p){std::string s;while(p&&*p)s.push_back(char(*p++));return s;}
extern "C" {
void* CreateFileA(const char*,unsigned long,unsigned long,void*,unsigned long,unsigned long,void*){return bad();}
void* CreateFileW(const uint16_t* p,unsigned long,unsigned long,void*,unsigned long,unsigned long,void*){
 opened=narrow(p); auto it=files.find(opened);if(it==files.end()){err=2;return bad();}
 return &it->second;
}
int ReadFile(void* h,void* b,unsigned long want,unsigned long* got,void*){
 if(read_error){*got=0;return 0;}
 auto& v=*static_cast<std::string*>(h);
 static std::map<void*,size_t> off;
 size_t n=v.size()-off[h];if(n>want)n=want;
 if(n)std::memcpy(b,v.data()+off[h],n);
 off[h]+=n;*got=(unsigned long)n;
 if(n<want)off.erase(h);
 return 1;
}
int CloseHandle(void*){return close_error?0:1;}
unsigned long GetLastError(){return err;}
}
static void* alloc(void*,size_t n) noexcept {if(fail_alloc){--fail_alloc;return nullptr;}++allocs;return std::malloc(n);}
static int fre(void*,void* p) noexcept{++frees;std::free(p);return 1;}
static int nh(void*,size_t){return 0;}
namespace preloader_l { namespace runtime {
const AllocatorPlatformOps& windows_allocator_platform_ops() noexcept {
 static const AllocatorPlatformOps o{nullptr,&alloc,&fre,&nh}; return o;
}
AllocationResult allocate_block(size_t n,AllocationRetryPolicy,const AllocatorPlatformOps& o) {
 void* p=o.heap_alloc?o.heap_alloc(o.context,n):nullptr;
 return AllocationResult{p,p?AllocationTerminal::none:AllocationTerminal::bad_alloc,n,n,0u,false};
}
AllocationReleaseStatus release_block(void* p,size_t,const AllocatorPlatformOps& o) noexcept {
 if(!p)return AllocationReleaseStatus::ok;
 return o.heap_free&&o.heap_free(o.context,p)?AllocationReleaseStatus::ok:AllocationReleaseStatus::release_failed;
}
}}
struct State {std::map<std::string,std::string> keys;const char* base;};
static bool leaf(void* ptr,const config2_pegtl::Span* path,size_t dep,config2_pegtl::Span k,config2_pegtl::Span v){
 auto& s=*static_cast<State*>(ptr);
 if(dep==1){std::string p(s.base+path[0].off,path[0].len);s.keys[p+"."+std::string(s.base+k.off,k.len)]=std::string(s.base+v.off,v.len);}
 return true;
}
namespace preloader_l { namespace config {
RuntimeConfigurationStatus set_runtime_configuration_cfg(const char* p,size_t n) noexcept {
 State s{{},p};config2_pegtl::Visitor v{&s,nullptr,&leaf};
 if(config2_pegtl::parse(p,n,v)!=config2_pegtl::Result::ok)return RuntimeConfigurationStatus::invalid_cfg;
 settings=std::move(s.keys);++commits;return RuntimeConfigurationStatus::ok;
}
}}
static bool require(bool v,int code){if(!v){std::fprintf(stderr,"CASE FAIL %d\n",code);std::exit(code);}return true;}
static constexpr uint16_t base_path[]={ 'b','a','s','e','.','c','f','g',0};
static constexpr uint16_t override_path[]={'o','v','e','r','r','i','d','e','.','c','f','g',0};
static auto load(){return config::load_runtime_configuration_pair_wide(base_path,override_path);}
int main(){using config::RuntimeConfigurationFileStatus;using Status=RuntimeConfigurationFileStatus;
 const std::string b=R"("Config2" { "Game" { "BlockReruns" "false" "Language" "en_US" } "User" { "Username" "Alice" } })";
 const std::string o=R"("Config2" { "Game" { "BlockReruns" "true" } "User" { "Username" "Bob" } })";
 require(load()==Status::not_found&&commits==0,1);
 files["base.cfg"]=b;
 require(load()==Status::ok&&commits==1&&settings["Game.Language"]=="en_US",2);
 require(settings["Game.BlockReruns"]=="false"&&settings["User.Username"]=="Alice",3);
 files["override.cfg"]=o;
 require(load()==Status::ok&&commits==2,4);
 require(settings["Game.Language"]=="en_US"&&settings["Game.BlockReruns"]=="true"&&settings["User.Username"]=="Bob",5);
 files.erase("base.cfg");require(load()==Status::ok&&commits==3&&settings.find("Game.Language")==settings.end(),6);
 files["base.cfg"]=b;
 files["override.cfg"]="\xef\xbb\xbf\"Config2\" { /* brace } */ \"Game\" { \"Language\" \"fr_FR\" } } // trailer\n";
 require(load()==Status::ok&&commits==4&&settings["Game.Language"]=="fr_FR"&&settings["User.Username"]=="Alice",7);
 files["override.cfg"]="\"Config2\" { \"Game\" { \"Language\" \"broken\" }";
 const auto snapshot=settings;require(load()==Status::invalid_cfg&&commits==4&&settings==snapshot,8);
 files["override.cfg"]=o;
 files["base.cfg"]="\"Config2\" { \"Game\" { \"Language\" \"bad\" }";
 require(load()==Status::invalid_cfg&&commits==4&&settings==snapshot,9);
 files["base.cfg"]=b;
 read_error=1;require(load()==Status::io_error&&commits==4&&settings==snapshot,10);read_error=0;
 close_error=1;require(load()==Status::io_error&&commits==4&&settings==snapshot,11);close_error=0;
 fail_alloc=1;require(load()==Status::resource_error&&commits==4&&settings==snapshot,12);
 require(allocs==frees,13);
 std::printf("PAIR_GCC_CLANG_GENUINE_PEGTL 13/13 PASS commits=%d\n",commits);
}
