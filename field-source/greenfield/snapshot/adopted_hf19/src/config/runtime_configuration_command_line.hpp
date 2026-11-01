#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace config { namespace runtime_command_line_detail {
struct CommandToken { const char* data; size_t size; bool present; };
enum class CommandTokenStatus : uint8_t { end = 0, token, malformed };

inline bool command_ws(char c) noexcept { return c==' ' || c=='\t' || c=='\r' || c=='\n'; }

inline CommandTokenStatus next_command_token(const char** cursor, CommandToken* out) noexcept {
    if(!cursor || !out || !*cursor) return CommandTokenStatus::malformed;
    const char* p=*cursor;
    while(*p && command_ws(*p)) ++p;
    if(!*p){ *cursor=p; *out={nullptr,0,false}; return CommandTokenStatus::end; }
    const char quote=(*p=='"' || *p=='\'') ? *p++ : 0;
    const char* begin=p;
    if(quote){
        while(*p && *p!=quote) ++p;
        if(*p!=quote){ *out={nullptr,0,false}; *cursor=p; return CommandTokenStatus::malformed; }
        *out={begin,static_cast<size_t>(p-begin),true};
        ++p;
        if(*p && !command_ws(*p)){ *out={nullptr,0,false}; *cursor=p; return CommandTokenStatus::malformed; }
    } else {
        while(*p && !command_ws(*p)) ++p;
        *out={begin,static_cast<size_t>(p-begin),true};
    }
    *cursor=p;
    return CommandTokenStatus::token;
}

inline bool token_equals(CommandToken token,const char* literal) noexcept {
    if(!token.present || !literal) return false;
    size_t n=0; while(literal[n]) ++n;
    if(token.size!=n) return false;
    for(size_t i=0;i<n;++i) if(token.data[i]!=literal[i]) return false;
    return true;
}
}}}
