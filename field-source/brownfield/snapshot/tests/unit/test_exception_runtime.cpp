#include "preloader_l/runtime/exception_runtime.hpp"
#include <cassert>
using namespace preloader_l::runtime;
namespace { int g_sig=0; void cb(int32_t sig){ g_sig=sig; } }
int main(){
 const auto nh=new_handler_escape_contract();
 assert(nh.active_roots==18 && nh.callback_layers==2 && nh.total_route_layers==36);
 assert(nh.search_terminate==24 && nh.target_unwind_terminate==12 && nh.catch_funclet_executes==0);
 assert(nh.outer_body_rsp_delta==0x60 && nh.inner_body_rsp_delta==0x90);
 // Non-C++ codes search even if a fabricated row has a non-zero action.
 CrtActionView noncpp[]={{access_violation_code,11,5}};
 assert(evaluate_crt_exception_filter(access_violation_code,noncpp,1)==SehFilterDecision::continue_search);
 CrtActionView none[]={{msvc_cpp_exception_code,4,0}};
 CrtActionView exec[]={{msvc_cpp_exception_code,8,5}};
 CrtActionView cont[]={{msvc_cpp_exception_code,11,1}};
 CrtActionView call[]={{msvc_cpp_exception_code,7,reinterpret_cast<uintptr_t>(&cb)}};
 assert(evaluate_crt_exception_filter(msvc_cpp_exception_code,none,1)==SehFilterDecision::continue_search);
 assert(evaluate_crt_exception_filter(msvc_cpp_exception_code,exec,1)==SehFilterDecision::execute_handler);
 assert(evaluate_crt_exception_filter(msvc_cpp_exception_code,cont,1)==SehFilterDecision::continue_execution);
 assert(evaluate_crt_exception_filter(msvc_cpp_exception_code,call,1)==SehFilterDecision::continue_execution && g_sig==7);
 CrtActionView no_cpp_row[]={{0xC000008Eu,8,0}};
 assert(evaluate_crt_exception_filter(msvc_cpp_exception_code,no_cpp_row,1)==SehFilterDecision::continue_search);
 return 0;
}
