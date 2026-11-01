#include "pair_leaves.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <source_location>
#include <stdexcept>
#include <string>
#include <vector>
namespace q=anadius::crt_pair_leaves_recovered;
namespace h=anadius::crt_heap_recovered;
namespace v=anadius::vcrt_ptd_recovered;
namespace a=anadius::crt_argv_recovered;
namespace p=anadius::crt_argv_parse_recovered;
namespace {
unsigned checks=0;
void check(bool yes,std::source_location w=std::source_location::current()) {
    ++checks;if(!yes)throw std::runtime_error("pair leaf line "+std::to_string(w.line()));
}
struct Vcrt final:v::Effects {
    v::State& state;std::vector<std::string> log;bool free_result=true,mutate_index=false;
    explicit Vcrt(v::State& s):state(s){}
    bool fls_free_050b54(std::uint32_t i)override{check(i==17);log.push_back("fls-free");if(mutate_index)state.fls_index=99;return free_result;}
    void uninitialize_locks_050984()override{check(state.fls_index==v::invalid_fls_index);log.push_back("locks");}
    std::uint32_t fls_alloc_050b0c(v::FlsCallback)override{throw std::logic_error("unused alloc");}
    void* fls_get_050b9c(std::uint32_t)override{throw std::logic_error("unused get");}
    bool fls_set_050be4(std::uint32_t,void*)override{throw std::logic_error("unused set");}
    v::Ptd* calloc_05cae4(std::size_t,std::size_t)override{throw std::logic_error("unused calloc");}
    void free_05caec(void*)override{throw std::logic_error("unused free");}
    std::uint32_t last_error()override{throw std::logic_error("unused error");}
    void set_last_error(std::uint32_t)override{throw std::logic_error("unused error");}
    bool initialize_locks_05093c()override{throw std::logic_error("unused locks");}
    [[noreturn]]void abort_05f03c()override{throw std::logic_error("unused abort");}
};
void leaves() {
    for(auto value:{std::uintptr_t{0},std::uintptr_t{1},~std::uintptr_t{0},std::uintptr_t{0x12345678}}) {
        h::State shared{value,7,0x1234,0x5678};auto& alias=shared;
        check(q::uninitialize_heap_0704e8(shared));check(alias.heap_0a5a10==0);
        check(shared.new_mode_0a5a20==7&&shared.cookie_0a1580==0x1234&&shared.encoded_handler_0a5178==0x5678);
        check(q::uninitialize_heap_0704e8(alias));
    }
    check(q::success_006500());
    for(bool free_result:{false,true})for(bool mutate:{false,true}) {
        v::Ptd initial;initial.frame_state=-77;v::State state(initial);state.fls_index=17;
        Vcrt e(state);e.free_result=free_result;e.mutate_index=mutate;
        check(q::uninitialize_vcrt_0644f0(state,e));
        check(e.log==std::vector<std::string>({"fls-free","locks"}));
        check(state.fls_index==v::invalid_fls_index&&initial.frame_state==-77);
        e.log.clear();check(q::uninitialize_vcrt_0644f0(state,e));check(e.log==std::vector<std::string>({"locks"}));
    }
}
struct Commands final:q::CommandLineEffects {
    q::CommandLines& state;std::span<const char> narrow;std::span<const char16_t> wide;
    std::span<const char> before_a;std::span<const char16_t> before_w;
    std::vector<std::string> log;std::function<void()> in_w;
    explicit Commands(q::CommandLines& s):state(s),before_a(s.narrow.command_line),before_w(s.wide_0a5a08){}
    std::span<const char> get_command_line_a()override {
        log.push_back("A");check(state.narrow.command_line.data()==before_a.data());return narrow;
    }
    std::span<const char16_t> get_command_line_w()override {
        log.push_back("W");check(state.narrow.command_line.data()==narrow.data());check(state.narrow.command_line.size()==narrow.size());
        check(state.wide_0a5a08.data()==before_w.data());if(in_w)in_w();return wide;
    }
};
void command_order() {
    const char old_a[]="old",a0[]="",a1[]="own.exe alpha";const char16_t old_w[]=u"old",w0[]=u"",w1[]=u"own.exe wide";
    const std::array<std::span<const char>,3> aa{{{},a0,a1}};
    const std::array<std::span<const char16_t>,3> ww{{{},w0,w1}};
    for(auto av:aa)for(auto wv:ww) {
        a::State argv;argv.command_line=old_a;std::span<const char16_t> wide=old_w;q::CommandLines shared{argv,wide};Commands e(shared);e.narrow=av;e.wide=wv;
        check(q::initialize_command_lines_06f9bc(shared,e));check(e.log==std::vector<std::string>({"A","W"}));
        check(argv.command_line.data()==av.data()&&argv.command_line.size()==av.size());
        check(wide.data()==wv.data()&&wide.size()==wv.size());
    }
    a::State argv;argv.command_line=old_a;std::span<const char16_t> wide=old_w;q::CommandLines shared{argv,wide};Commands e(shared);e.narrow=a1;e.wide=w1;
    e.in_w=[&]{argv.command_line=old_a;wide=w0;};
    check(q::initialize_command_lines_06f9bc(shared,e));
    check(argv.command_line.data()==old_a&&wide.data()==w1); // no late A rewrite
}
struct Argv final:a::Effects,p::Effects {
    void* owned=nullptr;std::uint32_t error=0;unsigned null_frees=0;
    ~Argv(){std::free(owned);}
    void initialize_multibyte_06f5f8()override{}
    std::uint32_t module_filename_06ecdc(std::span<char,260> out)override{const char name[]="fallback.exe";std::copy(std::begin(name),std::end(name),out.begin());return 12;}
    anadius::crt_errors_recovered::Cell32 errno_05d724()override{return anadius::crt_errors_recovered::Cell32(&error);}
    void invalid_parameter_05c954()override{throw std::logic_error("unexpected invalid");}
    a::Allocation calloc_066848(std::uint64_t n,std::uint64_t size)override{
        check(owned==nullptr&&n<4096&&size==1);owned=std::calloc(std::size_t(n),1);check(owned!=nullptr);
        return {owned,{static_cast<std::byte*>(owned),std::size_t(n)}};
    }
    void free_0668c0(void* p)override{check(p==nullptr);++null_frees;}
    std::int32_t expand_06ecd4(std::span<char*>,a::Expansion&)override{throw std::logic_error("unused expansion");}
    std::int32_t is_mb_lead_06f9a8(std::int32_t)override{return 0;}
};
void argv_consumer() {
    const char command[]="own.exe alpha \"two words\"";const char16_t wide_command[]=u"own.exe wide";
    a::State argv;std::span<const char16_t> wide;q::CommandLines shared{argv,wide};Commands e(shared);e.narrow=command;e.wide=wide_command;
    check(q::initialize_command_lines_06f9bc(shared,e));Argv parser;
    check(a::configure_0638e8(argv,1,parser,parser)==0);
    check(argv.argc==3&&std::strcmp(argv.argv[0],"own.exe")==0&&std::strcmp(argv.argv[1],"alpha")==0&&std::strcmp(argv.argv[2],"two words")==0);
    check(argv.argv[3]==nullptr&&argv.command_line.data()==command&&wide.data()==wide_command);
    check(parser.null_frees==2); // allocation helper and final publication
}
}
int main(){try{leaves();command_order();argv_consumer();std::cout<<"PASS "<<checks<<" ACRT pair leaf checks;shared heap/VCRT/argv state;owned command buffers\n";return 0;}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
