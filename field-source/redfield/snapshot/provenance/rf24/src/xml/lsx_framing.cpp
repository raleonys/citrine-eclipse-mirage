// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/xml/lsx_framing.hpp"

namespace redfield { namespace xml {
namespace {
constexpr char open_tag[] = "<LSX>";
constexpr char close_tag[] = "</LSX>";
constexpr size_t open_length = 5;
constexpr size_t close_length = 6;

bool bytes_equal(const char* input, const char* expected, size_t count) noexcept
{
    for (size_t i = 0; i < count; ++i) if (input[i] != expected[i]) return false;
    return true;
}
bool starts(const char* input,size_t length,size_t pos,const char* token,size_t token_n) noexcept {
    return pos<=length && token_n<=length-pos && bytes_equal(input+pos,token,token_n);
}
enum class LexMode : uint8_t { text=0,tag,comment,cdata,processing_instruction };
} // namespace

FrameScanResult scan_lsx_frame(const char* input, size_t length) noexcept
{
    if (!input && length != 0) return {FrameStatus::invalid_argument, 0};
    const size_t prefix_count = length < open_length ? length : open_length;
    for (size_t i = 0; i < prefix_count; ++i) if (input[i] != open_tag[i]) return {FrameStatus::invalid_prefix, 0};
    if (length < open_length + close_length) return {FrameStatus::need_more, 0};

    LexMode mode=LexMode::text;
    char quote=0;
    for(size_t i=open_length;i<length;++i){
        if(mode==LexMode::comment){
            if(starts(input,length,i,"-->",3u)){ mode=LexMode::text; i+=2u; }
            continue;
        }
        if(mode==LexMode::cdata){
            if(starts(input,length,i,"]]>",3u)){ mode=LexMode::text; i+=2u; }
            continue;
        }
        if(mode==LexMode::processing_instruction){
            if(starts(input,length,i,"?>",2u)){ mode=LexMode::text; ++i; }
            continue;
        }
        if(mode==LexMode::tag){
            const char c=input[i];
            if(quote){ if(c==quote) quote=0; continue; }
            if(c=='\'' || c=='"'){ quote=c; continue; }
            if(c=='>') mode=LexMode::text;
            continue;
        }

        if(starts(input,length,i,close_tag,close_length)) return {FrameStatus::frame_ready,i+close_length};
        if(input[i]!='<') continue;
        if(starts(input,length,i,"<!--",4u)){ mode=LexMode::comment; i+=3u; continue; }
        if(starts(input,length,i,"<![CDATA[",9u)){ mode=LexMode::cdata; i+=8u; continue; }
        if(starts(input,length,i,"<?",2u)){ mode=LexMode::processing_instruction; ++i; continue; }
        mode=LexMode::tag;
    }
    return {FrameStatus::need_more, 0};
}

} } // namespace redfield::xml
