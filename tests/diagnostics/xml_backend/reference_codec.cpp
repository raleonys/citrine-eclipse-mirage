// SPDX-License-Identifier: GPL-3.0-only

// Compile the unchanged first-party value policy extracted from the pinned
// control. The candidate does not compile or link the TinyXML2 reader/oracle.
#include "reference_value_policy.hpp"
#include "backend.hpp"
namespace xml_comparison {
std::string decode_reference_value(std::string_view value) {
    return policy::decode_references(value);
}
}
