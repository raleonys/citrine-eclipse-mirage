#include "preloader_l/xml/request_parser.hpp"
#include <string>
#include <cstring>
#include <memory>
#include <cstdio>
using namespace preloader_l::xml;
int main() {
    const size_t n=4096;
    std::string xml="<LSX><Request id=\"1\"><GetConfig><!-- retained comment -->";
    for (size_t i=0;i<n;++i) xml += "<Value>one</Value>";
    xml+="</GetConfig></Request></LSX>";
    RequestView v{};
    if(parse_request_view(xml.data(),xml.size(),&v)!=RequestStatus::ok) return 1;
    if(!v.command_content.direct_children || !v.command_content.direct_children->valid_trivia ||
       v.command_content.direct_children->count!=n) return 2;
    auto owned=v.command_content;
    xml.assign(xml.size(),'X');
    size_t cursor=0;
    DirectChildView child{};
    for(size_t i=0;i<n;++i) {
        auto rc=next_direct_child(owned,&cursor,&child);
        if(rc!=DirectChildStatus::found || !span_equals(child.name,"Value") || !span_equals(child.content,"one") || !child.content.direct_children) {
            std::fprintf(stderr,"iteration=%zu rc=%d name=%.*s content=%.*s doc=%d childvalid=%d count=%zu\n",i, int(rc), int(child.name.size),child.name.data?child.name.data:"",int(child.content.size),child.content.data?child.content.data:"",bool(child.content.document),child.content.direct_children?child.content.direct_children->valid_trivia:0,owned.direct_children->count);
            return 3;
        }
    }
    if(cursor!=n || next_direct_child(owned,&cursor,&child)!=DirectChildStatus::end) return 4;
    const char* bad="<LSX><Request id=\"1\"><GetConfig>NOT_WHITESPACE<Value/></GetConfig></Request></LSX>";
    if(parse_request_view(bad,std::strlen(bad),&v)!=RequestStatus::ok) return 5;
    if(!v.command_content.direct_children || v.command_content.direct_children->valid_trivia) return 6;
    cursor=0;
    if(next_direct_child(v.command_content,&cursor,&child)!=DirectChildStatus::malformed) return 7;
    TextSpan text{};
    if(find_direct_child_text(v.command_content,"Value",&text)!=ChildTextStatus::malformed) return 8;
    const char* nested="<LSX><Request><GetConfig><A><B/><B/></A></GetConfig></Request></LSX>";
    if(parse_request_view(nested,std::strlen(nested),&v)!=RequestStatus::ok) return 9;
    cursor=0;
    if(next_direct_child(v.command_content,&cursor,&child)!=DirectChildStatus::found || !span_equals(child.name,"A")) return 10;
    auto inner=child.content; cursor=0;
    if(!inner.direct_children || inner.direct_children->count!=2) return 11;
    for(int i=0;i<2;++i) if(next_direct_child(inner,&cursor,&child)!=DirectChildStatus::found || !span_equals(child.name,"B")) return 12;
    if(next_direct_child(inner,&cursor,&child)!=DirectChildStatus::end) return 13;
    return 0;
}
