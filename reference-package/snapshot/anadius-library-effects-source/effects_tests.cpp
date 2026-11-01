#include "effects_test_fixture.hpp"
#include <climits>

namespace {
using namespace b12_test;
void guards_and_ownership(){
    rejects<std::invalid_argument>([]{le::Dialog d({nullptr,message});},"missing dialog converter rejected");
    rejects<std::invalid_argument>([]{le::Dialog d({convert,nullptr});},"missing dialog endpoint rejected");
    Graph g;
    rejects<std::invalid_argument>([&]{le::LibraryEffects e({},g.dialog,std::make_unique<Phase>(g.phase));},"null stream rejected");
    rejects<std::invalid_argument>([&]{le::LibraryEffects e(g.output,{},std::make_unique<Phase>(g.phase));},"null dialog owner rejected");
    rejects<std::invalid_argument>([&]{le::LibraryEffects e(g.output,g.dialog,{});},"null phase provider rejected");
    rejects<std::invalid_argument>([&]{le::LibraryEffects e(g.output,g.dialog,std::make_unique<Phase>(g.phase),{});},"null error policy rejected");
    require(g.output->str().empty()&&g.sdk.conversions.empty()&&g.sdk.messages.empty(),"construction does no log/UI/conversion I/O");
    auto stream=std::make_shared<b11_test::OwnedStream>();auto dialog=std::make_shared<le::Dialog>(dialog_api());
    std::weak_ptr<b11_test::OwnedStream> stream_ref=stream;std::weak_ptr<le::Dialog> dialog_ref=dialog;
    {le::LibraryEffects e(stream,dialog,std::make_unique<Phase>(g.phase));stream.reset();dialog.reset();
     require(!stream_ref.expired()&&!dialog_ref.expired(),"selected dependencies retained by owner");
     e.begin_load_log_0863e0();e.append_log_path_and_flush("owned");
     require(stream_ref.lock()->str()=="Loading owned\n","retained stream and buffer remain usable");}
    require(stream_ref.expired()&&dialog_ref.expired(),"selected dependencies released after effects owner");
    bootstrap_backend::Resources resources;resources.library_effects=std::make_unique<Phase>(g.phase);
    auto bound=le::bind_library_effects(std::move(resources),g.output,g.dialog);
    require(dynamic_cast<le::LibraryEffects*>(bound.library_effects.get())!=nullptr,"Resources helper installs selected policies");
    require(!bound.server&&!bound.online&&!bound.host,"binder invents no foreign or network providers");
}
void logging(){
    Graph g;g.effects.begin_load_log_0863e0();
    require(g.output->str()=="Loading "&&g.output->flushes==0,"exact prefix is inserted without premature newline/flush");
    g.effects.append_log_path_and_flush(std::string_view("a\0b",3));
    require(g.output->str()==std::string("Loading a\0b\n",12),"counted formatted path keeps embedded NUL");
    require(g.output->flushes==1&&g.phase->other_calls==0,"selected log paths bypass lower and flush once");
    Graph width;width.output->fill('.');width.output->width(10);width.effects.begin_load_log_0863e0();
    width.output->width(4);width.effects.append_log_path_and_flush("x");
    require(width.output->str()=="..Loading ...x\n"&&width.output->width()==0,"both formatted insertions consume width");
    Graph prefix;prefix.output->maximum=2;prefix.output->exceptions(std::ios::badbit);
    rejects<std::ios_base::failure>([&]{prefix.effects.begin_load_log_0863e0();},"prefix stream exception propagates");
    require(prefix.output->str()=="Lo"&&prefix.output->flushes==0,"failed prefix is not rolled back or flushed");
    Graph flush;flush.output->fail_flush=true;flush.output->exceptions(std::ios::badbit);flush.effects.begin_load_log_0863e0();
    rejects<std::ios_base::failure>([&]{flush.effects.append_log_path_and_flush("x");},"flush failure propagates");
    require(flush.output->str()=="Loading x\n"&&flush.output->flushes==1,"flush failure preserves completed line");
    Graph bad;bad.output->setstate(std::ios::failbit);bad.effects.begin_load_log_0863e0();bad.effects.append_log_path_and_flush("x");
    require(bad.output->str().empty()&&bad.output->flushes==0,"nonthrowing failed stream fabricates no output");
}
void dialog(){
    Graph g;g.effects.message_038bc0("ba\xc5\x9fl\xc4\xb1k","metin \xf0\x9f\x98\x80",0x12345678);
    require(g.sdk.conversions.size()==4&&g.sdk.messages.size()==1,"title and body each query/fill before one endpoint");
    require(g.sdk.conversions[0].input=="ba\xc5\x9fl\xc4\xb1k"&&g.sdk.conversions[1].input==g.sdk.conversions[0].input&&g.sdk.conversions[2].input=="metin \xf0\x9f\x98\x80","title precedes body conversion");
    for(const auto& c:g.sdk.conversions){require(c.cp==65001&&c.flags==0,"dialog uses explicit UTF8 flags zero");}
    require(!g.sdk.conversions[0].destination&&g.sdk.conversions[1].destination&&!g.sdk.conversions[2].destination&&g.sdk.conversions[3].destination,"typed query/fill destination shape retained");
    require(g.sdk.messages[0].window==nullptr&&g.sdk.messages[0].flags==0x12345678,"null owner and all flag bits forwarded");
    require(g.sdk.messages[0].title==L"ba\u015fl\u0131k"&&g.sdk.messages[0].text==L"metin \U0001f600","actual UTF8 conversion receives correct endpoint argument order");
    g.sdk.conversions.clear();g.effects.message_038bc0("","",0);
    require(g.sdk.conversions.empty()&&g.sdk.messages.back().text.empty()&&g.sdk.messages.back().title.empty(),"empty title/body skip conversion but still call endpoint");
    g.effects.message_038bc0(std::string("a\0b",3),std::string("c\0d",3),0x10);
    require(g.sdk.conversions[0].input.size()==3&&g.sdk.conversions[2].input.size()==3,"embedded NUL participates in explicit-length conversion");
    require(g.sdk.messages.back().title==L"a"&&g.sdk.messages.back().text==L"c","endpoint observes C-string termination at embedded NUL");
    Graph invalid;invalid.effects.message_038bc0(std::string(1,static_cast<char>(0xff)),"body",0);
    require(invalid.sdk.messages.size()==1&&invalid.sdk.messages[0].title==L"\ufffd","flags zero native replacement is preserved instead of strict rejection");
    ui=&g.sdk;
    for(int result:{0,IDOK,IDCANCEL,IDYES,IDNO,999}){g.sdk.result=result;g.effects.message_038bc0("","",0);}
    require(g.sdk.messages.size()==9,"all endpoint return values are discarded by library void contract");
}
void dialog_failures(){
    Graph zero;zero.sdk.hook=[](UINT,DWORD,LPCCH,int,LPWSTR,int){return 0;};zero.effects.message_038bc0("title","body",0x10);
    require(zero.sdk.conversions.size()==4&&zero.sdk.conversions[1].destination&&zero.sdk.conversions[1].capacity==0,"zero query still performs second conversion");
    require(zero.sdk.messages.size()==1&&zero.sdk.messages[0].title.empty()&&zero.sdk.messages[0].text.empty(),"zero conversion errors are ignored before endpoint");
    Graph shorter;shorter.sdk.hook=[](UINT,DWORD,LPCCH,int,LPWSTR out,int){if(!out)return 4;out[0]=L'Z';return 1;};
    shorter.effects.message_038bc0("title","body",0);
    require(shorter.sdk.conversions[1].capacity==4&&shorter.sdk.conversions[3].capacity==4,"fill retains first query capacities");
    require(shorter.sdk.messages[0].title==L"Z"&&shorter.sdk.messages[0].text==L"Z","zero initialized first-size tail terminates shorter fill at endpoint");
    Graph failed_fill;failed_fill.sdk.hook=[](UINT,DWORD,LPCCH,int,LPWSTR out,int){return out?0:3;};failed_fill.effects.message_038bc0("a","b",0);
    require(failed_fill.sdk.messages.size()==1&&failed_fill.sdk.messages[0].text.empty(),"fill failure does not invent an exception or skip endpoint");
    Graph first;first.sdk.hook=[](UINT,DWORD,LPCCH,int,LPWSTR,int)->int{throw std::runtime_error("controlled title conversion exception");};
    rejects<std::runtime_error>([&]{first.effects.message_038bc0("a","b",0);},"out-of-contract injected converter exception propagates");
    require(first.sdk.conversions.size()==1&&first.sdk.messages.empty(),"title exception stops body and endpoint");
    Graph negative;negative.sdk.hook=[](UINT,DWORD,LPCCH,int,LPWSTR,int){return -2;};
    rejects<std::domain_error>([&]{negative.dialog->show("x","",0);},"impossible negative Win32 result rejected explicitly");
}
void errors_and_phase(){
    Graph g;
    for(int code:{0,22,-7,INT_MIN}){
        try{g.effects.throw_runtime_00cd70(code);}catch(const std::system_error& e){require(e.code().value()==code&&&e.code().category()==&std::generic_category(),"selected runtime code/category retained");}
    }
    for(std::uint32_t code:{0U,1113U,0x80000005U,0xffffffffU}){
        try{g.effects.throw_filesystem_00cdb0(code);}catch(const std::system_error& e){require(static_cast<std::uint32_t>(e.code().value())==code&&&e.code().category()==&std::system_category(),"selected filesystem code/category retained");}
    }
    g.effects.wait_steps_036880(2);g.effects.wait_steps_036880(-1);g.effects.finish_flag_036820();
    require(g.phase->waits==std::vector<int>({2,-1})&&g.phase->finishes==1,"two phase methods retain exact owned lower route");
    require(g.phase->other_calls==0&&g.output->str().empty()&&g.sdk.messages.empty(),"throw/phase paths do not reach lower selected policies or UI/log");
}
}
int main(){try{guards_and_ownership();logging();dialog();dialog_failures();errors_and_phase();std::cout<<"PASS: "<<b12_test::checks<<" library Effects core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
