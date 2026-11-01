#include "reference.hpp"
#include <cstdlib>
#include <stdexcept>
#include <system_error>

namespace anadius::module_reference {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
struct Provider {
    Api api;std::shared_ptr<void> backing;
    Provider(Api value,std::shared_ptr<void> owner):api(value),backing(std::move(owner)){}
    ~Provider(){Error error;backing.reset();}
};
struct Reference {
    std::shared_ptr<Provider> provider;HMODULE module=nullptr;
    explicit Reference(std::shared_ptr<Provider> value):provider(std::move(value)){}
    ~Reference()noexcept {
        Error error;
        if(module){
            try{if(!provider->api.release(module))std::abort();}
            catch(...){std::abort();}
        }
        provider.reset();
    }
};
}
online_version::CodeLease make_code_lease(Api api,std::shared_ptr<void> backing){
    Error error;
    if(!api.acquire||!api.release)throw std::invalid_argument("Complete module-reference SDK required");
    auto provider=std::make_shared<Provider>(api,std::move(backing));
    return [provider=std::move(provider)](online_version::Module module)->std::shared_ptr<void>{
        Error saved;
        if(!module)throw std::invalid_argument("Stable loaded module base required");
        // Allocate both object and control block BEFORE the OS acquires a ref.
        auto reference=std::make_shared<Reference>(provider);
        HMODULE owned=nullptr;BOOL result;
        try{result=provider->api.acquire(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                 reinterpret_cast<LPCWSTR>(module),&owned);}
        catch(...){std::abort();} // reference-count outcome is unknown
        const DWORD failure=::GetLastError();
        if(!result){
            if(owned)std::abort(); // violates Win32 failure contract
            throw std::system_error(static_cast<int>(failure),std::system_category(),"GetModuleHandleExW");
        }
        // No allocation or opaque call between successful acquire and adoption.
        reference->module=owned;
        if(!owned||reinterpret_cast<online_version::Module>(owned)!=module)std::abort();
        return reference;
    };
}
}
