// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/frame_read_test_fixture.hpp"
using namespace frame_test;
namespace {
struct Effects:read_test::Completion {
    s::ComposedReadContext* read=nullptr;
    void receive_body_025c40(s::ReceiveHandler& h,s::Error& e,std::uint64_t n,std::uint32_t flag)override{s::composed_receive_025c40(h,e,n,flag,*read);}
};
struct Bridge:s::FrameReadApi {
    using FrameReadApi::FrameReadApi;
    void construct_length_error_024390(s::ReadLengthError&,std::uintptr_t)override{r::unexpected();}
    [[noreturn]]void throw_length_error_023930(s::ReadLengthError&)override{r::unexpected();}
};
struct Loop {
    Fixture f;Effects effects;Bridge bridge{f.context};
    std::array<s::ReceiveStringBinding,1> strings{{{0x120a8,&f.base.buffer.value}}};
    std::array<s::ReceiveStreamBinding,1> streams{{{0x12010,&f.base.io.service,&f.base.io.implementation}}};
    s::ComposedReadContext read{f.buffer,f.base.io.context,effects,bridge,strings,streams};
    std::vector<std::string> messages;unsigned callbacks=0;
    Loop(){f.context.read=&read;effects.read=&read;f.base.io.implementation.state_08=0x10;
        f.base.io.effects.construct=[&](auto&,auto allocator){check(allocator==0x12078,"actual0280d0/025c40/021d10 literal allocator owner+78");};
        f.api.hook=[&](auto event){if(event=="begin"){messages.push_back(f.api.delivered);f.api.closure.reset();}};
    }
    void start(std::string initial){f.seed(initial,std::max<std::uint64_t>(15,initial.size()));s::start_frame_read_0280d0(0x12000,f.context);}
    void complete(std::string payload,std::int32_t error=0){
        // Controlled completion adapter: no claim to execute027710,023dd0,
        // handler scheduling or the worker here. Those have separate tests.
        auto operation=std::move(f.base.io.effects.operation);check(bool(operation),"pending actual021d10 allocation exists");
        check(payload.size()<=operation->io.requested_58,"fixture payload fits actual requested slice");
        auto bytes=f.base.buffer.memory.view(operation->buffer_data_50,payload.size());std::copy(payload.begin(),payload.end(),bytes.begin());
        auto h=operation->io.handler;operation->io.handler.control_a8=nullptr;h.error={error,{error==2?0x3333U:0x2222U}};h.transferred=payload.size();
        ++callbacks;s::invoke_receive_021a40(h,effects);
        // Consumer of a borrowed logical callback owns this release. No
        // rollback/unwind or original native control-block ABI is inferred.
        auto* const control=h.control_a8;
        if(control&&control->strong_08.fetch_sub(1)==1){control->dispose_slot_00();if(control->weak_0c.fetch_sub(1)==1)control->delete_slot_08();}
    }
    void eof(){complete("",2);check(!f.base.io.effects.operation&&f.owner.strong_08==1&&!f.owner.disposes,"EOF stops read/rearm and returns acquired source refs to initial owner");}
};
void retained(){
    for(auto capacity:{15ULL,31ULL,4095ULL}){Loop l;l.f.seed(std::string("a\0b\0tail",8),capacity);s::start_frame_read_0280d0(0x12000,l.f.context);
        check(l.f.base.io.effects.operation->io.requested_58==0&&!l.f.base.io.receive.calls,"prebuffered NUL match defers through actual zero-length receive publication");
        l.complete("");check(l.messages==std::vector<std::string>{"a"}&&l.f.base.buffer.text()==std::string("b\0tail",6),"first actual frame bridge consumes only first frame");
        l.complete("");check(l.messages==std::vector<std::string>({"a","b"})&&l.f.base.buffer.value.size_10==4+l.f.base.io.effects.operation->io.requested_58,"second frame leaves tail then actual R3 prepares next request using available capacity");
        l.eof();check(l.f.base.buffer.text()=="tail"&&l.callbacks==3&&l.f.api.restarts==0,"EOF commits unused prepare and preserves tail; recovered bridge used actual restart rather than test capture");
    }
}
void chunks(){
    for(unsigned split=0;split<=7;++split){Loop l;l.start("");const std::string payload("ab\0cd\0x",7);
        if(split)l.complete(payload.substr(0,split));
        if(split<7){while(l.f.base.io.effects.operation->io.requested_58==0)l.complete("");l.complete(payload.substr(split));}
        for(unsigned i=0;i<3&&l.messages.size()<2;++i)l.complete("");
        check(l.messages==std::vector<std::string>({"ab","cd"}),"all split points preserve exact frame payloads and order");
        l.eof();check(l.f.base.buffer.text()=="x"&&l.f.owner.strong_08==1&&!l.f.owner.disposes,"partial trailing frame survives EOF without delivery or leaked strong acquisition");
    }
    {Loop l;l.start("");l.complete(std::string("\0\0z",3));l.complete("");check(l.messages==std::vector<std::string>({"",""}),"empty NUL-delimited frames are delivered with empty copied strings");l.eof();check(l.f.base.buffer.text()=="z","trailing non-delimited byte retained");}
}
}
int main(){try{retained();chunks();std::cout<<"PASS: "<<checks<<" socket frame read integration checks; actual025480/0280d0/025c40/010040/008350/021d10/021a40 source loop, synthetic thread and controlled completion/environment adapters\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
