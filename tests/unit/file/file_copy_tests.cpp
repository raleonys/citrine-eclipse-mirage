// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/platform/file_copy.hpp"
#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>

namespace {
using namespace laureondo::file_copy;
std::size_t checks=0;
void require(bool condition,const char* message) {
    ++checks;if(!condition)throw std::runtime_error(message);
}
struct CloseAbort {};
struct Fake final : SystemCalls {
    std::vector<std::string> trace;
    std::deque<Result> copies{{false,80},{true,0}};
    std::deque<OpenResult> opens{{11,0},{22,0}};
    std::map<Handle,Identity> ids{{11,{7,{1}}},{22,{7,{2}}}};
    std::map<Handle,std::uint32_t> id_errors,legacy_errors,time_errors;
    std::map<Handle,LegacyIdentity> old_ids{{11,{7,0x12345678,0xabcdef01}},{22,{7,0x12345678,0xabcdef02}}};
    std::map<Handle,std::int64_t> times{{11,100},{22,100}};
    Handle fail_close=invalid_handle;
    Result copy(const Path& a,const Path& b,bool fail) override {
        require(a==u"source"&&b==u"target","source/destination order");
        trace.push_back(fail?"copy:exclusive":"copy:overwrite");
        require(!copies.empty(),"unexpected additional copy");auto r=copies.front();copies.pop_front();return r;
    }
    OpenResult open_attributes(const Path& path,std::uint32_t share) override {
        trace.push_back((path==u"source"?"open:source:":"open:target:")+std::to_string(share));
        require(!opens.empty(),"unexpected additional open");auto r=opens.front();opens.pop_front();return r;
    }
    std::uint32_t file_id_info(Handle h,Identity& out) override {
        trace.push_back("id:"+std::to_string(h));const auto error=id_errors[h];if(!error)out=ids[h];return error;
    }
    std::uint32_t legacy_file_info(Handle h,LegacyIdentity& out) override {
        trace.push_back("legacy:"+std::to_string(h));const auto error=legacy_errors[h];if(!error)out=old_ids[h];return error;
    }
    std::uint32_t last_write(Handle h,std::int64_t& out) override {
        trace.push_back("time:"+std::to_string(h));const auto error=time_errors[h];if(!error)out=times[h];return error;
    }
    bool close(Handle h) override {trace.push_back("close:"+std::to_string(h));return h!=fail_close;}
    [[noreturn]] void abort_close_failure() override {trace.push_back("abort");throw CloseAbort{};}
};
Result run(Fake& f,std::uint32_t options) {return copy_file_048d20(u"source",u"target",options,f);}
void result_is(Result r,bool copied,std::uint32_t error) {
    require(r.copied==copied&&r.error==error,"copy result/error differs from RE branch");
}
void basic_options() {
    for(auto r:{Result{true,0},Result{false,0},Result{false,5},Result{false,80}}) {
        Fake f;f.copies={r};const auto actual=run(f,2);result_is(actual,r.copied,r.error);
        require(f.trace==std::vector<std::string>{"copy:overwrite"},"overwrite must not enter identity/attribute branches");
    }
    for(auto r:{Result{true,0},Result{false,0},Result{false,5},Result{false,80},Result{false,183}}) {
        Fake f;f.copies={r};const auto actual=run(f,0);result_is(actual,r.copied,r.error);
        require(f.trace==std::vector<std::string>{"copy:exclusive"},"none preserves initial copy error");
    }
    Fake skip;result_is(run(skip,1),false,0);
    require(skip.trace==std::vector<std::string>{"copy:exclusive","open:source:3","open:target:3","id:11","id:22","close:22","close:11"},"skip complete identity/close sequence");
    Fake same;same.ids[22]=same.ids[11];result_is(run(same,1),false,32);
    // Every low-nibble combination is retained; high option bits are ignored.
    for(std::uint32_t mode=0;mode<16;++mode) {
        Fake baseline;const auto original=run(baseline,mode);
        for(std::uint32_t high:{0x10u,0x120u,0xfffffff0u}) {
            Fake f;const auto actual=run(f,high|mode);
            result_is(actual,original.copied,original.error);
            require(f.trace==baseline.trace,"options mask must retain low nibble only");
        }
        if(mode!=0&&mode!=1&&mode!=2)
            require(baseline.trace[1]=="open:source:1","non-skip modes use FILE_SHARE_READ only");
    }
}
void update_cases() {
    for(auto second:{Result{true,0},Result{false,5}}) {
        Fake f;f.times[22]=50;f.copies={{false,80},second};result_is(run(f,4),second.copied,second.error);
        require(f.trace==std::vector<std::string>{"copy:exclusive","open:source:1","open:target:1","time:11","time:22","close:22","close:11","copy:overwrite"},"update closes both handles before overwrite");
    }
    for(auto target:{100LL,101LL}) {
        Fake f;f.times[22]=target;result_is(run(f,4),false,0);
        require(f.trace==std::vector<std::string>{"copy:exclusive","open:source:1","open:target:1","time:11","time:22","id:11","id:22","close:22","close:11"},"not-newer update still checks identity");
    }
    Fake same;same.ids[22]=same.ids[11];result_is(run(same,4),false,32);
    for(auto source:{std::numeric_limits<std::int64_t>::min(),std::int64_t{-1},std::int64_t{0},std::numeric_limits<std::int64_t>::max()})
        for(auto target:{std::numeric_limits<std::int64_t>::min(),std::int64_t{-1},std::int64_t{0},std::numeric_limits<std::int64_t>::max()}) {
            Fake f;f.times[11]=source;f.times[22]=target;
            result_is(run(f,4),target<source,0);
        }
    for(Handle h:{11,22}) {
        Fake f;f.time_errors[h]=123;result_is(run(f,4),false,123);
        const auto expected=h==11
            ?std::vector<std::string>{"copy:exclusive","open:source:1","open:target:1","time:11","close:22","close:11"}
            :std::vector<std::string>{"copy:exclusive","open:source:1","open:target:1","time:11","time:22","close:22","close:11"};
        require(f.trace==expected,"time failure short-circuit and reverse cleanup");
    }
}
void open_failures() {
    Fake source;source.opens={{invalid_handle,2}};result_is(run(source,1),false,2);
    require(source.trace==std::vector<std::string>{"copy:exclusive","open:source:3"},"source open failure has no cleanup handle");
    Fake target;target.opens={{11,0},{invalid_handle,5}};result_is(run(target,1),false,5);
    require(target.trace==std::vector<std::string>{"copy:exclusive","open:source:3","open:target:3","close:11"},"destination failure closes only source");
    Fake zero;zero.opens={{invalid_handle,0},{invalid_handle,0}};result_is(run(zero,1),false,32);
    require(zero.trace==std::vector<std::string>{"copy:exclusive","open:source:3","open:target:3","id:-1","id:-1"},"original invalid-handle/zero-error corner is not normalized");
}
void identity_cases() {
    for(auto error:{0u,1u,5u,50u,87u,123u}) {
        Fake f;f.id_errors[11]=error;Identity out{};
        const auto actual=file_identity_04895c(11,out,f);
        require(actual==((error==50||error==87)?0:error),"FileIdInfo fallback error selection");
        if(error==50||error==87) {
            require(f.trace==std::vector<std::string>{"id:11","legacy:11"},"legacy fallback limited to50/87");
            require(out.volume==7&&out.file_id==std::array<std::uint8_t,16>{0x78,0x56,0x34,0x12,0x01,0xef,0xcd,0xab,0,0,0,0,0,0,0,0},"legacy file ID byte order and zero extension");
        }else require(f.trace==std::vector<std::string>{"id:11"},"other FileIdInfo errors never fall back");
    }
    for(auto error:{50u,87u}) {
        Fake f;f.id_errors[11]=error;f.legacy_errors[11]=123;Identity out{};
        require(file_identity_04895c(11,out,f)==123,"legacy failure is returned unchanged");
    }
    for(Handle h:{11,22}) {
        Fake f;f.id_errors[h]=5;result_is(run(f,1),false,5);
        const auto expected=h==11
            ?std::vector<std::string>{"copy:exclusive","open:source:3","open:target:3","id:11","close:22","close:11"}
            :std::vector<std::string>{"copy:exclusive","open:source:3","open:target:3","id:11","id:22","close:22","close:11"};
        require(f.trace==expected,"identity failure short-circuit and cleanup");
    }
    for(unsigned byte=0;byte<16;++byte) {
        Fake f;f.ids[22]=f.ids[11];f.ids[22].file_id[byte]^=1;result_is(run(f,1),false,0);
    }
    Fake volume;volume.ids[22]=volume.ids[11];++volume.ids[22].volume;result_is(run(volume,1),false,0);
    Fake fallback;fallback.id_errors[11]=50;fallback.id_errors[22]=87;fallback.old_ids[22]=fallback.old_ids[11];result_is(run(fallback,1),false,32);
}
void fatal_cleanup() {
    for(Handle h:{11,22})for(std::uint32_t mode:{1u,4u}) {
        Fake f;f.fail_close=h;if(mode==4)f.times[22]=50;
        try{(void)run(f,mode);require(false,"close failure must not return");}catch(const CloseAbort&){}
        require(f.trace.back()=="abort","close failure immediately enters fatal boundary");
        require(f.copies.size()==1,"no overwrite after close failure");
        if(h==22)require(f.trace[f.trace.size()-2]=="close:22","destination close failure precedes source close");
        else require(f.trace[f.trace.size()-3]=="close:22"&&f.trace[f.trace.size()-2]=="close:11","source close follows destination");
    }
    Fake f;f.opens={{11,0},{invalid_handle,5}};f.fail_close=11;
    try{(void)run(f,1);require(false,"failed cleanup after open error must abort");}catch(const CloseAbort&){}
}
}
int main() {
    try {basic_options();update_cases();open_failures();identity_cases();fatal_cleanup();
        std::cout<<"file copy recovered branches: PASS ("<<checks<<" checks)\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
