#pragma once
#include "../anadius-crt-heap-native-source/native_api.hpp"
#include "../anadius-crt-recalloc-source/recalloc.hpp"
#include "../anadius-crt-error-source/errors.hpp"
namespace anadius::crt_heap_native_bridge {
// Required rebuilt policies. No original image address is invoked here.
class Policy {
public:
    virtual ~Policy()=default;
    virtual void lock_0625f8(std::uint32_t)=0;
    virtual void unlock_06264c(std::uint32_t)=0;
    virtual std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t)=0;
    virtual void invalid_parameter_05c954()=0;
};
// Borrowed dependencies must outlive every recovered algorithm call.
// This supplies byte-array storage only; typed PTD/Context lifetimes remain
// the caller's responsibility. Positive extents above PTRDIFF_MAX are outside
// the adapter domain and rejected before any native allocation/reallocation.
class Effects final:public crt_recalloc_recovered::Effects {
public:
    Effects(crt_heap_native::Calls&,crt_errors_recovered::State&,
            crt_errors_recovered::Effects&,Policy&);
    crt_heap_recovered::Heap get_process_heap() final;
    void* heap_alloc(crt_heap_recovered::Heap,std::uint32_t,std::uint64_t) final;
    bool heap_free(crt_heap_recovered::Heap,std::uint32_t,void*) final;
    void* heap_realloc(crt_heap_recovered::Heap,std::uint32_t,void*,std::uint64_t) final;
    std::uint64_t heap_size(crt_heap_recovered::Heap,std::uint32_t,void*) final;
    std::uint32_t last_error() final;
    std::uint32_t errno_from_os_error_05d650(std::uint32_t) final;
    void write_error_slot_05d724(std::uint32_t) final;
    void lock_0625f8(std::uint32_t) final;
    void unlock_06264c(std::uint32_t) final;
    std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t) final;
    void invalid_parameter_05c954() final;
    void zero_bytes_0793d0(void*,std::uint64_t) final;
private:
    crt_heap_native::Calls& native_;
    crt_errors_recovered::State& errors_;
    crt_errors_recovered::Effects& error_ptd_;
    Policy& policy_;
};
}
