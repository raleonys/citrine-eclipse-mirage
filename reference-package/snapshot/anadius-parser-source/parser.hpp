#pragma once
#include "../anadius-config-source/configuration.hpp"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace anadius::parser {
struct Position {std::size_t byte=0,line=1,column=1;};
enum class ErrorRule {grammar, object_members, quoted_character};
class ParseError : public std::runtime_error {
public:
    ParseError(ErrorRule r,Position p);
    ErrorRule rule;
    Position position; // Position within input AFTER removal of one UTF-8 BOM.
};
struct Node {
    std::string name,value;
    // Logical children: first-created key slots, last completed assignment wins.
    // This source/action order IS NOT original MSVC unordered-container order.
    std::vector<Node> children;
    std::vector<std::string> completed_assignments;
    const Node* find(std::string_view key) const;
};
struct State {
    Node root;
    std::vector<Node> open_objects;
    std::string accumulator,key,value;
    bool removed_utf8_bom=false;
};
// Static grammar/action reconstruction. No I/O, target code, locale or DLL calls.
// Empty input succeeds. Syntax failure throws ParseError; completed earlier
// nodes and action state remain available in State. Original exception ABI,
// text/type demangling and unwind cursor side effects are not reproduced.
bool parse(std::string_view raw_bytes,State& state);

class BinaryIterationOrder {
public:
    virtual ~BinaryIterationOrder()=default;
    // Must return ORIGINAL Windows hash/list order at the configuration-copy
    // stage. Source order, libc++ order and this vector order are not substitutes.
    virtual std::vector<std::string> order_for(const Node& logical_node,
         const std::vector<std::string>& path) const=0;
};
configuration::Node to_configuration_node(const Node&,const BinaryIterationOrder&);
} // namespace anadius::parser
