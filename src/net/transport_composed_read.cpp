// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_composed_read.hpp"
#include <algorithm>
#include <bit>
#include <limits>

namespace preloader_l::socket {
namespace {
constexpr auto npos=std::numeric_limits<std::uint64_t>::max();
ReceiveBufferString& string_at(std::uintptr_t identity,ComposedReadContext& context){
    ReceiveBufferString* found=nullptr;bool matched=false;
    for(const auto& item:context.strings){
        if(item.identity!=identity)continue;
        if(matched)throw UnsupportedInput("Composed receive string identity is ambiguous");
        matched=true;found=item.string;
    }
    if(!found)throw UnsupportedInput("Composed receive string has no live projection");
    return *found;
}
ReceiveStreamBinding& stream_at(std::uintptr_t identity,ComposedReadContext& context){
    ReceiveStreamBinding* found=nullptr;
    for(auto& item:context.streams){
        if(item.identity!=identity)continue;
        if(found)throw UnsupportedInput("Composed receive stream identity is ambiguous");
        found=&item;
    }
    if(!found || !found->service || !found->implementation)
        throw UnsupportedInput("Composed receive stream members have no live projection");
    return *found;
}
std::uint64_t effective(const ReceiveHandler& h,const ReceiveBufferString& string){
    return h.words_60_78[2]==npos?std::min(string.size_10,h.words_60_78[3]):h.words_60_78[2];
}
std::uint64_t iterator_start(std::uint64_t delta,std::uint64_t length){
    // Native generic buffer iterator is instantiated with exactly one buffer.
    // Positive advance consumes at most that buffer; negative advance from
    // begin clamps at begin. Empty-buffer nonzero advance has no progressing
    // normal path in the retained generic code and is outside this projection.
    if(!length && delta)throw UnsupportedInput("Nonzero cursor on empty receive range has no progressing normal projection");
    const auto signed_delta=std::bit_cast<std::int64_t>(delta);
    if(signed_delta<=0)return 0;
    return std::bit_cast<std::int64_t>(length)>signed_delta?delta:length;
}
void finish(ReceiveHandler& h,const Error& incoming,ComposedReadContext& context){
    Error copy;
    if(h.words_88_98[0]==npos)copy={3,context.completion.misc_category_0285b0()};
    else copy=incoming;
    // Incoming code and result are reloaded after the category callback.
    const auto result=incoming.value!=0 || h.words_88_98[0]==npos?0:h.words_88_98[0];
    context.api.finish_frame_025480(h,copy,result);
}
}
void composed_receive_025c40(ReceiveHandler& h,const Error& incoming,std::uint64_t bytes,std::uint32_t flag,
    ComposedReadContext& context,ComposedReadStart call_021d10){
    h.dword_84=flag;
    if(flag!=1){
        auto& string=string_at(static_cast<std::uintptr_t>(h.words_60_78[1]),context);
        const auto unused=h.words_88_98[1]-bytes; // unsigned64 subtraction
        const auto current=effective(h,string);
        resize_receive_buffer_010040(string,current<unused?0:current-unused,0,context.buffer);
        if(incoming.value!=0 || bytes==0){finish(h,incoming,context);return;}
    }
    auto& string=string_at(static_cast<std::uintptr_t>(h.words_60_78[1]),context);
    const auto initial_effective=h.words_60_78[2];
    const auto effective_count=effective(h,string);
    const auto maximum=h.words_60_78[3];
    const auto visible=std::min(string.size_10,maximum);
    const auto length=std::min(effective_count,visible);
    const auto base=string.size_10?receive_buffer_address(string):0;
    auto position=iterator_start(h.words_88_98[0],length);
    const auto delimiter=h.byte_80;
    while(position!=length && context.buffer.memory.view(base+static_cast<std::uintptr_t>(position),1)[0]!=delimiter)++position;
    std::uint64_t request=0;
    if(position!=length)h.words_88_98[0]=position+std::uint64_t{1};
    else{
        const auto resolved=initial_effective==npos?std::min(string.size_10,maximum):initial_effective;
        if(resolved==h.words_60_78[3])h.words_88_98[0]=npos;
        else{
            h.words_88_98[0]=length;
            const auto remaining=h.words_60_78[3]-effective(h,string);
            const auto spare=std::min(string.capacity_18,h.words_60_78[3])-effective(h,string);
            request=std::min(std::min(remaining,std::uint64_t{65536}),std::max(spare,std::uint64_t{512}));
        }
    }
    h.words_88_98[1]=request;
    if(flag==0 && request==0){finish(h,incoming,context);return;}
    const auto offset_before_prepare=effective(h,string);
    const auto maximum_before_prepare=h.words_60_78[3];
    if(offset_before_prepare>maximum_before_prepare || request>maximum_before_prepare-offset_before_prepare){
        ReadLengthError exception;
        context.api.construct_length_error_024390(exception,0x180084840ULL);
        context.api.throw_length_error_023930(exception);
    }
    resize_receive_buffer_010040(string,effective(h,string)+request,0,context.buffer);
    // Native reloads stream, requested length, string identity and maximum
    // after resize. The old effective offset remains cached across allocation.
    const auto stream_identity=static_cast<std::uintptr_t>(h.words_60_78[0]);
    const auto live_request=h.words_88_98[1];
    auto& live_string=string_at(static_cast<std::uintptr_t>(h.words_60_78[1]),context);
    const auto live_visible=std::min(live_string.size_10,h.words_60_78[3]);
    const auto live_base=live_string.size_10?receive_buffer_address(live_string):0;
    const auto offset=std::min(offset_before_prepare,live_visible);
    ReceiveBufferSlice slice{live_base+static_cast<std::uintptr_t>(offset),std::min(live_visible-offset,live_request)};
    auto& stream=stream_at(stream_identity,context);
    if(!call_021d10)throw UnsupportedInput("Composed receive callee is not bound");
    call_021d10(*stream.service,*stream.implementation,slice,0,h,stream_identity+std::uintptr_t{0x68},context.receive);
}
void ComposedReadEffects::receive_body_025c40(ReceiveHandler& h,Error& error,std::uint64_t bytes,std::uint32_t flag){
    if(!read_ || &read_->completion!=this)throw UnsupportedInput("Composed receive context must be bound to these completion effects");
    composed_receive_025c40(h,error,bytes,flag,*read_);
}
} // namespace preloader_l::socket
