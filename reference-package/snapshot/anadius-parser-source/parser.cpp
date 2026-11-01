#include "parser.hpp"
#include <algorithm>
#include <utility>

namespace anadius::parser {
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
class Parser {
    std::string_view text;
    State& state;
    Position pos;
    bool at(char c) const{return pos.byte<text.size() && text[pos.byte]==c;}
    void bump() {
        if(text[pos.byte++]=='\n'){++pos.line;pos.column=1;}
        else ++pos.column;
    }
    [[noreturn]] void error(ErrorRule rule) const {throw ParseError(rule,pos);}
    // 0135b0 ascii-space mask: HT LF VT FF CR SP; no locale assumption.
    bool space() {
        if(pos.byte==text.size())return false;
        const auto c=static_cast<unsigned char>(text[pos.byte]);
        if(c!=' ' && !(c>='\t' && c<='\r'))return false;
        bump();return true;
    }
    // 013460: // through and including LF/CRLF, or to EOF.
    bool comment() {
        if(text.substr(pos.byte,2)!="//")return false;
        bump();bump();
        while(pos.byte<text.size()){
            if(at('\n')){bump();return true;}
            if(text.substr(pos.byte,2)=="\r\n"){bump();bump();return true;}
            bump();
        }
        return true;
    }
    // 013ed0 direct body form: failure may consume '/' or the entire unfinished
    // comment. Callers using a rewind marker restore it; direct body error does not.
    bool block_comment_body() {
        if(!at('/'))return false;
        bump();
        if(!at('*'))return false;
        bump();
        while(pos.byte<text.size()){
            if(text.substr(pos.byte,2)=="*/"){bump();bump();return true;}
            bump();
        }
        return false;
    }
    // 013520 and the repeat tail of013ab0 wrap the block attempt in a marker.
    bool sep(){
        if(space() || comment())return true;
        const auto saved=pos;
        if(block_comment_body())return true;
        pos=saved;return false;
    }
    // 013ab0 /013cc0's initial Sep has no successful enclosing marker to restore
    // before a direct syntax throw; preserve its consumed-failure diagnostic.
    bool seps(){
        if(!(space() || comment() || block_comment_body()))return false;
        while(sep()){}
        return true;
    }
    // 013810 +013640 +0141a0. Unknown escape falls through Regular:
    // backslash remains literal; only backslash-quote/backslash-backslash decode.
    bool quoted() {
        if(!at('"'))return false;
        bump();
        while(!at('"')){
            if(pos.byte==text.size() || at('\r') || at('\n'))error(ErrorRule::quoted_character);
            if(at('\\') && pos.byte+1<text.size() &&
               (text[pos.byte+1]=='"' || text[pos.byte+1]=='\\')){
                bump();state.accumulator.push_back(text[pos.byte]);bump();
            }else{state.accumulator.push_back(text[pos.byte]);bump();}
        }
        bump();return true;
    }
    void move_accumulator(std::string& destination) {
        destination=std::move(state.accumulator);state.accumulator.clear();
    }
    // 012cf0 assigns a fresh scalar node into current object: duplicate replaces
    // previous node including all previous child objects, rather than merging.
    void scalar_action() {
        Node node;node.name=std::move(state.key);state.key.clear();
        node.value=std::move(state.value);state.value.clear();
        if(state.open_objects.empty())throw std::logic_error("parser action stack empty");
        assign(state.open_objects.back(),std::move(node));
    }
    // 012bf0 moves completed frame to parent/root; all children are replaced.
    void object_action() {
        Node node=std::move(state.open_objects.back());state.open_objects.pop_back();
        assign(state.open_objects.empty()?state.root:state.open_objects.back(),std::move(node));
    }
    // 0138e0 ordered choice: KeyValue before Object. Input markers rewind failed
    // alternatives; action state (key strings/open frames) does not roll back.
    bool member() {
        const auto start=pos;
        if(quoted()){
            move_accumulator(state.key);
            if(seps() && quoted()){
                move_accumulator(state.value);scalar_action();return true;
            }
        }
        pos=start;
        if(object()){object_action();return true;}
        pos=start;return false;
    }
    // 013b70: ObjectName action happens immediately after quoted name, even if
    // following '{' does not match. It is NOT a transaction rolled back on false.
    bool object() {
        if(!quoted())return false;
        Node frame;move_accumulator(frame.name);state.open_objects.push_back(std::move(frame));
        while(sep()){}
        if(!at('{'))return false;
        bump();members();return true;
    }
    // 013cc0: until<'~}'> sor<list<sor<KeyValue,Object>,Seps>,Seps>.
    // Repeated outer until also permits adjacent members without a separator;
    // KeyValue itself still requires >=1 Sep between quoted key and value.
    void members() {
        while(!at('}')){
            const auto start=pos;
            if(member()){
                while(true){
                    const auto next=pos;
                    if(seps() && member())continue;
                    pos=next;break;
                }
            }else{
                pos=start;
                if(!seps())error(ErrorRule::object_members);
            }
        }
        bump();
    }
public:
    Parser(std::string_view input,State& s):text(input),state(s){}
    bool run() {
        // 014310 only accepts Objects/Seps/EOL at root. Every invalid ordinary
        // exit is a throw helper; no normal plain-false exit was identified.
        while(pos.byte<text.size()){
            if(at('\n')){bump();continue;}
            if(text.substr(pos.byte,2)=="\r\n"){bump();bump();continue;}
            if(sep())continue;
            const auto start=pos;
            if(object()){object_action();continue;}
            pos=start;error(ErrorRule::grammar);
        }
        return true;
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
    return Parser(raw,state).run();
}
configuration::Node to_configuration_node(const Node& node,const BinaryIterationOrder& order) {
    std::vector<std::string> path;return convert(node,order,path);
}
} // namespace anadius::parser
