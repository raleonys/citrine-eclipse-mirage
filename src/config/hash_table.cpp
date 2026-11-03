// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/config/hash_table.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <set>
#include <stdexcept>

namespace preloader_l::config_hash {
std::uint64_t fnv1a64(std::string_view bytes) noexcept {
    std::uint64_t hash=0xcbf29ce484222325ULL;
    for(unsigned char byte:bytes)hash=(hash^byte)*0x100000001b3ULL;
    return hash;
}
namespace {
void valid_capacity(std::uint64_t capacity) {
    if(capacity<8 || capacity>max_native_buckets || !std::has_single_bit(capacity))
        throw std::invalid_argument("Config2 invalid logical bucket capacity");
}
void valid_count(std::uint64_t count) {
    if(count>max_native_size)throw std::length_error("Config2 native size limit");
}
std::vector<std::string> regroup(const std::vector<std::string>& source,std::uint64_t buckets) {
    // 017570 unique-key branch. Bucket groups appear in order of their first
    // source-list record. Each following collision is spliced before the first.
    // Use owned vector groups, not host unordered_map iteration order.
    std::map<std::uint64_t,std::size_t> slot;
    std::vector<std::vector<std::string>> groups;
    for(const auto& key:source) {
        const auto bucket=fnv1a64(key)&(buckets-1);
        const auto [found,created]=slot.emplace(bucket,groups.size());
        if(created)groups.emplace_back();
        groups[found->second].push_back(key);
    }
    std::vector<std::string> result;
    result.reserve(source.size());
    for(const auto& group:groups)result.insert(result.end(),group.rbegin(),group.rend());
    return result;
}
}
std::uint64_t growth_target(std::uint64_t current,std::uint64_t desired) {
    valid_capacity(current);valid_count(desired);
    // 017440 uses binary32 division/ceil, with load factor fixed at observed1.0.
    // max_native_size is below2^63; conversion is defined in this admitted range.
    const auto minimum=std::max<std::uint64_t>(8,static_cast<std::uint64_t>(std::ceil(static_cast<float>(desired)/1.0F)));
    if(minimum<=current)return current;
    if(current<0x200 && minimum<=current*8)return current*8;
    return minimum;
}
std::uint64_t rounded_buckets(std::uint64_t requested) {
    if(requested<8)throw std::invalid_argument("Config2 rehash request is outside observed minimum8 domain");
    if(requested>max_native_buckets)throw std::length_error("Config2 native bucket limit");
    // All observed callers use017440's minimum8. Smaller raw native arguments
    // are deliberately outside this logical model, not original error claims.
    return std::bit_ceil(requested);
}
void Table::validate() const {
    valid_capacity(bucket_count);valid_count(traversal.size());
    std::set<std::string,std::less<>> unique;
    for(const auto& key:traversal)
        if(!unique.insert(key).second)throw std::invalid_argument("Config2 duplicate logical table key");
}
void Table::rebuild(std::uint64_t buckets) {
    const auto capacity=rounded_buckets(buckets);
    auto reordered=regroup(traversal,capacity);
    bucket_count=capacity;traversal=std::move(reordered);
}
bool Table::insert_unique(std::string_view key) {
    valid_capacity(bucket_count);
    if(std::find(traversal.begin(),traversal.end(),key)!=traversal.end())return false;
    const std::string owned_key(key); // Provider view may point into old traversal.
    if(traversal.size()>=max_native_size)throw std::length_error("Config2 native size limit");
    const auto count=traversal.size()+1;
    const float ratio=static_cast<float>(count)/static_cast<float>(bucket_count);
    if(ratio>1.0F)rebuild(growth_target(bucket_count,count));
    const auto bucket=fnv1a64(owned_key)&(bucket_count-1);
    const auto before=std::find_if(traversal.begin(),traversal.end(),[&](const auto& old) {
        return (fnv1a64(old)&(bucket_count-1))==bucket;
    });
    traversal.insert(before,owned_key);
    return true;
}
void Table::assign_from(const Table& source) {
    if(this==&source)return;
    source.validate();valid_capacity(bucket_count);
    const auto capacity=growth_target(bucket_count,source.traversal.size());
    traversal=source.traversal;
    rebuild(capacity); // 0162f0 calls017570 even when capacity is unchanged.
}
Table Table::clone(const Table& source) {
    source.validate();Table result;result.bucket_count=source.bucket_count;
    for(const auto& key:source.traversal)result.insert_unique(key);
    return result;
}
const Node* Node::find(std::string_view key) const {
    const auto found=children.find(key);return found==children.end()?nullptr:&found->second;
}
Node& Node::lookup(std::string_view key) {
    const auto found=children.find(key);
    if(found!=children.end())return found->second;
    // A missing key may borrow a substring of table.traversal. Rehashing
    // invalidates that view, so first own the key in its stable map node.
    // On insertion failure, discard the provisional child before retry.
    const auto [row,inserted]=children.try_emplace(std::string(key));
    if(!inserted)return row->second;
    try {table.insert_unique(row->first);}
    catch(...) {children.erase(row);throw;}
    return row->second;
}
void Node::validate() const {
    table.validate();
    if(children.size()!=table.traversal.size())throw std::invalid_argument("Config2 node/list size mismatch");
    for(const auto& key:table.traversal)
        if(children.find(key)==children.end())throw std::invalid_argument("Config2 node/list key mismatch");
}
void Node::assign_from(const Node& source) {
    if(this==&source)return;
    validate();source.validate();
    name=source.name;value=source.value;
    std::map<std::string,Node,std::less<>> replacement;
    for(std::size_t i=0;i<source.table.traversal.size();++i) {
        const auto& key=source.table.traversal[i];
        const auto& from=source.children.at(key);
        Node row;
        if(i<table.traversal.size()) {
            row=std::move(children.at(table.traversal[i]));
            row.assign_from(from);
        }else row=Node::clone(from);
        replacement.emplace(key,std::move(row));
    }
    table.assign_from(source.table);
    children=std::move(replacement);
}
Node Node::clone(const Node& source) {
    source.validate();Node result;result.name=source.name;result.value=source.value;
    result.table=Table::clone(source.table);
    for(const auto& key:source.table.traversal)
        result.children.emplace(key,Node::clone(source.children.at(key)));
    return result;
}
Node from_parser(const parser::Node& source) {
    Node result;result.name=source.name;result.value=source.value;
    for(const auto& child:source.children) {
        if(!result.table.insert_unique(child.name))throw std::invalid_argument("Config2 parser has duplicate logical slots");
        result.children.emplace(child.name,from_parser(child));
    }
    return result;
}
Node selected_copy(const parser::Node& selected) {
    const auto parsed=from_parser(selected);Node result;result.assign_from(parsed);return result;
}
configuration::Node to_configuration(const Node& source) {
    source.validate();configuration::Node result;result.value=source.value;
    result.iteration_order=source.table.traversal;
    for(const auto& key:source.table.traversal)
        result.children.emplace(key,to_configuration(source.children.at(key)));
    return result;
}
} // namespace preloader_l::config_hash
