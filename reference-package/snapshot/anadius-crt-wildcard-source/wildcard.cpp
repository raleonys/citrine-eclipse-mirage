#include "wildcard.hpp"
#include <bit>
#include <stdexcept>

namespace anadius::crt_wildcard_recovered {
Address first_wildcard(Address input,Effects& effects) {
    for(auto current=input;;++current) {
        const auto value=effects.read_u8(current);
        if(value==0)return 0;
        if(value=='*'||value=='?')return current;
    }
}
std::uint32_t expand_06e5bc(Address argv,Result* output,Effects& effects) {
    if(output==nullptr) {
        effects.errno_cell_05d724().store(22);
        effects.invalid_parameter_05c954();
        return 22;
    }
    *output={};
    crt_wildcard_storage::Vector vector{};
    for(auto slot=argv;;slot+=8) {
        const auto argument=effects.read_word(slot);
        if(argument==0)break;
        const auto wildcard=first_wildcard(argument,effects);
        const auto status=wildcard?
            crt_wildcard_search::search_06e984(argument,wildcard,vector,effects):
            crt_wildcard_storage::append_06e800(argument,0,0,vector,effects);
        if(status!=0) {
            const auto begin=vector.begin;
            for(auto current=begin;current!=vector.end;current+=8)
                effects.free_0668c0(effects.read_word(current));
            effects.free_0668c0(begin);
            return status;
        }
    }
    // Snapshot source range before the final allocation/copy/cleanup effects.
    const auto begin=vector.begin,end=vector.end;
    const auto pointers=(end-begin)/8+1;
    std::uint64_t characters=0;
    for(auto current=begin;current!=end;current+=8)
        characters+=effects.length(effects.read_word(current))+1;
    const auto allocation=effects.allocate_buffer_063888(pointers,characters,1);
    if(allocation==0) {
        effects.free_0668c0(0);
        for(auto current=begin;current!=end;current+=8)
            effects.free_0668c0(effects.read_word(current));
        effects.free_0668c0(begin);
        return 0xffffffffU;
    }
    const auto text_begin=allocation+pointers*8;
    auto text=text_begin;
    for(auto current=begin;current!=end;current+=8) {
        const auto source=effects.read_word(current);
        const auto bytes=effects.length(source)+1;
        if(effects.memcpy_s_0767fc(text,text_begin-text+characters,source,bytes)!=0)
            effects.watson_05c9a4();
        effects.write_word(allocation+(current-begin),text);
        text+=bytes;
    }
    output->base=allocation;
    output->pointer_slots=pointers;
    effects.free_0668c0(0);
    for(auto current=begin;current!=end;current+=8)
        effects.free_0668c0(effects.read_word(current));
    effects.free_0668c0(begin);
    return 0;
}
std::uint32_t thunk_expand_06ecd4(Address argv,Result* output,Effects& effects) {
    return expand_06e5bc(argv,output,effects);
}
Address previous_locale_077170(Address start,Address position,Address locale,PreviousEffects& effects) {
    if(start==0||position==0) {
        effects.errno_cell_05d724().store(22);
        effects.invalid_parameter_05c954();
        return 0;
    }
    if(start>=position)return 0;
    const auto context=effects.locale_update_058b04(locale);
    auto previous=position-1;
    if(effects.read_u32(context.multibyte_info+8)!=0) {
        do {
            --previous;
            if(previous<start)break;
        }while((effects.read_u8(context.multibyte_info+0x19+effects.read_u8(previous))&4)!=0);
        previous=position-1-((position-previous)&1);
    }
    if(context.clear_thread_flag!=0) {
        const auto address=context.ptd+0x3a8;
        effects.write_u32(address,effects.read_u32(address)&0xfffffffdU);
    }
    return previous;
}
Address previous_077168(Address start,Address position,PreviousEffects& effects) {
    return previous_locale_077170(start,position,0,effects);
}
std::uint32_t select_code_page_region(PreviousEffects& effects) {
    const auto context=effects.locale_update_058b04(0);
    auto code_page=effects.read_u32(context.locale_info+0xc);
    if(code_page!=65001)code_page=effects.file_api_ansi_066c1c()!=0?0U:1U;
    if(context.clear_thread_flag!=0) {
        const auto address=context.ptd+0x3a8;
        effects.write_u32(address,effects.read_u32(address)&0xfffffffdU);
    }
    return code_page;
}
Address SupportEffects::previous_077168(Address start,Address position) {
    return crt_wildcard_recovered::previous_077168(start,position,*this);
}
std::uint32_t SupportEffects::choose_code_page(){return select_code_page_region(*this);}
void SupportEffects::sort_new_segment(Address begin,std::uint64_t count) {
    // New opaque rebuilt comparator identity; never a callable original VA.
    crt_sort_recovered::qsort_0768f0(begin,count,8,1,*this);
}
std::int32_t SupportEffects::compare(Address comparator,Address lhs,Address rhs) {
    if(comparator!=1)throw std::domain_error("Unsupported rebuilt wildcard comparator binding");
    return crt_sort_recovered::compare_slot_addresses_06e41c(lhs,rhs);
}
std::int32_t ExpansionPolicy::expand_06ecd4(std::span<char*> input,crt_argv_recovered::Expansion& output) {
    Result result;
    const auto status=thunk_expand_06ecd4(reinterpret_cast<Address>(input.data()),&result,effects_);
    output.base=reinterpret_cast<char**>(result.base);
    output.pointers={output.base,static_cast<std::size_t>(result.pointer_slots)};
    return std::bit_cast<std::int32_t>(status);
}
}
