#include "preloader_l/debug/debug_api.hpp"
#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/net/session_state.hpp"
#include "preloader_l/runtime/loader_bridge.hpp"
#include "preloader_l/runtime/loader_signals.hpp"
#include "preloader_l/runtime/runtime_state.hpp"
#include <cstring>
#include <cstdint>

int main(){
 using namespace preloader_l;
 runtime::reset_signals_for_test(); runtime::reset_state_for_test(); preloader_l_debug_reset_stats();
 void* module=reinterpret_cast<void*>(static_cast<uintptr_t>(0x180000000ull));
 runtime::handle_loader_event(module,runtime::loader_process_attach);
 if(preloader_l_debug_runtime_init()!=static_cast<uint32_t>(runtime::InitResult::ok)) return 1;
 if(preloader_l_debug_session_reset(45)!=static_cast<uint32_t>(net::SessionStatus::ok)) return 2;
 constexpr char p1[]="<LSX><Request id=\"45\"><Get";
 constexpr char p2[]="Config/></Request></LSX>";
 char out[4096]{}; size_t written=99;
 if(preloader_l_debug_session_feed(p1,sizeof(p1)-1,out,sizeof(out),&written)!=PRELOADER_L_DEBUG_SESSION_NEED_MORE || written!=0) return 3;
 preloader_l_debug_session_status st{}; if(preloader_l_debug_get_session_status(&st,sizeof(st))!=0u) return 4;
 if(st.phase!=static_cast<uint32_t>(net::SessionPhase::connected) || st.session_id!=45 || st.receive_events!=1 || st.buffered_bytes!=sizeof(p1)-1) return 5;
 // F323-S0470-001: a complete frame with insufficient response capacity
 // stays pending and required-size metadata must not count as delivered bytes.
 char tiny[8]{}; size_t required=0;
 if(preloader_l_debug_session_feed(p2,sizeof(p2)-1,tiny,sizeof(tiny),&required)!=static_cast<uint32_t>(core::DispatchStatus::capacity)) return 6;
 if(required<sizeof(tiny)) return 7;
 if(preloader_l_debug_get_session_status(&st,sizeof(st))!=0u) return 8;
 if(st.frames_consumed!=0 || st.send_events!=0 || st.sent_bytes!=0 || st.buffered_bytes==0) return 9;
 if(preloader_l_debug_session_feed(nullptr,0,out,sizeof(out),&written)!=static_cast<uint32_t>(core::DispatchStatus::ok)) return 10;
 if(written==0 || !std::strstr(out,"<GetConfigResponse>")) return 11;
 if(preloader_l_debug_get_session_status(&st,sizeof(st))!=0u) return 12;
 if(st.receive_events!=2 || st.frames_consumed!=1 || st.buffered_bytes!=0 || st.send_events!=1 || st.sent_bytes!=written) return 13;
 constexpr char pair[]="<LSX><Request id=\"451\"><GetConfig/></Request></LSX><LSX><Request id=\"452\"><GetConfig/></Request></LSX>";
 if(preloader_l_debug_session_feed(pair,sizeof(pair)-1,out,sizeof(out),&written)!=static_cast<uint32_t>(core::DispatchStatus::ok)) return 14;
 if(preloader_l_debug_get_session_status(&st,sizeof(st))!=0u || st.buffered_bytes==0 || st.frames_consumed!=2) return 15;
 if(preloader_l_debug_session_feed(nullptr,0,out,sizeof(out),&written)!=static_cast<uint32_t>(core::DispatchStatus::ok)) return 16;
 if(preloader_l_debug_get_session_status(&st,sizeof(st))!=0u || st.buffered_bytes!=0 || st.frames_consumed!=3) return 17;
 if(preloader_l_debug_session_close()!=static_cast<uint32_t>(net::SessionStatus::ok)) return 18;
 if(preloader_l_debug_get_session_status(&st,sizeof(st))!=0u || st.phase!=static_cast<uint32_t>(net::SessionPhase::closed)) return 19;
 if(preloader_l_debug_runtime_shutdown()!=static_cast<uint32_t>(runtime::ShutdownResult::ok)) return 20;
 return 0;
}
