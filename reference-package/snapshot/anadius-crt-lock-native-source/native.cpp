#include "native.hpp"
#include <exception>
#include <stdexcept>
#include <utility>

namespace anadius::crt_lock_native {
const Api& native_api() noexcept {
    static const Api api{&::InitializeCriticalSectionAndSpinCount,
        &::DeleteCriticalSection,&::EnterCriticalSection,&::LeaveCriticalSection};
    return api;
}
Banks::Banks(Api api):api_(api) {
    if(!api_.initialize_spin||!api_.destroy||!api_.enter||!api_.leave)
        throw std::invalid_argument("Incomplete native critical-section API");
}
Banks::~Banks() {
    for(const auto& c:acrt_)if(c.initialized)std::terminate();
    for(const auto& c:vcrt_)if(c.initialized)std::terminate();
}
const Banks::Cell& Banks::cell(LockRef ref) const {
    using Bank=crt_locks_recovered::Bank;
    if(ref.index<0)throw std::out_of_range("Negative critical-section bank index");
    if(ref.bank==Bank::acrt&&static_cast<std::uint64_t>(ref.index)<acrt_.size())
        return acrt_[static_cast<std::size_t>(ref.index)];
    if(ref.bank==Bank::vcrt&&static_cast<std::uint64_t>(ref.index)<vcrt_.size())
        return vcrt_[static_cast<std::size_t>(ref.index)];
    throw std::out_of_range("Critical-section bank or index out of range");
}
Banks::Cell& Banks::cell(LockRef ref) {
    return const_cast<Cell&>(std::as_const(*this).cell(ref));
}
Banks::Cell& Banks::cell_at(void* pointer) {
    for(auto& c:acrt_)if(pointer==&c.section)return c;
    for(auto& c:vcrt_)if(pointer==&c.section)return c;
    throw std::invalid_argument("Pointer is not an exact owned critical-section object");
}
void Banks::require_fresh(const Cell& c) {
    if(c.initialized)throw std::logic_error("Critical section is already initialized");
}
void Banks::require_initialized(const Cell& c) {
    if(!c.initialized)throw std::logic_error("Critical section is not initialized");
}
void* Banks::storage(LockRef ref){return &cell(ref).section;}
bool Banks::initialized(LockRef ref) const{return cell(ref).initialized;}
std::int32_t Banks::init_spin_at(void* pointer,std::uint32_t spin) {
    auto& c=cell_at(pointer);
    require_fresh(c);
    const BOOL result=api_.initialize_spin(&c.section,spin);
    if(result!=0)c.initialized=true;
    return static_cast<std::int32_t>(result);
}
std::int32_t Banks::init_ex_at(Address function,void* pointer,std::uint32_t spin,std::uint32_t flags) {
    auto& c=cell_at(pointer);
    require_fresh(c);
    if(function==0)throw std::invalid_argument("Null native InitializeCriticalSectionEx procedure");
    const auto call=reinterpret_cast<decltype(&::InitializeCriticalSectionEx)>(function);
    const BOOL result=call(&c.section,spin,flags);
    if(result!=0)c.initialized=true;
    return static_cast<std::int32_t>(result);
}
std::uint64_t Banks::initialize_and_spin(LockRef ref,std::uint32_t spin) {
    return static_cast<std::uint32_t>(init_spin_at(storage(ref),spin));
}
std::uint64_t Banks::call_initialize_ex(Address function,LockRef ref,std::uint32_t spin,std::uint32_t flags) {
    return static_cast<std::uint32_t>(init_ex_at(function,storage(ref),spin,flags));
}
void Banks::destroy(LockRef ref) {
    auto& c=cell(ref);require_initialized(c);
    api_.destroy(&c.section);
    c.initialized=false;
}
void Banks::enter(LockRef ref) {
    auto& c=cell(ref);require_initialized(c);api_.enter(&c.section);
}
void Banks::leave(LockRef ref) {
    auto& c=cell(ref);require_initialized(c);api_.leave(&c.section);
}
static_assert(sizeof(BOOL)==sizeof(std::int32_t));
static_assert(sizeof(CRITICAL_SECTION)==40,"Recovered Win64 bank assumes 40-byte SDK CS");
}
