#include "factory.hpp"
#include <stdexcept>

namespace anadius::thread_domain {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
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
