#include "preloader_l/runtime/exception_policy.hpp"
#include <cassert>
using namespace preloader_l::runtime;
int main(){
 const auto a=allocator_escape_contract(AllocatorEscapeShape::noexcept_a);
 const auto t=allocator_escape_contract(AllocatorEscapeShape::target_unwind);
 const auto b=allocator_escape_contract(AllocatorEscapeShape::noexcept_b);
 assert(a.disposition==AllocatorEscapeDisposition::search_terminate && a.route_count==6 && a.body_rsp_to_caller_rsp_delta==0x1D0 && !a.catch_funclet_executes);
 assert(b.disposition==AllocatorEscapeDisposition::search_terminate && b.route_count==6 && b.body_rsp_to_caller_rsp_delta==0x170 && !b.catch_funclet_executes);
 assert(t.disposition==AllocatorEscapeDisposition::target_unwind_terminate && t.route_count==6 && t.body_rsp_to_caller_rsp_delta==0x190);
 assert(t.allocator_core_cleanup && t.allocator_mid_cleanup && !t.catch_funclet_executes);
 assert(a.route_count+b.route_count+t.route_count==curio_allocator_escape_routes);
 assert(curio_search_terminate_routes==12 && curio_target_unwind_terminate_routes==6 && curio_catch_funclet_routes==0);
 static_assert(noexcept(invoke_noexcept_boundary(nullptr,nullptr)),"noexcept boundary must remain noexcept");
 return 0;
}
