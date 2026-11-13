// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/config/order.hpp"
#include <algorithm>

namespace citrine_eclipse_mirage::config_order {
UnsupportedOrder::UnsupportedOrder()
    :std::runtime_error("Config2 order reconstruction supports at most eight unique children per container") {}
std::uint64_t fnv1a64(std::string_view bytes) noexcept {
    std::uint64_t hash=0xcbf29ce484222325ULL;
    for(unsigned char byte:bytes) hash=(hash^byte)*0x100000001b3ULL;
    return hash;
}
namespace {
std::vector<std::string> rebuild8(const std::vector<std::string>& source) {
    if(source.size()>8) throw UnsupportedOrder();
    std::vector<std::string> list;
    list.reserve(source.size());
    for(const auto& key:source) {
        if(std::find(list.begin(),list.end(),key)!=list.end())
            throw std::invalid_argument("Config2 order input contains duplicate unique-key slots");
        const auto bucket=fnv1a64(key)&7U;
        // 011970 returns sentinel for an empty bucket and its first record for
        // a missing unique key in an occupied bucket. 012820 splices before it.
        const auto before=std::find_if(list.begin(),list.end(),[bucket](const auto& old) {
            return (fnv1a64(old)&7U)==bucket;
        });
        list.insert(before,key);
    }
    return list;
}
}
std::vector<std::string> parsed_order(const parser::Node& node) {
    if(node.children.size()>8) throw UnsupportedOrder();
    std::vector<std::string> keys;
    keys.reserve(node.children.size());
    for(const auto& child:node.children) keys.push_back(child.name);
    return rebuild8(keys);
}
std::vector<std::string> copy_order(const std::vector<std::string>& source_traversal) {
    // 0162f0 rebuilds even when 017440 returns the unchanged bucket count.
    // For unique keys 017570 moves each collision before its existing bucket's
    // first record. The fresh descendant-copy route 011ba0 uses the same splice.
    return rebuild8(source_traversal);
}
std::vector<std::string> SelectedCopyOrder::order_for(const parser::Node& node,
    const std::vector<std::string>& path) const {
    (void)path; // Same one-copy rule at every depth of the selected subtree.
    return copy_order(parsed_order(node));
}
} // namespace citrine_eclipse_mirage::config_order
