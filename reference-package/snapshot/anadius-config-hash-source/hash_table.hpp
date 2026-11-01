#pragma once
#include "../anadius-parser-source/parser.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace anadius::config_hash {
// Logical key/list/capacity reconstruction. No MSVC layout or allocation ABI.
inline constexpr std::uint64_t max_native_size=0x1745d1745d1745dULL;
inline constexpr std::uint64_t max_native_buckets=1ULL<<59;
std::uint64_t fnv1a64(std::string_view) noexcept;
std::uint64_t growth_target(std::uint64_t current_buckets,std::uint64_t desired_size);
// Observed caller domain [8,2^59]; rejecting smaller requests is a model guard.
std::uint64_t rounded_buckets(std::uint64_t requested);
struct Table {
    std::uint64_t bucket_count=8;
    std::vector<std::string> traversal;
    bool insert_unique(std::string_view key); // Existing key never moves/grows.
    void assign_from(const Table&);          // Retain destination capacity, rebuild.
    static Table clone(const Table&);        // Inherit source capacity, reinsert.
    void validate() const;
private:
    void rebuild(std::uint64_t buckets);
};

struct Node {
    std::string name,value;
    Table table;
    std::map<std::string,Node,std::less<>> children;
    const Node* find(std::string_view key) const;
    // Original operator[] copies outer key but leaves new inner name/value empty.
    Node& lookup(std::string_view key);
    // Reuse existing destination list records POSITIONALLY before parent rehash.
    // Their child tables retain capacity; extra records use recursive clone.
    void assign_from(const Node&);
    static Node clone(const Node&);
    void validate() const;
};
Node from_parser(const parser::Node&);
Node selected_copy(const parser::Node& selected_config2);
configuration::Node to_configuration(const Node&);
} // namespace anadius::config_hash
