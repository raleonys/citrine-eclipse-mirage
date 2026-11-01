#include "fls.hpp"
#include <exception>
#include <stdexcept>
namespace anadius::crt_fls_native {
namespace p=crt_ptd_recovered;
std::atomic<Binding*> Binding::active_{nullptr};
Binding::Binding(crt_heap_recovered::State& h,crt_heap_recovered::Effects& heap,
    crt_ptd_data_recovered::Identities ids,crt_ptd_data_recovered::Effects& data,
    crt_glue::AbortPolicy& abort,crt_glue::FlsApi api)
    :ids_(ids),data_(data),effects_(api,&callback,h,heap,ids_,data_,abort){
    Binding* expected=nullptr;
    if(!active_.compare_exchange_strong(expected,this))
        throw std::logic_error("native FLS callback domain already bound");
}
Binding::~Binding(){
    // Do not abandon a live native slot just because recovered State was reset.
    if(registered_!=p::invalid_fls_index||state_.fls_index!=p::invalid_fls_index)
        std::terminate();
    Binding* expected=this;
    if(!active_.compare_exchange_strong(expected,nullptr))std::terminate();
}
VOID WINAPI Binding::callback(PVOID value)noexcept{
    auto* self=active_.load();
    if(!self||value==p::construction_sentinel())std::terminate();
    try{crt_ptd_data_recovered::fls_destroy_0649cc(value,self->ids_,self->data_);}
    catch(...){std::terminate();}
}
bool Binding::retry_release(){
    if(state_.fls_index!=p::invalid_fls_index)
        throw std::logic_error("run recovered uninitialize before native release retry");
    return registered_==p::invalid_fls_index||fls_free(registered_);
}
p::Ptd* Binding::fls_get_value(std::uint32_t i){return effects_.fls_get_value(i);}
bool Binding::fls_set_value(std::uint32_t i,p::Ptd* v){return effects_.fls_set_value(i,v);}
std::uint32_t Binding::fls_alloc(p::FlsCallback c){
    if(registered_!=p::invalid_fls_index)throw std::logic_error("native FLS slot already owned");
    registered_=effects_.fls_alloc(c);return registered_;
}
bool Binding::fls_free(std::uint32_t i){
    if(i==p::invalid_fls_index||i!=registered_)throw std::logic_error("native FLS release identity mismatch");
    const auto released=effects_.fls_free(i);
    if(released)registered_=p::invalid_fls_index;
    return released;
}
std::uint32_t Binding::last_error(){return effects_.last_error();}
void Binding::set_last_error(std::uint32_t e){effects_.set_last_error(e);}
p::Ptd* Binding::calloc_ptd_066848(){return effects_.calloc_ptd_066848();}
void Binding::free_0668c0(p::Ptd* v){effects_.free_0668c0(v);}
void Binding::construct_ptd_array_0648fc(p::Ptd* v){effects_.construct_ptd_array_0648fc(v);}
void Binding::destroy_ptd_array_0649ec(p::Ptd* v){effects_.destroy_ptd_array_0649ec(v);}
[[noreturn]]void Binding::abort_05f03c(){effects_.abort_05f03c();}
}
