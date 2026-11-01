#include "preloader_l/runtime/exception_policy.hpp"
#include <cassert>
#include <exception>
#include <new>
using namespace preloader_l::runtime;
int main(){
 bool exact_bad_alloc=false, base_bad_alloc=false;
 try { throw_allocation_terminal(AllocationTerminal::bad_alloc); }
 catch (const std::bad_alloc&) { exact_bad_alloc=true; }
 assert(exact_bad_alloc);
 try { throw_allocation_terminal(AllocationTerminal::bad_alloc); }
 catch (const std::exception&) { base_bad_alloc=true; }
 assert(base_bad_alloc);

 bool exact_array=false, as_bad_alloc=false, as_exception=false;
 try { throw_allocation_terminal(AllocationTerminal::bad_array_new_length); }
 catch (const std::bad_array_new_length&) { exact_array=true; }
 assert(exact_array);
 try { throw_allocation_terminal(AllocationTerminal::bad_array_new_length); }
 catch (const std::bad_alloc&) { as_bad_alloc=true; }
 assert(as_bad_alloc);
 try { throw_allocation_terminal(AllocationTerminal::bad_array_new_length); }
 catch (const std::exception&) { as_exception=true; }
 assert(as_exception);
 return 0;
}
