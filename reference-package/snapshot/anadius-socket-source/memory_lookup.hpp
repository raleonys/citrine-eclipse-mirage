#pragma once

namespace anadius::socket_recovered {
// Exact projection lookup. Found returns a descriptor, not authority or a
// payload lifetime pin. Miss and Ambiguous remain distinct for owner overlays.
enum class MemoryLookup {Miss,Found,Ambiguous};
} // namespace anadius::socket_recovered
