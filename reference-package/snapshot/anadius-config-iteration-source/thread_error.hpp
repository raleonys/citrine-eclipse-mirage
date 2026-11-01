#pragma once
#include <string_view>

namespace anadius::bootstrap_policies {
// Exact retained 04a648 indexed code/context pair. Valid native call indices
// are bounded to 0..6; other values are rejected before table access.
struct ThreadErrorSpec {int code;std::string_view context;};
ThreadErrorSpec thread_error_spec(int index);
// Rebuilt C++ exception representation. The code and context are retained data;
// compiler category text, what() layout, locale and native MSVC EH/ABI equality
// are not implied by source-level exception propagation.
[[noreturn]] void throw_thread_error_04a648(int index);
} // namespace anadius::bootstrap_policies
