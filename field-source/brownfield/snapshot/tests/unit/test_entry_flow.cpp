#ifndef PRELOADER_L_TRACE_DIAGNOSTICS
#define PRELOADER_L_TRACE_DIAGNOSTICS 1
#endif
#include "preloader_l/runtime/entry_flow.hpp"
#include "preloader_l/runtime/runtime_status.hpp"
#include "preloader_l/runtime/event_trace.hpp"
#include <stdint.h>
namespace { int seq[4]{}; int n=0; bool active_ok=true;
int active(void*,uint32_t,void*,void*) noexcept { seq[n++]=1; return active_ok?1:0; }
int secondary(void*,uint32_t,void*,void*) noexcept { seq[n++]=2; return 1; }
}
int main(){
 if(preloader_l::runtime::run_curio_entry_flow((void*)1,1,nullptr,&active,&secondary,nullptr)!=1)return 1;
 if(n!=2||seq[0]!=1||seq[1]!=2)return 2;
 n=0; if(preloader_l::runtime::run_curio_entry_flow((void*)1,0,nullptr,&active,&secondary,nullptr)!=1)return 3;
 if(n!=1||seq[0]!=2)return 4;
 n=0; active_ok=false; if(preloader_l::runtime::run_curio_entry_flow((void*)1,1,nullptr,&active,&secondary,nullptr)!=0)return 5;
 if(n!=1||seq[0]!=1)return 6;
 preloader_l::runtime::note_active_attach((void*)0x1234);
 const auto s=preloader_l::runtime::active_runtime_snapshot();
 if(s.module!=0x1234u||s.attach_count!=1||s.unresolved_mask!=0u||s.consistency_open_mask!=0u)return 7;
 if(static_cast<uint16_t>(preloader_l::runtime::TraceEvent::persistent_session_closed)<=static_cast<uint16_t>(preloader_l::runtime::TraceEvent::registry_iat_hook_status))return 8;
 return 0;
}
