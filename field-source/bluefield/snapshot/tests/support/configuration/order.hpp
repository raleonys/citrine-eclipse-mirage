#pragma once
#include "src/configuration/parser.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace preloader_l::config_order {
// Deliberate source reconstruction boundary, not an original binary exception.
class UnsupportedOrder : public std::runtime_error {
public:
    UnsupportedOrder();
};
std::uint64_t fnv1a64(std::string_view bytes) noexcept; // 012820 / 011ba0 / 017570
// At most eight distinct children: fresh mask7, bucket_count8, load_factor1.
// Node.children must be the parser's first-created unique-key slots. Completed
// duplicate assignments replace values/subtrees without moving the key slot.
std::vector<std::string> parsed_order(const parser::Node&);
// One copy into a fresh eight-bucket destination, supplied original traversal.
// Does not model copy into a previously grown/reused section destination.
std::vector<std::string> copy_order(const std::vector<std::string>& source_traversal);

// ONLY the first selected Config2 subtree copy (019d52 -> 0165c0). Each copied
// descendant also gets one rebuild. The caller must select Config2 before
// conversion; this is not an order provider for the whole root document or for
// Content/Entitlements/Achievements after subsequent copies within 019ad0.
class SelectedCopyOrder final : public parser::BinaryIterationOrder {
public:
    std::vector<std::string> order_for(const parser::Node&,
        const std::vector<std::string>& path) const override;
};
} // namespace preloader_l::config_order
