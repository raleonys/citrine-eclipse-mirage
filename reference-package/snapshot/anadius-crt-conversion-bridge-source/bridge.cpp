#include "bridge.hpp"
namespace anadius::crt_conversion_bridge {
BufferEffects::BufferEffects(crt_heap_recovered::State& h,crt_realloc_recovered::Effects& hf,
    crt_errors_recovered::State& e,crt_errors_recovered::Effects& ef,
    MemoryEffects& m,crt_conversion_api::Effects& a)
    :heap_(h),heap_fx_(hf),errors_(e),error_fx_(ef),memory_(m),api_(a){}
std::uint8_t BufferEffects::read_u8(Address a){return memory_.read_u8(a);}
std::uint16_t BufferEffects::read_u16(Address a){return memory_.read_u16(a);}
void BufferEffects::write_zero_u8(Address a){memory_.write_zero_u8(a);}
void BufferEffects::write_zero_u16(Address a){memory_.write_zero_u16(a);}
Address BufferEffects::malloc_06a308(std::uint64_t bytes){
    return reinterpret_cast<Address>(crt_realloc_recovered::malloc_base_06a308(bytes,heap_,heap_fx_));
}
void BufferEffects::free_0668c0(Address a){crt_heap_recovered::free_0668c0(reinterpret_cast<void*>(a),heap_,heap_fx_);}
std::int32_t BufferEffects::multi_byte_to_wide_06ccc8(std::uint32_t cp,std::uint32_t flags,
    Address src,std::int32_t count,Address dst,std::int32_t capacity){
    return crt_conversion_api::multi_byte_to_wide_06ccc8(cp,flags,src,count,dst,capacity,api_);
}
std::int32_t BufferEffects::wide_char_to_multi_byte_06cd58(std::uint32_t cp,std::uint32_t flags,
    Address src,std::int32_t count,Address dst,std::int32_t capacity,Address def,Address used){
    return crt_conversion_api::wide_char_to_multi_byte_06cd58(cp,flags,src,count,dst,capacity,def,used,api_);
}
std::uint32_t BufferEffects::last_error(){return memory_.last_error();}
void BufferEffects::map_os_error_05d698(std::uint32_t error){crt_errors_recovered::map_os_error_05d698(error,errors_,error_fx_);}
crt_errors_recovered::Cell32 BufferEffects::errno_cell_05d724(){return crt_errors_recovered::errno_cell_05d724(errors_,error_fx_);}
WildcardEffects::WildcardEffects(crt_recalloc_bridge::Allocator& a,crt_conversion_recovered::Effects& c,
    crt_runtime_recovered::State& r,crt_runtime_recovered::Effects& rf,crt_conversion_api::FileApiEffects& f)
    :WildcardHeapEffects(a),conversion_(c),runtime_(r),runtime_fx_(rf),file_api_(f){}
crt_wildcard_search::Conversion WildcardEffects::to_wide_06004c(Address src,std::uint32_t cp){
    crt_conversion_recovered::Buffer buffer;
    const auto status=crt_conversion_recovered::to_wide_06004c(src,buffer,cp,conversion_);
    return{buffer.data,buffer.owned!=0,status};
}
crt_wildcard_search::Conversion WildcardEffects::to_narrow_06e430(Address src,std::uint32_t cp){
    crt_conversion_recovered::Buffer buffer;
    const auto status=crt_conversion_recovered::to_narrow_06e430(src,buffer,cp,conversion_);
    return{buffer.data,buffer.owned!=0,status};
}
std::uint32_t WildcardEffects::file_api_ansi_066c1c(){
    return crt_conversion_api::file_api_ansi_066c1c(runtime_,runtime_fx_,file_api_);
}
}
