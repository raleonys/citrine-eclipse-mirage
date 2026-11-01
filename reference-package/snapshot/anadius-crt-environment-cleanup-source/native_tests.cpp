#include "environment_cleanup.hpp"
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <source_location>
#include <stdexcept>
#include <string>
#include <vector>
namespace e=anadius::crt_environment_recovered;
namespace c=anadius::crt_environment_cleanup_recovered;
namespace {
unsigned checks=0;
void check(bool value,std::source_location w=std::source_location::current()) {
    ++checks;if(!value)throw std::runtime_error("environment cleanup line "+std::to_string(w.line()));
}
struct Fixture final:e::Effects {
    std::vector<std::unique_ptr<char[]>> payloads;
    std::vector<std::unique_ptr<char*[]>> arrays;
    std::map<void*,std::string> names;
    std::set<void*> freed;
    std::vector<std::string> log;
    std::function<void(void*)> during_free;
    e::State narrow; e::Environment wide{},wide_initial{};
    c::State state{narrow,wide,wide_initial};
    e::Environment make(const std::string& name,bool wide_bytes=false) {
        auto table=std::make_unique<char*[]>(3);
        for(unsigned i=0;i<2;++i) {
            auto payload=std::make_unique<char[]>(wide_bytes?6:3);
            payload[0]=char('A'+i);payload[1]=wide_bytes?0:'x';
            if(wide_bytes){payload[2]='Z';payload[3]=0;payload[4]=0;payload[5]=0;}
            else payload[2]=0;
            table[i]=payload.get();names[payload.get()]=name+"."+std::to_string(i);
            payloads.push_back(std::move(payload));
        }
        table[2]=nullptr;auto* data=table.get();names[data]=name;arrays.push_back(std::move(table));
        return {data,3};
    }
    void free_0668c0(void* value)override {
        check(value!=nullptr&&names.contains(value));
        check(freed.insert(value).second);
        log.push_back(names.at(value));if(during_free)during_free(value);
        // Storage is owned by this fixture until teardown, so callback-driven
        // reads of later pointer slots remain lifetime-safe. Double free fails.
    }
    void initialize_multibyte_06f5f8()override{throw std::logic_error("unused init");}
    std::span<const char16_t> get_environment_strings_w()override{throw std::logic_error("unused acquire");}
    void free_environment_strings_w(std::span<const char16_t>)override{throw std::logic_error("unused release");}
    std::int32_t convert_06cd58(std::span<const char16_t>,std::int32_t,e::Buffer,std::int32_t)override{throw std::logic_error("unused conversion");}
    e::Buffer malloc_06a308(std::uint64_t)override{throw std::logic_error("unused malloc");}
    e::Environment calloc_pointers_066848(std::uint64_t,std::uint64_t)override{throw std::logic_error("unused calloc");}
    e::Buffer calloc_chars_066848(std::uint64_t,std::uint64_t)override{throw std::logic_error("unused calloc");}
    std::int32_t strcpy_s_05cff0(e::Buffer,std::span<const char>)override{throw std::logic_error("unused copy");}
    [[noreturn]]void invoke_watson_05c9a4()override{throw std::logic_error("unused watson");}
};
void add(std::vector<std::string>& out,const std::string& name){out.push_back(name+".0");out.push_back(name+".1");out.push_back(name);}
void configure(Fixture& f,unsigned mode,bool wide,std::vector<std::string>& first,std::vector<std::string>& last) {
    const auto initial=f.make(wide?"wi":"ni",wide),current=f.make(wide?"wc":"nc",wide);
    e::Environment selected{},saved{};
    switch(mode){
    case 0:break;
    case 1:selected=saved=initial;add(last,wide?"wi":"ni");break;
    case 2:selected=current;saved=initial;add(first,wide?"wc":"nc");add(last,wide?"wi":"ni");break;
    case 3:saved=initial;add(last,wide?"wi":"ni");break;
    case 4:selected=current;add(first,wide?"wc":"nc");break;
    default:throw std::logic_error("mode");
    }
    if(wide){f.wide=selected;f.wide_initial=saved;}else{f.narrow.current_0a5288=selected;f.narrow.initial_0a52a0=saved;}
}
void alias_matrix() {
    for(unsigned n=0;n<5;++n)for(unsigned w=0;w<5;++w) {
        Fixture f;std::vector<std::string> nf,wf,nl,wl;configure(f,n,false,nf,nl);configure(f,w,true,wf,wl);
        const auto nc=f.narrow.current_0a5288,ni=f.narrow.initial_0a52a0,wc=f.wide,wi=f.wide_initial;
        std::vector<std::string> wanted;for(const auto* x:{&nf,&wf,&nl,&wl})wanted.insert(wanted.end(),x->begin(),x->end());
        check(c::uninitialize_callback_0644a0(f.state,f));check(f.log==wanted);
        check(f.narrow.current_0a5288.data==nc.data&&f.narrow.initial_0a52a0.data==ni.data);
        check(f.wide.data==wc.data&&f.wide_initial.data==wi.data);
    }
}
void helper_and_mutations() {
    {
        Fixture f;const auto same=f.make("same");f.narrow.initial_0a52a0=same;
        auto view=same;view.slots=1;c::uninitialize_narrow_063f60(view,f.state,f);check(f.log.empty());
        // Alias comparison uses identity only, not the synthetic bound metadata.
        f.wide_initial=same;c::uninitialize_wide_063f7c(view,f.state,f);check(f.log.empty());
    }
    {
        Fixture f;const auto old=f.make("old"),replacement=f.make("replacement");
        const auto wi=f.make("wi",true),wc=f.make("wc",true),ni=f.make("ni");
        f.narrow={old,ni};f.wide=wc;f.wide_initial=wi;
        f.during_free=[&](void* p){if(p==old.data[0]){f.wide=wi;f.narrow.initial_0a52a0=replacement;}};
        c::uninitialize_environment_064010(f.state,f);
        std::vector<std::string> wanted;add(wanted,"old");add(wanted,"replacement");add(wanted,"wi");
        check(f.log==wanted);check(f.narrow.initial_0a52a0.data==replacement.data&&f.wide.data==wi.data);
    }
    {
        Fixture f;auto current=f.make("current",true),replacement=f.make("new",true);
        f.wide=current;f.during_free=[&](void* p){if(p==current.data[0])current.data[1]=replacement.data[1];};
        c::uninitialize_wide_063f7c(f.wide,f.state,f);
        check(f.log==std::vector<std::string>({"current.0","new.1","current"}));
    }
    {
        Fixture f;const auto ni=f.make("ni"),first=f.make("first",true),last=f.make("last",true);
        f.narrow={ni,ni};f.wide=f.wide_initial=first;
        f.during_free=[&](void* p){if(p==ni.data)f.wide_initial=last;};
        c::uninitialize_environment_064010(f.state,f);
        std::vector<std::string> wanted;add(wanted,"ni");add(wanted,"last");check(f.log==wanted);
    }
    {
        Fixture f;auto invalid=f.make("invalid");invalid.slots=1;f.narrow.current_0a5288=invalid;
        bool rejected=false;try{c::uninitialize_environment_064010(f.state,f);}catch(const std::domain_error&){rejected=true;}
        check(rejected);check(f.log==std::vector<std::string>({"invalid.0"}));
        check(f.narrow.current_0a5288.data==invalid.data);
    }
}
}
int main(){try{alias_matrix();helper_and_mutations();std::cout<<"PASS "<<checks<<" environment cleanup checks;25 narrow/wide alias pairs;owned pointer vectors and mutation ordering\n";return 0;}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
