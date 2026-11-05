#include "policy.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <algorithm>
#include <bit>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <type_traits>
#include <vector>

namespace anadius::callback_policy {
namespace {
struct LastError {
    DWORD value=::GetLastError();
    ~LastError(){::SetLastError(value);}
    void restore() const {::SetLastError(value);}
    void capture(){value=::GetLastError();}
};
std::wstring stage(const void* input){
    std::wstring text;
    if(input)for(std::size_t i=0;;++i){
        wchar_t unit;std::memcpy(&unit,static_cast<const std::byte*>(input)+i*2,2);
        if(!unit)break;
        if(text.size()==INT_MAX)throw std::domain_error("Wide input exceeds selected INT_MAX domain");
        text+=unit;
    }
    return text;
}
bool overlap(const void* a,std::size_t an,const void* b,std::size_t bn){
    if(!a||!b||!an||!bn)return false;
    const auto x=reinterpret_cast<std::uintptr_t>(a),y=reinterpret_cast<std::uintptr_t>(b);
    return x<=y?y-x<an:x-y<bn;
}
}
CrtApi compiler_crt_api() noexcept{return {::_stricmp,::strcpy_s,::_wcsicmp,::wcscpy_s};}
ByteWide::ByteWide(CrtApi api,std::shared_ptr<CrtProvider> owned):api_(api),owned_(std::move(owned)){
    if(!owned_&&(!api_.compare_a||!api_.copy_a||!api_.compare_w||!api_.copy_w))
        throw std::invalid_argument("All compiler CRT policies are required");
}
int ByteWide::compare_utf16(const void* a,const void* b){
    LastError error;const auto x=stage(a),y=stage(b);error.restore();
    try{const auto result=owned_?owned_->compare_w(a?x.c_str():nullptr,b?y.c_str():nullptr):api_.compare_w(a?x.c_str():nullptr,b?y.c_str():nullptr);error.capture();return result;}
    catch(...){error.capture();throw;}
}
int ByteWide::copy_utf16(void* out,std::size_t capacity,const void* input){
    LastError error;
    if(capacity>INT_MAX)throw std::domain_error("Wide capacity exceeds selected INT_MAX domain");
    const auto text=stage(input);
    if(overlap(out,capacity*2,input,input?(text.size()+1)*2:0))
        throw std::domain_error("Overlapping wide copy is outside the selected CRT domain");
    std::vector<wchar_t> buffer(out?capacity:0);wchar_t zero=0;
    // CRT clears valid output BEFORE invoking a possibly nonreturning invalid
    // handler. Preserve that visibility on the actual borrowed byte storage.
    if(out&&capacity&&(!input||text.size()+1>capacity))std::memcpy(out,&zero,2);
    error.restore();
    int result;
    try{result=owned_?owned_->copy_w(out?(capacity?buffer.data():&zero):nullptr,capacity,input?text.c_str():nullptr):api_.copy_w(out?(capacity?buffer.data():&zero):nullptr,capacity,input?text.c_str():nullptr);}
    catch(...){error.capture();throw;}
    error.capture();
    if(!result){
        if(!out||!input||text.size()+1>capacity)throw std::logic_error("CRT copy violated its result contract");
        std::memcpy(out,buffer.data(),(text.size()+1)*2);
    }else if(out&&capacity)std::memcpy(out,buffer.data(),2);
    return result;
}
[[noreturn]] void compiler_exit(std::uint32_t code){std::exit(std::bit_cast<std::int32_t>(code));}
Storage::Storage(Resources value,const native_recovered::OriginalSlots& slots):resources(std::move(value)),wide(resources.crt,resources.owned_crt),
    original_owner(resources.originals?resources.originals(slots):nullptr),
    original_boundary(resources.original_scope?std::make_shared<anadius::original_checkpoint::Boundary>(resources.original_scope):
        resources.callback_scope?std::make_shared<anadius::original_checkpoint::Boundary>(resources.callback_scope):nullptr){
    if(!resources.memory||!resources.stream||!resources.encoding||!resources.errors||!resources.exit)
        throw std::invalid_argument("Owned memory/stream/conversion/terminal policies required");
    if(resources.originals&&!original_owner)throw std::invalid_argument("Original provider factory returned null");
}
Policy::Policy(native_context::PolicyView view,Resources resources)
    :Storage(std::move(resources),view.backend.runtime.originals),OriginalEffects(view.backend.runtime.originals,wide,original_owner,original_boundary),
     execution_(view.backend.execution),controller_(::GetCurrentThreadId()){
    if(!view.callbacks.fabricated.empty())throw std::invalid_argument("Incoming fabricated handles cannot be adopted");
}
Policy::~Policy() noexcept{if(!handles_.empty())std::abort();}
void Policy::check_thread() const {
    LastError error;
    if(resources.callback_scope){
        if(!resources.callback_scope())throw std::logic_error("Callback policy requires its active admitted PTD scope");
        return;
    }
    if(::GetCurrentThreadId()!=controller_)throw std::logic_error("Callback policy requires its serialized controller thread");
}
std::size_t Policy::owned_handles() const{check_thread();return handles_.size();}
void Policy::check_connect_body()const{
    LastError error;
    if(resources.connect_body_scope){if(!resources.connect_body_scope())throw std::logic_error("Network body requires its admitted PTD scope");return;}
    check_thread();
}
std::string Policy::filesystem_narrow(std::u16string_view text){
    check_thread();auto& r=resources;
    const auto cp=encoding_recovered::filesystem_codepage_048ac0(*r.encoding);
    return encoding_recovered::narrow_006e60(text,cp,*r.encoding,*r.errors);
}
int Policy::crt_stricmp(const char* a,const char* b){check_thread();return resources.owned_crt?resources.owned_crt->compare_a(a,b):resources.crt.compare_a(a,b);}
int Policy::crt_strcpy_s(char* a,std::size_t n,const char* b){check_thread();return resources.owned_crt?resources.owned_crt->copy_a(a,n,b):resources.crt.copy_a(a,n,b);}
registry_recovered::Handle Policy::allocate_4_04b0bc(){
    check_thread();LastError error;const auto ticket=handles_.insert(handles_.end(),0);error.restore();
    try{*ticket=reinterpret_cast<std::uintptr_t>(resources.memory->allocate(4));error.capture();return *ticket;}
    catch(...){error.capture();handles_.erase(ticket);throw;}
}
void Policy::free_04b0f8(registry_recovered::Handle handle){
    check_thread();LastError error;const auto it=std::find(handles_.begin(),handles_.end(),handle);
    if(it==handles_.end())throw std::invalid_argument("Foreign or already released callback handle");
    error.restore();
    try{resources.memory->deallocate(reinterpret_cast<void*>(handle),4);error.capture();handles_.erase(it);}
    catch(...){error.capture();throw;}
}
void Policy::wait_036880(std::int32_t n){check_thread();execution_.wait_steps_036880(n);}
void Policy::wait_steps_036880(std::int32_t n){
    if(n==-1&&resources.connect_wait_scope){
        {LastError error;if(!resources.connect_wait_scope())throw std::logic_error("Network startup wait requires its admitted PTD scope");}
        execution_.wait_steps_036880(n);return;
    }
    wait_036880(n);
}
std::string_view Policy::registry_literal(std::uint64_t va){
    switch(va){
    case 0x180080528:return "fake key created";
    case 0x180080540:return "fake key deleted";
    case 0x1800808c8:return "replacing Client Path";
    case 0x1800808e0:return "replacing the language with ";
    case 0x180080900:return "fake key queried something else than what I replace";
    }
    throw std::domain_error("Registry literal outside retained VA whitelist");
}
void Policy::log_literal_006880(std::uint64_t va){check_thread();const auto text=registry_literal(va);const auto shared=shared_log::acquire(*resources.stream);*resources.stream<<text.data();}
void Policy::log_text_007990(std::string_view text){check_thread();const auto shared=shared_log::acquire(*resources.stream);*resources.stream<<text;}
void Policy::finish_log_0088e0(){check_thread();const auto shared=shared_log::acquire(*resources.stream);*resources.stream<<std::endl;}
void Policy::log_literal_006880(std::string_view text){
    connect_body_call([&]{
        if(text!="game connected on port "&&text!="; redirecting to ")throw std::domain_error("Network literal outside recovered domain");
        const auto shared=shared_log::acquire(*resources.stream);*resources.stream<<(text=="game connected on port "?"game connected on port ":"; redirecting to ");
    });
}
void Policy::log_port_00ad70(std::uint16_t value){connect_body_call([&]{const auto shared=shared_log::acquire(*resources.stream);*resources.stream<<value;});}
void Policy::log_finish_0088e0(){connect_body_call([&]{const auto shared=shared_log::acquire(*resources.stream);*resources.stream<<std::endl;});}
void Policy::dialog_038bc0(std::string_view title,std::string_view text,std::uint32_t flags){connect_body_call([&]{resources.dialog.show(title,text,flags);});}
[[noreturn]] void Policy::exit_05cdc0(std::uint32_t code){connect_body_call([&]{resources.exit(code);});std::terminate();}
native_context::PolicyFactory make_policy_factory(Resources resources){
    struct State {Resources resources;bool used=false;};
    auto state=std::make_shared<State>(State{std::move(resources)});
    return [state](native_context::PolicyView view)->std::unique_ptr<native_recovered::OriginalEffects>{
        if(state->used)throw std::logic_error("Callback policy resources already consumed");state->used=true;
        return std::make_unique<Policy>(view,std::move(state->resources));
    };
}
static_assert(sizeof(wchar_t)==2&&!std::is_abstract_v<Policy>&&!std::is_abstract_v<ByteWide>);
static_assert(!std::is_move_constructible_v<Policy>);
} // namespace anadius::callback_policy
