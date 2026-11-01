#include "bridge.hpp"
#include <limits>
#include <new>
#include <stdexcept>
namespace anadius::crt_heap_native_bridge {
namespace {
void require_extent(std::uint64_t bytes){
    if(bytes>static_cast<std::uint64_t>(std::numeric_limits<std::ptrdiff_t>::max()))
        throw std::domain_error("native heap byte-array extent exceeds PTRDIFF_MAX");
}
void* start_bytes(void* storage,std::uint64_t bytes){
    // Non-allocating placement array new has no cookie/offset. Default
    // initialization of std::byte does not overwrite the preserved contents.
    return storage?::new(storage) std::byte[static_cast<std::size_t>(bytes)]:nullptr;
}
}
Effects::Effects(crt_heap_native::Calls& n,crt_errors_recovered::State& s,crt_errors_recovered::Effects& e,Policy& p):native_(n),errors_(s),error_ptd_(e),policy_(p){}
crt_heap_recovered::Heap Effects::get_process_heap(){return native_.get_process_heap();}
void* Effects::heap_alloc(crt_heap_recovered::Heap h,std::uint32_t f,std::uint64_t n){require_extent(n);return start_bytes(native_.alloc(h,f,n),n);}
bool Effects::heap_free(crt_heap_recovered::Heap h,std::uint32_t f,void* p){return native_.free(h,f,p);}
void* Effects::heap_realloc(crt_heap_recovered::Heap h,std::uint32_t f,void* p,std::uint64_t n){require_extent(n);return start_bytes(native_.realloc(h,f,p,n),n);}
std::uint64_t Effects::heap_size(crt_heap_recovered::Heap h,std::uint32_t f,void* p){return native_.size(h,f,p);}
std::uint32_t Effects::last_error(){return native_.last_error();}
std::uint32_t Effects::errno_from_os_error_05d650(std::uint32_t e){return crt_errors_recovered::errno_from_os_error_05d650(e);}
void Effects::write_error_slot_05d724(std::uint32_t e){crt_errors_recovered::errno_cell_05d724(errors_,error_ptd_).store(e);}
void Effects::lock_0625f8(std::uint32_t i){policy_.lock_0625f8(i);}
void Effects::unlock_06264c(std::uint32_t i){policy_.unlock_06264c(i);}
std::int32_t Effects::invoke_new_handler(std::uintptr_t a,std::uint64_t n){return policy_.invoke_new_handler(a,n);}
void Effects::invalid_parameter_05c954(){policy_.invalid_parameter_05c954();}
void Effects::zero_bytes_0793d0(void* p,std::uint64_t n){require_extent(n);if(n)std::memset(p,0,static_cast<std::size_t>(n));}
}
