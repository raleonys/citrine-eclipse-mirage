// SPDX-License-Identifier: GPL-3.0-only

#include "reader_policy.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace preloader_l::xml_reader_bridge::detail {
namespace {
bool space(unsigned char c){return c==' '||(c>=9&&c<=13);}
bool name_start(unsigned char c){return c>=128||(c>='A'&&c<='Z')||(c>='a'&&c<='z')||c==':'||c=='_';}
bool name_part(unsigned char c){return name_start(c)||(c>='0'&&c<='9')||c=='.'||c=='-';}
constexpr const char* labels[]={"XML_SUCCESS","","","","","",
    "XML_ERROR_PARSING_ELEMENT","XML_ERROR_PARSING_ATTRIBUTE","XML_ERROR_PARSING_TEXT",
    "XML_ERROR_PARSING_CDATA","XML_ERROR_PARSING_COMMENT","XML_ERROR_PARSING_DECLARATION",
    "XML_ERROR_PARSING_UNKNOWN","XML_ERROR_EMPTY_DOCUMENT","XML_ERROR_MISMATCHED_ELEMENT",
    "XML_ERROR_PARSING","","","XML_ELEMENT_DEPTH_EXCEEDED"};
std::string diagnostic(int code,int line,std::string_view prefix={},std::string_view value={}) {
    char header[160];
    std::snprintf(header,sizeof header,"Error=%s ErrorID=%d (0x%x) Line number=%d",labels[code],code,code,line);
    std::string result(header);
    if(!prefix.empty()){result+=": ";result+=prefix;result.append(value.substr(0,999-result.size()));}
    if(result.size()>999)result.resize(999);
    return result;
}
template<class T> class Stack {
    std::array<T,8> local_;
    std::vector<T> extra_;
    std::size_t size_=0;
public:
    std::size_t size()const{return size_;}
    T& top(){return size_<=local_.size()?local_[size_-1]:extra_.back();}
    void push(T value){if(size_<local_.size())local_[size_]=value;else extra_.push_back(value);++size_;}
    void pop(){if(size_>local_.size())extra_.pop_back();--size_;}
};
struct Frame {std::string_view name;int line=0;pugi::xml_node node;};
// This scanner owns project compatibility decisions, not a vendor DOM. The
// normal path validates then uses pugixml's parser. Only accepted legacy forms
// outside its grammar are constructed through pugixml's public DOM API.
class Scanner {
    std::string_view input_;
    std::size_t at_=0;
    int line_=1;
    Stack<Frame> stack_;
    bool declarations_only_=true;
    pugi::xml_document* output_;
    Result result_;
    char peek()const{return at_<input_.size()?input_[at_]:0;}
    void consume(std::size_t end) {
        const auto count=std::count(input_.begin()+at_,input_.begin()+end,'\n');
        line_=count>std::numeric_limits<int>::max()-line_?std::numeric_limits<int>::max():line_+static_cast<int>(count);
        at_=end;
    }
    void whitespace(){while(space(static_cast<unsigned char>(peek()))) {
        if(peek()=='\v'||peek()=='\f')result_.construct=true;
        consume(at_+1);
    }}
    std::string_view name() {
        const auto start=at_;
        if(name_start(static_cast<unsigned char>(peek()))) {
            ++at_;while(name_part(static_cast<unsigned char>(peek())))++at_;
        }
        return input_.substr(start,at_-start);
    }
    void fail(int code,int line,std::string_view prefix={},std::string_view value={}) {
        result_.code=code;result_.diagnostic=diagnostic(code,line,prefix,value);
    }
    pugi::xml_node parent(){return stack_.size()?stack_.top().node:*output_;}
    pugi::xml_node node(pugi::xml_node_type type,std::string_view value) {
        auto created=parent().append_child(type);
        if(!created||!(type==pugi::node_element?created.set_name(value.data(),value.size()):
                                              created.set_value(value.data(),value.size())))throw std::bad_alloc();
        return created;
    }
    void attribute(pugi::xml_node element,std::string_view key,std::string_view value) {
        auto created=element.append_attribute("");
        if(!created||!created.set_name(key.data(),key.size())||!created.set_value(value.data(),value.size()))
            throw std::bad_alloc();
    }
    void text(std::size_t start,int initial_line) {
        const auto end=input_.find('<',start);
        if(end==input_.npos){fail(8,initial_line);return;}
        if(end+1==input_.size()){fail(15,initial_line);return;}
        if(output_)node(pugi::node_pcdata,input_.substr(start,end-start));
        // Whitespace identification temporarily looked ahead. Restore the text
        // start so its line contribution is counted exactly once.
        consume(end);
        if(!stack_.size())declarations_only_=false;
    }
    void delimited(std::size_t header,std::string_view terminator,int code,int initial_line,
                   pugi::xml_node_type type,bool declaration=false) {
        at_+=header;const auto begin=at_;
        const auto end=input_.find(terminator,begin);
        if(end==input_.npos){fail(code,initial_line);return;}
        auto value=input_.substr(begin,end-begin);
        consume(end+terminator.size());
        if(declaration&&(stack_.size()||!declarations_only_)) {
            auto normalized=normalize_newlines(value);
            fail(11,initial_line,"XMLDeclaration value=",normalized.empty()?value:std::string_view(normalized));return;
        }
        if(output_) {
            if(type==pugi::node_pi) {
                // Neither interface exposes PI/unknown payloads; preserve their
                // non-text position (not a comment, which GetText skips).
                auto created=parent().append_child(pugi::node_pi);
                if(!created||!created.set_name("legacy"))throw std::bad_alloc();
            }else node(type,value);
        }
        if(!declaration&&!stack_.size())declarations_only_=false;
    }
    bool attributes(std::string_view element,int element_line,pugi::xml_node native,
                    bool& closed,bool closing) {
        std::array<std::string_view,8> local;
        std::size_t count=0;
        std::unordered_set<std::string_view> many;
        std::vector<std::pair<std::string_view,std::string_view>> closing_attributes;
        for(;;) {
            whitespace();
            if(!peek()){fail(6,element_line,"XMLElement name=",element);return false;}
            if(name_start(static_cast<unsigned char>(peek()))) {
                const auto attr_line=line_;auto key=name();whitespace();
                if(peek()!='='){fail(7,attr_line,"XMLElement name=",element);return false;}
                ++at_;whitespace();const auto quote=peek();
                if(quote!='\''&&quote!='"'){fail(7,attr_line,"XMLElement name=",element);return false;}
                const auto start=++at_;auto end=input_.find(quote,start);
                if(end==input_.npos){fail(7,attr_line,"XMLElement name=",element);return false;}
                const auto value=input_.substr(start,end-start);consume(end+1);
                bool duplicate=false;
                if(count<local.size()) {
                    duplicate=std::find(local.begin(),local.begin()+count,key)!=local.begin()+count;
                    local[count]=key;
                }else {
                    if(count==local.size())many.insert(local.begin(),local.end());
                    duplicate=!many.insert(key).second;
                }
                ++count;
                if(duplicate){fail(7,attr_line,"XMLElement name=",element);return false;}
                if(output_) {
                    if(closing)closing_attributes.emplace_back(key,value);
                    else attribute(native,key,value);
                }
            }else if(peek()=='>'){++at_;return true;}
            else if(input_.substr(at_).starts_with("/>")) {
                at_+=2;closed=true;
                // A slash-closed closing spelling is a retained legacy leaf.
                if(closing) {
                    result_.construct=true;
                    if(output_) {
                        auto created=node(pugi::node_element,element);
                        for(auto [key,value]:closing_attributes)attribute(created,key,value);
                    }
                }
                return true;
            }else{fail(6,element_line);return false;}
        }
    }
    void element(int initial_line) {
        ++at_;const auto before=at_;whitespace();
        if(at_!=before)result_.construct=true;
        const bool closing=peek()=='/';if(closing)++at_;
        auto label=name();
        if(label.empty()){fail(15,initial_line);return;}
        pugi::xml_node native;
        if(output_&&!closing)native=node(pugi::node_element,label);
        bool closed=false;
        if(!attributes(label,initial_line,native,closed,closing))return;
        if(!stack_.size())declarations_only_=false;
        if(closed)return;
        if(closing) {
            // Closing-tag attributes are accepted by the reference but are not
            // members of the parent's attribute list.
            if(!stack_.size()){result_.end=before-1;at_=input_.size();return;}
            const auto parent=stack_.top();
            if(parent.name!=label){fail(14,parent.line,"XMLElement name=",parent.name);return;}
            stack_.pop();return;
        }
        if(at_==input_.size()){fail(14,initial_line,"XMLElement name=",label);return;}
        if(stack_.size()>=498){fail(18,line_,"Element nesting is too deep.");return;}
        stack_.push({label,initial_line,native});
    }
public:
    Scanner(std::string_view input,pugi::xml_document* output):input_(input),output_(output){}
    Result run() {
        whitespace();
        if(input_.substr(at_).starts_with("\xef\xbb\xbf"))at_+=3;
        result_.begin=at_;result_.end=input_.size();
        if(!peek()){fail(13,0);return result_;}
        while(at_<input_.size()&&!result_.code) {
            const auto text_start=at_;const auto text_line=line_;
            whitespace();if(!peek())break;
            const auto initial_line=line_;
            auto tail=input_.substr(at_);
            if(peek()!='<') {at_=text_start;line_=text_line;text(text_start,initial_line);}
            else if(tail.starts_with("<?"))delimited(2,"?>",11,initial_line,pugi::node_pi,true);
            else if(tail.starts_with("<!--"))delimited(4,"-->",10,initial_line,pugi::node_comment);
            else if(tail.starts_with("<![CDATA["))delimited(9,"]]>",9,initial_line,pugi::node_cdata);
            else if(tail.starts_with("<!")) {
                result_.construct=true;delimited(2,">",12,initial_line,pugi::node_pi);
            }else element(initial_line);
        }
        if(!result_.code&&stack_.size())fail(15,stack_.top().line);
        return result_;
    }
};
}
std::string normalize_newlines(std::string_view value) {
    const auto first=value.find('\r');
    if(first==value.npos)return {};
    std::string result;result.reserve(value.size());
    for(std::size_t at=0;at<value.size();++at) {
        const char c=value[at];
        if(c=='\r'||c=='\n') {
            if(at+1<value.size()&&((c=='\r'&&value[at+1]=='\n')||(c=='\n'&&value[at+1]=='\r')))++at;
            result+='\n';
        }else result+=c;
    }
    return result;
}
Result inspect(std::string_view value){return Scanner(value,nullptr).run();}
void construct(pugi::xml_document& output,std::string_view value) {
    auto result=Scanner(value,&output).run();
    if(result.code)throw std::logic_error("XML compatibility construction disagreed with validation");
}
}
