#include "src/configuration/parser.hpp"
#include <tao/pegtl.hpp>
#include <type_traits>
#include <utility>

namespace preloader_l::parser {
ParseError::ParseError(ErrorRule r,Position p)
 :std::runtime_error("recovered Config2 grammar mismatch"),rule(r),position(p) {}
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
// The original package establishes these VDF rules and action ordering. PEGTL
// owns matching and input rewind; State actions intentionally do not roll back.
namespace peg = tao::pegtl;
namespace vdf {
struct Escaped : peg::seq<peg::one<'\\'>,peg::one<'"','\\'>> {};
struct Regular : peg::not_one<'\r','\n'> {};
struct Character : peg::sor<Escaped,Regular> {};
struct Quoted : peg::seq<peg::one<'"'>,
    peg::until<peg::one<'"'>,peg::must<Character>>> {};
struct Key : Quoted {};
struct Value : Quoted {};
struct ObjectName : Quoted {};
struct Eol : peg::sor<peg::one<'\n'>,peg::string<'\r','\n'>> {};
struct LineComment : peg::seq<peg::string<'/','/'>,
    peg::until<peg::sor<Eol,peg::eof>>> {};
// Separate '/' and '*' matches are intentional: a direct failed opener consumes
// '/', while a failed attempt under a required rewind guard restores the cursor.
struct BlockComment : peg::seq<peg::one<'/'>,peg::one<'*'>,
    peg::until<peg::string<'*','/'>>> {};
struct Sep : peg::sor<peg::ascii::space,LineComment,BlockComment> {};
struct Seps : peg::plus<Sep> {};
struct Object;
struct KeyValue : peg::seq<Key,Seps,Value> {};
struct Member : peg::sor<KeyValue,Object> {};
struct ObjectMembers : peg::until<peg::one<'}'>,
    peg::sor<peg::list<Member,Seps>,Seps>> {};
struct Object : peg::seq<ObjectName,peg::star<Sep>,peg::one<'{'>,
    peg::must<ObjectMembers>> {};
struct Grammar : peg::until<peg::eof,
    peg::sor<Eol,Sep,Object,peg::raise<Grammar>>> {};
} // namespace vdf

template<class Rule> struct Action : peg::nothing<Rule> {};
template<> struct Action<vdf::Escaped> {
    template<class Input> static void apply(const Input& in,State& s) {
        s.accumulator.push_back(in.peek_char(1));
    }
};
template<> struct Action<vdf::Regular> {
    template<class Input> static void apply(const Input& in,State& s) {
        s.accumulator.push_back(in.peek_char());
    }
};
void move_accumulator(State& s,std::string& destination) {
    destination=std::move(s.accumulator);s.accumulator.clear();
}
template<> struct Action<vdf::Key> {
    static void apply0(State& s) {move_accumulator(s,s.key);}
};
template<> struct Action<vdf::Value> {
    static void apply0(State& s) {move_accumulator(s,s.value);}
};
template<> struct Action<vdf::ObjectName> {
    static void apply0(State& s) {
        Node frame;move_accumulator(s,frame.name);
        s.open_objects.push_back(std::move(frame));
    }
};
template<> struct Action<vdf::KeyValue> {
    static void apply0(State& s) {
        Node node;node.name=std::move(s.key);s.key.clear();
        node.value=std::move(s.value);s.value.clear();
        if(s.open_objects.empty())throw std::logic_error("parser action stack empty");
        assign(s.open_objects.back(),std::move(node));
    }
};
template<> struct Action<vdf::Object> {
    static void apply0(State& s) {
        Node node=std::move(s.open_objects.back());s.open_objects.pop_back();
        assign(s.open_objects.empty()?s.root:s.open_objects.back(),std::move(node));
    }
};
// Keep the owned diagnostic interface consumed by Application and existing tests.
// This is not an assertion about the original PEGTL version or exception ABI.
template<class Rule> struct Control : peg::normal<Rule> {
    template<class Input> [[noreturn]] static void raise(const Input& in,State&) {
        static_assert(std::is_same_v<Rule,vdf::Character> ||
                      std::is_same_v<Rule,vdf::ObjectMembers> ||
                      std::is_same_v<Rule,vdf::Grammar>);
        constexpr auto rule=std::is_same_v<Rule,vdf::Character> ? ErrorRule::quoted_character :
            std::is_same_v<Rule,vdf::ObjectMembers> ? ErrorRule::object_members : ErrorRule::grammar;
        const auto p=in.current_position();
        throw ParseError(rule,{p.count,p.line,p.column});
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
    // Explicit LF tracking keeps CR and CRLF diagnostics identical on all hosts.
    peg::text_view_input<peg::ascii::scan::lf> input(raw.empty()?std::string_view(""):raw);
    return peg::parse<vdf::Grammar,Action,Control>(input,state);
}
configuration::Node to_configuration_node(const Node& node,const BinaryIterationOrder& order) {
    std::vector<std::string> path;return convert(node,order,path);
}
} // namespace preloader_l::parser
