#include "crt_glue.hpp"
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace anadius::crt_glue {
FlsApi native_fls_api() noexcept {
    return {&::FlsGetValue,&::FlsSetValue,&::FlsAlloc,&::FlsFree,&::GetLastError,&::SetLastError};
}
PtdEffects::PtdEffects(FlsApi api,PFLS_CALLBACK_FUNCTION callback,
    crt_heap_recovered::State& heap,crt_heap_recovered::Effects& heap_effects,
    const crt_ptd_data_recovered::Identities& ids,crt_ptd_data_recovered::Effects& data,
    AbortPolicy& abort)
    :api_(api),callback_(callback),heap_(heap),heap_effects_(heap_effects),ids_(ids),data_(data),abort_(abort) {
    if(!api_.get||!api_.set||!api_.allocate||!api_.release||!api_.error||!api_.set_error||!callback_)
        throw std::invalid_argument("Complete FLS API and rebuilt destructor callback required");
}
crt_ptd_recovered::Ptd* PtdEffects::fls_get_value(std::uint32_t index) {
    return static_cast<crt_ptd_recovered::Ptd*>(api_.get(static_cast<DWORD>(index)));
}
bool PtdEffects::fls_set_value(std::uint32_t index,crt_ptd_recovered::Ptd* value) {
    return api_.set(static_cast<DWORD>(index),value)!=FALSE;
}
std::uint32_t PtdEffects::fls_alloc(crt_ptd_recovered::FlsCallback callback) {
    if(callback!=crt_ptd_recovered::FlsCallback::destroy_0649cc)
        throw std::invalid_argument("Unknown recovered FLS destructor");
    return static_cast<std::uint32_t>(api_.allocate(callback_));
}
bool PtdEffects::fls_free(std::uint32_t index) { return api_.release(static_cast<DWORD>(index))!=FALSE; }
std::uint32_t PtdEffects::last_error() { return static_cast<std::uint32_t>(api_.error()); }
void PtdEffects::set_last_error(std::uint32_t error) { api_.set_error(static_cast<DWORD>(error)); }
crt_ptd_recovered::Ptd* PtdEffects::calloc_ptd_066848() {
    auto* storage=crt_heap_recovered::calloc_base_066848(1,sizeof(crt_ptd_recovered::Ptd),heap_,heap_effects_);
    if(!storage)return nullptr;
    // New C++ lifetime bridge for trivial records on the observed zeroed heap.
    return std::construct_at(static_cast<crt_ptd_recovered::Ptd*>(storage));
}
void PtdEffects::free_0668c0(crt_ptd_recovered::Ptd* value) {
    crt_heap_recovered::free_0668c0(value,heap_,heap_effects_);
}
void PtdEffects::construct_ptd_array_0648fc(crt_ptd_recovered::Ptd* value) {
    crt_ptd_data_recovered::construct_0648fc(value,ids_,data_);
}
void PtdEffects::destroy_ptd_array_0649ec(crt_ptd_recovered::Ptd* value) {
    crt_ptd_data_recovered::destroy_0649ec(value,ids_,data_);
}
[[noreturn]] void PtdEffects::abort_05f03c() { abort_.abort_05f03c(); }
ErrorPtd::ErrorPtd(crt_ptd_recovered::State& state,crt_ptd_recovered::Effects& effects):state_(state),effects_(effects){}
void* ErrorPtd::optional_ptd_064d08() { return crt_ptd_recovered::optional_preserve_error_064d08(state_,effects_); }
HeapErrors::HeapErrors(crt_errors_recovered::State& state,crt_errors_recovered::Effects& effects):state_(state),effects_(effects){}
std::uint32_t HeapErrors::errno_from_os_error_05d650(std::uint32_t error) {
    return crt_errors_recovered::errno_from_os_error_05d650(error);
}
void HeapErrors::write_error_slot_05d724(std::uint32_t error) {
    crt_errors_recovered::errno_cell_05d724(state_,effects_).store(error);
}
InvalidPtd::InvalidPtd(crt_ptd_recovered::State& state,crt_ptd_recovered::Effects& effects):state_(state),effects_(effects){}
std::uint32_t InvalidPtd::last_error() { return effects_.last_error(); }
void InvalidPtd::set_last_error(std::uint32_t error) { effects_.set_last_error(error); }
void* InvalidPtd::optional_ptd_064dd0(std::uint32_t*,std::uint64_t index) {
    if(index!=0)throw std::out_of_range("Only recovered PTD state index0 is bound");
    return crt_ptd_recovered::optional_raw_state0_064dd0(state_,effects_);
}
ThreadEffects::ThreadEffects(LPTHREAD_START_ROUTINE start,thread_native::Api api,
    crt_runtime_recovered::State& runtime,crt_runtime_recovered::Effects& runtime_effects,
    crt_errors_recovered::State& errors,crt_errors_recovered::Effects& error_effects,Dependencies deps)
    :RecoveredWin32Effects(start,api,runtime,runtime_effects,errors,error_effects),deps_(deps){}
threads_recovered::Context* ThreadEffects::calloc_context_066848() {
    auto* storage=crt_heap_recovered::calloc_base_066848(1,sizeof(threads_recovered::Context),deps_.heap,deps_.heap_effects);
    if(!storage)return nullptr;
    return std::construct_at(static_cast<threads_recovered::Context*>(storage));
}
void ThreadEffects::free_0668c0(threads_recovered::Context* value) {
    crt_heap_recovered::free_0668c0(value,deps_.heap,deps_.heap_effects);
}
void ThreadEffects::invalid_parameter_05c954() {
    crt_invalid_recovered::invalid_parameter_noinfo_05c954(deps_.invalid);
}
threads_recovered::Context*& ThreadEffects::required_thread_slot_064b90() {
    return crt_ptd_recovered::required_preserve_error_064b90(deps_.ptd,deps_.ptd_effects)->thread_context;
}
threads_recovered::Context* ThreadEffects::optional_thread_context_064d08() {
    auto* ptd=crt_ptd_recovered::optional_preserve_error_064d08(deps_.ptd,deps_.ptd_effects);
    return ptd?ptd->thread_context:nullptr;
}
std::uint32_t ThreadEffects::invoke_entry(std::uintptr_t entry,std::uintptr_t argument) {
    using Application=unsigned (__stdcall*)(void*);
    return reinterpret_cast<Application>(entry)(reinterpret_cast<void*>(argument));
}
static_assert(std::is_trivially_destructible_v<crt_ptd_recovered::Ptd>);
static_assert(std::is_trivially_destructible_v<threads_recovered::Context>);
static_assert(!std::is_abstract_v<ThreadEffects>);
}
