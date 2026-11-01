#ifndef PRELOADER_L_TRACE_DIAGNOSTICS
#define PRELOADER_L_TRACE_DIAGNOSTICS 1
#endif
#include "preloader_l/runtime/trace_format.hpp"
#include <cstring>
int main(){
    char out[96]{};
    const size_t n=preloader_l::runtime::format_trace_line(preloader_l::runtime::TraceEvent::listener_active,3216,out,sizeof(out));
    if(!n) return 1;
    if(std::strcmp(out,"[preloader_l] listener_active value=3216\n")!=0) return 2;
    char tiny[8]{};
    if(preloader_l::runtime::format_trace_line(preloader_l::runtime::TraceEvent::failure,123,tiny,sizeof(tiny))!=0) return 3;
    return 0;
}
