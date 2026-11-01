#include "preloader_l/runtime/exception_runtime.hpp"
#include <cassert>
using namespace preloader_l::runtime;
int main(){
 auto s=evaluate_stream_exception_policy(StreamExceptionPolicy::catch_and_swallow,2,4,4);
 assert(s.resulting_state==2 && !s.rethrow_current_exception);
 s=evaluate_stream_exception_policy(StreamExceptionPolicy::set_badbit_conditional_rethrow,2,4,0);
 assert(s.resulting_state==6 && !s.rethrow_current_exception);
 s=evaluate_stream_exception_policy(StreamExceptionPolicy::set_badbit_conditional_rethrow,2,4,4);
 assert(s.resulting_state==6 && s.rethrow_current_exception);
 TrackedAvRange r{0x1000,0x2000,0x1200,0x3000,0x4000,0x3200,0x80};
 auto d=evaluate_tracked_av(access_violation_code,0x1220,0x1230,&r,1);
 assert(d.decision==SehFilterDecision::continue_execution && d.tracked && d.exact_source_span);
 assert(d.rewrite_rip && d.mapped_rip==0x3230);
 d=evaluate_tracked_av(access_violation_code,0x1800,0x5000,&r,1);
 assert(d.tracked && !d.exact_source_span && !d.rewrite_rip && d.decision==SehFilterDecision::continue_execution);
 d=evaluate_tracked_av(msvc_cpp_exception_code,0x1220,0x1230,&r,1);
 assert(d.decision==SehFilterDecision::continue_search && !d.tracked);
 d=evaluate_tracked_av(access_violation_code,0x9000,0x1230,&r,1);
 assert(d.decision==SehFilterDecision::continue_search && !d.tracked);
 return 0;
}
