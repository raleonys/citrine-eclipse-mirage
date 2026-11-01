#pragma once
#include "policy.hpp"
#include "../anadius-native-context-source/context_test_fixture.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include <sstream>
#include <iostream>

namespace b16_test {
using namespace anadius;namespace cp=callback_policy;namespace nc=native_context;namespace n=native_recovered;
inline unsigned checks=0;
inline void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
template<class E,class F>void rejects(F&& f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
struct ConvertState {std::uint32_t locale=65001,cp=0;bool ansi=true,fail=false;unsigned queries=0,fills=0;std::u16string input;};
class Encoding final:public encoding_recovered::Calls {
    std::shared_ptr<ConvertState> s_;
public:
    explicit Encoding(std::shared_ptr<ConvertState> s):s_(std::move(s)){}
    std::uint32_t locale_codepage_0629ec()override{return s_->locale;}
    bool are_file_apis_ansi()override{return s_->ansi;}
    int multi_byte_to_wide(std::uint32_t,std::uint32_t,const char*,int,char16_t*,int)override{throw std::logic_error("unused wide conversion");}
    int wide_to_multi_byte(std::uint32_t cp,std::uint32_t,const char16_t* input,int size,char* out,int count,bool* used)override{
        s_->cp=cp;s_->input.assign(input,input+size);if(used)*used=false;if(s_->fail)return 0;
        if(out){++s_->fills;for(int i=0;i<count;++i)out[i]=static_cast<char>(input[i]);}else ++s_->queries;
        return size;
    }
    std::uint32_t last_error()override{return 1113;}
};
struct Terminal {std::uint32_t code;};
inline void selected_exit(std::uint32_t value){throw Terminal{value};}
inline void returning_exit(std::uint32_t){}
inline bool deny_free=false;
inline void* WINAPI selected_alloc(HANDLE heap,DWORD flags,SIZE_T bytes){
    auto& s=*b6_core_test::observed;++s.allocations;
    if(heap!=::GetProcessHeap()||(flags!=0&&flags!=HEAP_ZERO_MEMORY))throw std::logic_error("own process heap flags");
    if(s.deny_alloc){::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return nullptr;}
    auto* value=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();if(value)s.blocks.emplace(value,bytes);::SetLastError(error);return value;
}
inline BOOL WINAPI selected_free(HANDLE heap,DWORD flags,void* pointer){
    if(deny_free){::SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    return b6_core_test::free_heap(heap,flags,pointer);
}
struct Fixture {
    b6_core_test::Fixture heap;
    b15_test::Driver driver;
    std::shared_ptr<thread_domain::ApplicationMemory> memory;
    std::shared_ptr<std::ostringstream> output=std::make_shared<std::ostringstream>();
    std::shared_ptr<ConvertState> conversion=std::make_shared<ConvertState>();
    b12_test::Ui ui;
    Fixture(){
        heap.domain.reset();auto apis=b6_core_test::apis();
        apis.heap={b6_core_test::get_heap,selected_alloc,::HeapReAlloc,selected_free,::HeapSize,::GetLastError};
        heap.domain=thread_domain::DomainStorage::create(b6_core_test::seed(),apis);
        if(!heap.domain->initialize())throw std::runtime_error("own memory domain initialize");memory=heap.domain->make_memory();b12_test::ui=&ui;
    }
    ~Fixture(){b12_test::ui=nullptr;memory.reset();}
    cp::Resources resources(){cp::Resources r;r.memory=memory;r.stream=output;r.encoding=std::make_unique<Encoding>(conversion);r.dialog=library_effects::Dialog(b12_test::dialog_api());r.exit=selected_exit;return r;}
    nc::CallbackSeed callbacks=b15_test::seed();
    nc::PolicyView view(){return {driver.view(),callbacks};}
};
inline LSTATUS WINAPI failing_open_a(HKEY,LPCSTR,DWORD,REGSAM,PHKEY out){if(out)*out=reinterpret_cast<HKEY>(0xdead);::SetLastError(0x444);return 2;}
inline LSTATUS WINAPI failing_open_w(HKEY,LPCWSTR,DWORD,REGSAM,PHKEY out){if(out)*out=reinterpret_cast<HKEY>(0xbeef);return 2;}
inline void set_fabrication_originals(Fixture& f){
    f.driver.runtime.originals[host_init_recovered::slot(n::Callback::reg_open_a_00f800)]=reinterpret_cast<std::uintptr_t>(&failing_open_a);
    f.driver.runtime.originals[host_init_recovered::slot(n::Callback::reg_open_w_00f960)]=reinterpret_cast<std::uintptr_t>(&failing_open_w);
    f.driver.runtime.registry_values={{"ClientKey",u"Own\\Client"},{"ClientKey2",u"Own\\Client2"},{"ClientValue",u"Path"},
        {"ClientPath",u"C:\\own\\target.exe"},{"LanguageKey",u"Own\\Locale"},{"LanguageValue",u"Locale"},{"Language",u"en_US"},{"Documents",u"C:\\own\\docs"}};
}
inline std::u16string read(const void* value){std::u16string out;for(std::size_t i=0;;++i){char16_t c;std::memcpy(&c,static_cast<const std::byte*>(value)+i*2,2);if(!c)return out;out+=c;}}
} // namespace b16_test
