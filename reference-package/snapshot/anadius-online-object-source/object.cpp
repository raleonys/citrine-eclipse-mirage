#include "object.hpp"
#include "../anadius-online-version-source/version.hpp"
#include <cstring>
#include <stdexcept>
#include <cstdlib>
#include <limits>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace anadius::online_object {
DestroyFunction destroy_slot(Object object){
    if(!object)throw online_recovered::OutOfEvidenceInput("Null adopted online object");
    std::uintptr_t table=0,address=0;
    std::memcpy(&table,reinterpret_cast<const void*>(object),sizeof(table));
    if(!table)throw online_recovered::OutOfEvidenceInput("Online object has null vptr");
    std::memcpy(&address,reinterpret_cast<const void*>(table),sizeof(address));
    if(!address)throw online_recovered::OutOfEvidenceInput("Online object has null destroy slot");
    return reinterpret_cast<DestroyFunction>(address);
}
}
namespace anadius::online_version {
namespace {struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}void restore()const{::SetLastError(value);}void capture(){value=::GetLastError();}};}
Procedure Binding::resolve_factory(Module module,const std::function<Procedure()>& lower){
    Error error;std::shared_ptr<void> retired;
    if(!lower)throw std::invalid_argument("Explicit online factory lookup required");
    {std::lock_guard lock(gate_);
     if(active_||queries_||poisoned_||object_||object_unknown_||!module||module!=module_)throw std::logic_error("Factory lookup requires exact quiescent loaded module");
     active_=true;factory_=0;retired=std::move(factory_code_);}
    bool lookup_returned=false;
    try{
        error.restore();const auto procedure=lower();error.capture();lookup_returned=true;
        std::shared_ptr<void> code;
        if(procedure){code=lease_(module);if(!code)throw std::invalid_argument("Online factory lease returned null");}
        {std::lock_guard lock(gate_);factory_=procedure;factory_code_=std::move(code);}
        retired.reset();{std::lock_guard lock(gate_);active_=false;}
        return procedure;
    }catch(...){if(!lookup_returned)error.capture();retired.reset();std::lock_guard lock(gate_);active_=false;throw;}
}
Object Binding::create(Procedure procedure,const char16_t* cookie){
    Error error;online_object::FactoryFunction function;
    {std::lock_guard lock(gate_);
     if(active_||queries_||poisoned_||object_||object_unknown_||!module_||!procedure||procedure!=factory_||!factory_code_)
         throw std::logic_error("Factory call requires exact resolved export and no prior object");
     if(!cookie)throw online_recovered::OutOfEvidenceInput("Factory cookie must be readable NUL UTF16");
     active_=true;object_unknown_=true;function=reinterpret_cast<online_object::FactoryFunction>(factory_);}
    try{
        error.restore();const auto object=function(cookie);error.capture();
        {std::lock_guard lock(gate_);object_=object;object_unknown_=false;active_=false;}
        return object;
    }catch(...){error.capture();std::lock_guard lock(gate_);poisoned_=true;active_=false;throw;}
}
void Binding::destroy(Object object){
    Error error;
    {std::lock_guard lock(gate_);
     if(active_||queries_||poisoned_||object_unknown_||!module_||!object||object!=object_||!factory_code_)
         throw std::logic_error("Destroy requires exact adopted live object");
     active_=true;object_unknown_=true;}
    bool invoked=false;
    try{
        const auto function=online_object::destroy_slot(object);
        error.restore();invoked=true;function(reinterpret_cast<void*>(object));error.capture();
        {std::lock_guard lock(gate_);object_=0;object_unknown_=false;active_=false;}
    }catch(...){if(invoked)error.capture();std::lock_guard lock(gate_);poisoned_=true;active_=false;throw;}
}
void Binding::begin_object(Object object){
    Error error;std::lock_guard lock(gate_);
    if(active_||poisoned_||object_unknown_||!module_||!object||object!=object_||!factory_code_||
       (query_policy_==QueryPolicy::Serialized&&queries_))
        throw std::logic_error("Foreign query requires exact adopted live object");
    if(queries_==std::numeric_limits<std::size_t>::max())throw std::length_error("Online query counter overflow");
    ++queries_;
}
void Binding::end_object()noexcept{Error error;std::lock_guard lock(gate_);if(!queries_)std::abort();--queries_;}
}
