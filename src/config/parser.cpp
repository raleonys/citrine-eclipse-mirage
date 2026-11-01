// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/config/parser.hpp"
#include "preloader_l/config/vdf_grammar.hpp"
#include <type_traits>
#include <utility>

namespace preloader_l::parser {
ParseError::ParseError(ErrorRule r,Position p)
 :std::runtime_error("Config2 grammar mismatch"),rule(r),position(p) {}
const Node* Node::find(std::string_view key) const {
    for(const auto& node:children) if(node.name==key)return &node;
    return nullptr;
}
namespace {
void assign(Node& parent,Node node) {
    parent.completed_assignments.push_back(node.name);
    for(auto& old:parent.children)
        if(old.name==node.name){old=std::move(node);return;}
    parent.children.push_back(std::move(node));
}
// Only the RE-documented actions mutate State. PEGTL owns input matching and
// rewind; a failed ordered alternative intentionally leaves action side effects.
namespace peg = tao::pegtl;
template<class Rule> struct VdfAction : peg::nothing<Rule> {};
template<> struct VdfAction<vdf::EscapedByte> {
    template<class Input> static void apply(const Input& input,State& state) {
        state.accumulator.push_back(input.peek_char(1));
    }
};
template<> struct VdfAction<vdf::RegularByte> {
    template<class Input> static void apply(const Input& input,State& state) {
        state.accumulator.push_back(input.peek_char());
    }
};
void take_accumulator(State& state,std::string& destination) {
    destination=std::move(state.accumulator);state.accumulator.clear();
}
template<> struct VdfAction<vdf::Key> {
    static void apply0(State& state) {take_accumulator(state,state.key);}
};
template<> struct VdfAction<vdf::Value> {
    static void apply0(State& state) {take_accumulator(state,state.value);}
};
template<> struct VdfAction<vdf::ObjectName> {
    static void apply0(State& state) {
        // 012ff0: push the named frame before the following '{' is checked.
        Node frame;take_accumulator(state,frame.name);
        state.open_objects.push_back(std::move(frame));
    }
};
template<> struct VdfAction<vdf::KeyValue> {
    static void apply0(State& state) {
        // 012cf0: assignment replaces the entire earlier node, never merges it.
        Node node;node.name=std::move(state.key);state.key.clear();
        node.value=std::move(state.value);state.value.clear();
        if(state.open_objects.empty())throw std::logic_error("parser action stack empty");
        assign(state.open_objects.back(),std::move(node));
    }
};
template<> struct VdfAction<vdf::Object> {
    static void apply0(State& state) {
        // 012bf0: only a completed object is moved into its parent/root.
        Node node=std::move(state.open_objects.back());state.open_objects.pop_back();
        assign(state.open_objects.empty()?state.root:state.open_objects.back(),std::move(node));
    }
};
template<class Rule> struct VdfControl : peg::normal<Rule> {
    template<class Input> [[noreturn]] static void raise(const Input& input,State&) {
        static_assert(std::is_same_v<Rule,vdf::Character> ||
            std::is_same_v<Rule,vdf::ObjectMembers> || std::is_same_v<Rule,vdf::Document>);
        constexpr auto rule=std::is_same_v<Rule,vdf::Character>?ErrorRule::quoted_character:
            std::is_same_v<Rule,vdf::ObjectMembers>?ErrorRule::object_members:ErrorRule::grammar;
        const auto position=input.current_position();
        throw ParseError(rule,{position.count,position.line,position.column});
    }
};
configuration::Node convert(const Node& node,const BinaryIterationOrder& orders,
                           std::vector<std::string>& path) {
    configuration::Node result;result.value=node.value;
    for(const auto& child:node.children){
        path.push_back(child.name);
        result.children.emplace(child.name,convert(child,orders,path));
        path.pop_back();
    }
    if(!node.children.empty()){
        result.iteration_order=orders.order_for(node,path);
        (void)result.ordered_children(); // Reject guessed/incomplete provider data.
    }
    return result;
}
} // namespace
bool parse(std::string_view raw,State& state) {
    state.removed_utf8_bom=raw.substr(0,3)==std::string_view("\xef\xbb\xbf",3);
    if(state.removed_utf8_bom)raw.remove_prefix(3);
    // Explicit LF-only counting matches the byte/line/column contract
    // for CR, LF and CRLF on both hosts. Empty views may otherwise have nullptr data.
    peg::text_view_input<peg::ascii::scan::lf> input(raw.empty()?std::string_view(""):raw);
    return peg::parse<vdf::Document,VdfAction,VdfControl>(input,state);
}
configuration::Node to_configuration_node(const Node& node,const BinaryIterationOrder& order) {
    std::vector<std::string> path;return convert(node,order,path);
}
} // namespace preloader_l::parser
