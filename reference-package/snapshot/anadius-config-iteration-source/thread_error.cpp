#include "thread_error.hpp"
#include <array>
#include <stdexcept>
#include <string>
#include <system_error>

namespace anadius::bootstrap_policies {
namespace {
// Retained 0x180087df0 code cells and 0x180087db8 context-pointer table.
// A rebuilt std::generic_category is selected after the original singleton's
// vtable name slot was independently resolved to 00fe60 returning "generic".
constexpr std::array<ThreadErrorSpec,7> errors{{
    {16,"device or resource busy"},
    {22,"invalid argument"},
    {3,"no such process"},
    {12,"not enough memory"},
    {1,"operation not permitted"},
    {36,"resource deadlock would occur"},
    {11,"resource unavailable try again"}
}};
}
ThreadErrorSpec thread_error_spec(int index){
    if(index<0||static_cast<unsigned>(index)>=errors.size())
        throw std::out_of_range("Thread error index outside retained 0..6 table");
    return errors[static_cast<unsigned>(index)];
}
[[noreturn]] void throw_thread_error_04a648(int index){
    const auto spec=thread_error_spec(index);
    throw std::system_error(spec.code,std::generic_category(),std::string(spec.context));
}
} // namespace anadius::bootstrap_policies
