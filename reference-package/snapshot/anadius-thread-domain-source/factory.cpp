#include "factory.hpp"
#include "../anadius-crt-cache-source/cache.hpp"
#include <stdexcept>

namespace anadius::thread_domain {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
class NativeEntropy final:public crt_cache_recovered::EntropyEffects {
    EntropyImports api_;
    std::uint64_t stack_cell_=0;
public:
    explicit NativeEntropy(EntropyImports api):api_(api){}
    void system_time_as_file_time(std::uint64_t& value)override{
        FILETIME time{};api_.time(&time);
        value=static_cast<std::uint64_t>(time.dwLowDateTime)|(static_cast<std::uint64_t>(time.dwHighDateTime)<<32);
    }
    std::uint32_t current_thread_id()override{return api_.thread();}
    std::uint32_t current_process_id()override{return api_.process();}
    crt_cache_recovered::CounterObservation query_performance_counter()override{
        LARGE_INTEGER counter;
        if(!api_.counter(&counter))return {false,std::nullopt};
        return {true,static_cast<std::uint64_t>(counter.QuadPart)};
    }
    std::uintptr_t entropy_stack_cell_address()override{return reinterpret_cast<std::uintptr_t>(&stack_cell_);}
};
struct Ownership {
    // Seed is Impl's first member, hence is destroyed after its dependents.
    std::shared_ptr<void> backing;
    struct OpaqueIdentity {} identity;
    explicit Ownership(std::shared_ptr<void> value):backing(std::move(value)){}
    ~Ownership(){Error error;backing.reset();}
};
bool complete(const Apis& a){
    const auto& h=a.heap;const auto& f=a.fls;const auto& l=a.locks;const auto& r=a.runtime;
    return h.get_process_heap&&h.heap_alloc&&h.heap_realloc&&h.heap_free&&h.heap_size&&h.get_last_error&&
        f.get&&f.set&&f.allocate&&f.release&&f.error&&f.set_error&&
        l.initialize_spin&&l.destroy&&l.enter&&l.leave&&
        r.load&&r.procedure&&r.release&&r.error&&r.process_flags;
}
}
InitialPolicy make_native_initial_policy(EntropyImports api){
    Error error;
    if(!api.time||!api.thread||!api.process||!api.counter)
        throw std::invalid_argument("Complete native initial-policy entropy imports required");
    NativeEntropy entropy(api);crt_cache_recovered::CookieState cookie;
    crt_cache_recovered::initialize_cookie_04c134(cookie,entropy);
    return {cookie.cookie,0,nullptr,nullptr,0};
}
std::shared_ptr<DomainStorage> make_initial_domain(InitialInputs& in){
    Error error;
    if(in.consumed_)throw std::logic_error("Initial Domain inputs already consumed");
    if(!complete(in.apis))throw std::invalid_argument("Complete initial Domain SDK tables required");
    in.consumed_=true;
    auto owner=std::make_shared<Ownership>(std::move(in.backing));
    const auto& p=in.policy;
    Seed seed{p.security_cookie,p.new_mode,p.new_handler,p.invalid_handler,p.thread_policy,
        reinterpret_cast<std::uintptr_t>(&owner->identity),owner};
    return DomainStorage::create(std::move(seed),in.apis);
}
}
