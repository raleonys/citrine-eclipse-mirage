#include "checkpoint.hpp"
#include <stdexcept>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace anadius::original_checkpoint {
Boundary::Boundary(std::function<bool()> permission):permission_(std::move(permission)){
    if(!permission_)throw std::invalid_argument("Original SDK boundary requires explicit PTD permission");
}
void Boundary::verify(){
    struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}} error;
    if(!permission_())throw std::logic_error("Original SDK call violated selected callback PTD permission");
}
} // namespace anadius::original_checkpoint
