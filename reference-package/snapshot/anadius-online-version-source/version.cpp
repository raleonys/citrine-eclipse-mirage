#include "version.hpp"
#include <stdexcept>
#include <cstdlib>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace anadius::online_version {
namespace {struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}void capture(){value=::GetLastError();}void restore()const{::SetLastError(value);}};}
Binding::Binding(CodeLease lease,QueryPolicy policy):lease_(std::move(lease)),query_policy_(policy){if(!lease_||(policy!=QueryPolicy::Serialized&&policy!=QueryPolicy::Concurrent))throw std::invalid_argument("Explicit online module/code lease and valid query policy required");}
Binding::~Binding()noexcept{
    Error error;std::shared_ptr<void> retired,retired_object;CodeLease retired_factory;bool unresolved;
    {std::lock_guard lock(gate_);unresolved=active_||queries_||object_||object_unknown_;
     if(!unresolved){active_=true;procedure_=factory_=0;retired=std::move(code_);retired_object=std::move(factory_code_);}}
    if(unresolved)std::abort(); // no metadata lock or owner release at fail-stop
    // Both code and factory captures die while metadata gate is still alive,
    // outside its lock and with managed reentry still denied.
    retired.reset();retired_object.reset();retired_factory.swap(lease_);retired_factory={};
}
Snapshot Binding::snapshot()const{Error error;std::lock_guard lock(gate_);return {module_,procedure_,active_||queries_!=0,poisoned_,static_cast<bool>(code_),factory_,object_,object_unknown_,static_cast<bool>(factory_code_)};}
Module Binding::load(const std::function<Module()>& lower){
    Error error;if(!lower)throw std::invalid_argument("Explicit online load required");
    {std::lock_guard lock(gate_);if(active_||queries_||poisoned_||module_)throw std::logic_error("Online version load is not empty/quiescent");active_=true;}
    try{
        error.restore();
        const auto module=lower();error.capture();
        {std::lock_guard lock(gate_);module_=module;active_=false;}
        return module;
    }catch(...){error.capture();std::lock_guard lock(gate_);active_=false;poisoned_=true;throw;}
}
Procedure Binding::resolve(Module module,const std::function<Procedure()>& lower){
    Error error;std::shared_ptr<void> retired;
    if(!lower)throw std::invalid_argument("Explicit version lookup required");
    {std::lock_guard lock(gate_);
     if(active_||queries_||poisoned_||object_||object_unknown_||!module||module!=module_)throw std::logic_error("Version lookup requires exact loaded quiescent online module");
     active_=true;procedure_=0;retired=std::move(code_);}
    bool lookup_returned=false;
    try{
        error.restore();
        const auto procedure=lower();error.capture();lookup_returned=true;
        std::shared_ptr<void> code;
        if(procedure){code=lease_(module);if(!code)throw std::invalid_argument("Online version lease returned null");}
        {std::lock_guard lock(gate_);procedure_=procedure;code_=std::move(code);}
        retired.reset();
        {std::lock_guard lock(gate_);active_=false;}
        return procedure;
    }catch(...){if(!lookup_returned)error.capture();retired.reset();std::lock_guard lock(gate_);active_=false;throw;}
}
std::int32_t Binding::call(Procedure procedure){
    Error error;VersionFunction function;
    {std::lock_guard lock(gate_);
     if(active_||queries_||poisoned_||!module_||!procedure||procedure!=procedure_||!code_)throw std::logic_error("Version call requires exact live resolved procedure");
     active_=true;function=reinterpret_cast<VersionFunction>(procedure_);}
    try{
        error.restore();
        const auto result=function();error.capture();
        {std::lock_guard lock(gate_);active_=false;}
        return result;
    }catch(...){error.capture();std::lock_guard lock(gate_);active_=false;throw;}
}
bool Binding::free(Module module,const std::function<bool()>& lower){
    Error error;std::shared_ptr<void> retired,retired_object;
    if(!lower)throw std::invalid_argument("Explicit online free required");
    {std::lock_guard lock(gate_);
     if(active_||queries_||poisoned_||object_||object_unknown_||!module||module!=module_)throw std::logic_error("Online free requires exact quiescent module without live/unknown object");
     active_=true;procedure_=factory_=0;}
    try{
        error.restore();
        const bool result=lower();error.capture();
        {std::lock_guard lock(gate_);module_=0;retired=std::move(code_);retired_object=std::move(factory_code_);}
        retired.reset();retired_object.reset();
        {std::lock_guard lock(gate_);active_=false;}
        return result;
    }catch(...){error.capture();std::lock_guard lock(gate_);active_=false;poisoned_=true;throw;}
}
static_assert(sizeof(void*)==8&&sizeof(std::int32_t)==4);
} // namespace anadius::online_version
