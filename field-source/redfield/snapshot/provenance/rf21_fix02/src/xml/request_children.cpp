#include "preloader_l/xml/request_parser.hpp"
#include "tinyxml2_dom.hpp"
#include <cstring>
namespace preloader_l { namespace xml {
namespace {
bool allowed_children(const tinyxml2::XMLElement* parent) noexcept {
    for (auto* child=parent->FirstChild();child;child=child->NextSibling()) {
        if (const auto* text=child->ToText()) {
            if (!dom::whitespace(text->Value())) return false;
        } else if (!child->ToElement() && !child->ToComment()) return false;
    }
    return true;
}
} // namespace
ChildTextStatus find_direct_child_text(TextSpan content,const char* name,TextSpan* value) noexcept {
    if (value) *value=TextSpan{};
    if (!value || !name || !*name || (!content.data&&content.size)) return ChildTextStatus::invalid_argument;
    try {
        if (!content.element || !content.document) return ChildTextStatus::malformed;
        const auto* parent=content.element;
        const auto& doc=content.document;
        if (content.direct_children ? !content.direct_children->valid_trivia : !allowed_children(parent))
            return ChildTextStatus::malformed;
        for (auto* e=parent->FirstChildElement();e;e=e->NextSiblingElement()) {
            if (std::strcmp(e->Name(),name)) continue;
            const tinyxml2::XMLNode* node=e->FirstChild();
            if (!node) return dom::decoded("",doc,value) ? ChildTextStatus::found:ChildTextStatus::malformed;
            const auto* txt=node->ToText();
            if (!txt || node->NextSibling()) return ChildTextStatus::malformed;
            const char* raw=txt->Value();
            const char* start=raw;
            while (*start==' '||*start=='\t'||*start=='\r'||*start=='\n'||*start=='\v'||*start=='\f') ++start;
            const char* end=start+std::strlen(start);
            while(end>start && (end[-1]==' '||end[-1]=='\t'||end[-1]=='\r'||end[-1]=='\n'||end[-1]=='\v'||end[-1]=='\f')) --end;
            *value=TextSpan{start,static_cast<size_t>(end-start),doc};
            return ChildTextStatus::found;
        }
        return ChildTextStatus::not_found;
    } catch (...) { return ChildTextStatus::malformed; }
}
DirectChildStatus next_direct_child(TextSpan content,size_t* cursor,DirectChildView* child) noexcept {
    if (child) *child=DirectChildView{};
    if (!cursor || !child || (!content.data&&content.size)) return DirectChildStatus::invalid_argument;
    try {
        if (!content.element || !content.document) return DirectChildStatus::malformed;
        const auto* parent=content.element;
        const auto& doc=content.document;
        if (content.direct_children ? !content.direct_children->valid_trivia : !allowed_children(parent))
            return DirectChildStatus::malformed;
        const tinyxml2::XMLElement* element=nullptr;
        if (content.direct_children) {
            if (*cursor<content.direct_children->count) element=content.direct_children->nodes[*cursor];
        } else {
            element=parent->FirstChildElement();
            for (size_t i=0;i<*cursor && element;++i) element=element->NextSiblingElement();
        }
        if (!element) return DirectChildStatus::end;
        if (!dom::decoded(element->Name(),doc,&child->name)) return DirectChildStatus::malformed;
        child->opening_tag=TextSpan{nullptr,0,doc,element};
        // Simple text nodes are already semantic; do not stringify them back
        // to entity-encoded XML just to have callers decode them again.
        const auto* first=element->FirstChild();
        if (first && first->ToText() && !first->NextSibling()) {
            if (!dom::decoded(first->Value(),doc,&child->content,element))
                return DirectChildStatus::malformed;
        } else {
            child->content=TextSpan{nullptr,0,doc,element};
        }
        child->self_closing=element->FirstChild()==nullptr;
        // A nested caller receives an owned index with one trivia validation;
        // repeated next_direct_child on that view is also linear overall.
        child->content.direct_children=dom::child_index(element);
        ++*cursor;
        return DirectChildStatus::found;
    } catch (...) { return DirectChildStatus::malformed; }
}
} }
