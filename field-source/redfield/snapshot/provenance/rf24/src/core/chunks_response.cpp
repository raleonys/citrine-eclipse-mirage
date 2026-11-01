// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/chunks_response.hpp"
#include "redfield/handlers/integration_routes.hpp"
#include "stream_response_support.hpp"
namespace redfield { namespace handlers { namespace {
using redfield::StreamWriter;
using redfield::safe_attr;
bool parse_chunk_id(redfield::xml::TextSpan source,int32_t* value) noexcept {
    if(!value)return false;
    redfield::xml::SemanticValue semantic;
    if(semantic.assign_parsed(source)!=redfield::xml::ValueCodecStatus::ok)return false;
    const auto v=semantic.view();
    if(!v.data||v.size==0)return false;
    size_t i=0;bool negative=false;
    if(v.data[i]=='-'||v.data[i]=='+'){negative=v.data[i]=='-';if(++i==v.size)return false;}
    uint64_t magnitude=0;
    for(;i<v.size;++i){
        const unsigned char c=static_cast<unsigned char>(v.data[i]);
        if(c<'0'||c>'9')return false;
        magnitude=magnitude*10u+static_cast<uint64_t>(c-'0');
        const uint64_t limit=negative?UINT64_C(2147483648):UINT64_C(2147483647);
        if(magnitude>limit)return false;
    }
    *value=negative?static_cast<int32_t>(-static_cast<int64_t>(magnitude)):static_cast<int32_t>(magnitude);
    return true;
}

size_t i32_text(int32_t v,char* out) noexcept {
    uint32_t mag=v<0?static_cast<uint32_t>(-(int64_t)v):static_cast<uint32_t>(v); char tmp[16]{}; size_t n=0;
    do{tmp[n++]=static_cast<char>('0'+mag%10u);mag/=10u;}while(mag);
    size_t pos=0;if(v<0)out[pos++]='-';for(size_t i=0;i<n;++i)out[pos+i]=tmp[n-1u-i];pos+=n;out[pos]='\0';return pos;
}
} // namespace
HandlerStatus handle_are_chunks_installed(const redfield::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!output&&capacity)||!redfield::xml::span_equals(request.command,"AreChunksInstalled"))
        return redfield::xml::span_equals(request.command,"AreChunksInstalled")?HandlerStatus::invalid_request:HandlerStatus::wrong_command;
    if(!safe_attr(request.id)||!safe_attr(redfield::xml::response_sender_policy(request)))return HandlerStatus::invalid_request;

    StreamWriter w(output,capacity);
    w.append(" <LSX>\n  <Response id=\"");
    w.append_source(request.id);
    w.append("\" sender=\"");
    w.append_source(redfield::xml::response_sender_policy(request));
    w.append("\">\n    <AreChunksInstalledResponse ItemId=\"\" Installed=\"true\">");

    size_t cursor=0;
    for(;;){
        redfield::xml::DirectChildView child{};
        const auto status=redfield::xml::next_direct_child(request.command_content,&cursor,&child);
        if(status==redfield::xml::DirectChildStatus::end)break;
        if(status!=redfield::xml::DirectChildStatus::found){if(written)*written=0;return HandlerStatus::invalid_request;}
        if(!redfield::xml::span_equals(child.name,"ChunkIds"))continue;
        int32_t chunk_id=0;
        if(!parse_chunk_id(child.content,&chunk_id)){if(written)*written=0;return HandlerStatus::unsupported;}
        char text[16]{};const size_t text_n=i32_text(chunk_id,text);
        w.append("<ChunkIds>");w.append(text,text_n);w.append("</ChunkIds>");
    }
    w.append("</AreChunksInstalledResponse>\n  </Response>\n </LSX>\n");
    // RF10: explicit successful measure pass after full original validation.
    // Unlike StreamWriter::finish, this distinguishes invalid size from capacity.
    if(!output){
        if(!w.valid)return HandlerStatus::response_error;
        *written=w.pos;return HandlerStatus::ok;
    }
    return w.finish(written)?HandlerStatus::ok:HandlerStatus::response_error;
}

} }
namespace redfield {
ResponseResult dispatch_chunks_response(const xml::Request& request,char* output,std::size_t capacity,std::size_t* written) noexcept {
    if(!written)return ResponseResult::rejected;
    *written=0;if(!output || !capacity)return ResponseResult::rejected;
    using redfield::handlers::HandlerStatus;
    std::size_t measured=0;
    const auto measure=redfield::handlers::handle_are_chunks_installed(request,nullptr,0,&measured);
    if(measure==HandlerStatus::unsupported || measure==HandlerStatus::wrong_command)return ResponseResult::no_response;
    if(measure!=HandlerStatus::ok || measured>=capacity)return ResponseResult::rejected;
    std::size_t actual=0;
    const auto render=redfield::handlers::handle_are_chunks_installed(request,output,capacity,&actual);
    if(render!=HandlerStatus::ok || actual!=measured || actual>=capacity)return ResponseResult::rejected;
    *written=actual;return ResponseResult::response;
}
}
