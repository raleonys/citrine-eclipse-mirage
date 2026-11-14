// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/runtime/thread_launch.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
using namespace laureondo::threads;
namespace {
int checks;
void require(bool ok, const char* message) { ++checks; if (!ok) throw std::runtime_error(message); }
struct Exit { Handle module; std::uint32_t code; };
struct Injected {};
struct Fake : Effects {
    std::vector<std::string> events;
    std::vector<std::unique_ptr<Context>> allocated;
    Context* slot{};
    Context* created{};
    bool fail_allocate{}, module_success{true}, fail_create{}, invoke_throws{}, invalid_throws{}, destroy_before_return{};
    Handle module{0x700}, result{0x900}, close_value{}, failure_handle{};
    std::uint32_t id{77}, error{123}, mapped{}, error_slot{}, exit_code{0xaabbccdd};
    int policy{2}, init_result{};
    void* seen_security{};
    std::uint64_t seen_stack{};
    std::uint32_t seen_flags{};
    std::uintptr_t seen_entry{}, seen_argument{};
    virtual std::uint32_t dispatch(std::uintptr_t, std::uintptr_t) { return exit_code; }
    Context* calloc_context_066848() override {
        events.push_back("calloc"); if (fail_allocate) return nullptr;
        allocated.push_back(std::make_unique<Context>()); return allocated.back().get();
    }
    void free_0668c0(Context* c) override {
        events.push_back(c ? "free-context" : "free-null");
        if (c) require(std::any_of(allocated.begin(),allocated.end(),[&](auto& p){return p.get()==c;}),"owned allocation freed");
    }
    bool get_module_handle_ex_w(std::uint32_t flags,std::uintptr_t address,Handle* out) override {
        events.push_back("get-module"); require(flags==4,"module flag FROM_ADDRESS");
        require(address==seen_entry,"module lookup uses entry address");
        if (module_success) *out=module; return module_success;
    }
    Handle create_thread(void* security,std::uint64_t stack,Context* context,
                         std::uint32_t flags,std::uint32_t* out) override {
        events.push_back("create"); seen_security=security;seen_stack=stack;seen_flags=flags;created=context;
        require(context->entry==seen_entry && context->argument==seen_argument,"CRT context stores application callback/payload");
        require(context->handle==0 && context->runtime_initialized==0,"calloc fields initially zero");
        if (fail_create) {context->handle=failure_handle;return 0;}
        *out=id;
        if(destroy_before_return)allocated.pop_back(); // synthetic worker already ended
        return result;
    }
    std::uint32_t last_error() override {events.push_back("last-error");return error;}
    void write_error_slot_05d724(std::uint32_t v) override {events.push_back("error-slot");error_slot=v;}
    void invalid_parameter_05c954() override {events.push_back("invalid");if(invalid_throws)throw Injected{};}
    void map_os_error_05d698(std::uint32_t v) override {events.push_back("map-error");mapped=v;}
    bool close_handle(Handle h) override {events.push_back("close");close_value=h;return false;}
    bool free_library(Handle h) override {events.push_back("free-library");require(h==module,"module failure cleanup");return false;}
    Context*& required_thread_slot_064b90() override {events.push_back("required-slot");return slot;}
    Context* optional_thread_context_064d08() override {events.push_back("optional-slot");return slot;}
    int runtime_policy_06c0d0() override {events.push_back("policy");return policy;}
    int initialize_runtime_067320(int v) override {events.push_back("initialize");require(v==1,"runtime init argument");return init_result;}
    void uninitialize_runtime_06736c() override {events.push_back("uninitialize");}
    std::uint32_t invoke_entry(std::uintptr_t fn,std::uintptr_t arg) override {
        events.push_back("invoke");require(slot && slot->entry==fn && slot->argument==arg,"PTD set before callback");
        if(invoke_throws)throw Injected{};return dispatch(fn,arg);
    }
    [[noreturn]] void free_library_and_exit_thread(Handle h,std::uint32_t code) override {events.push_back("free-exit");throw Exit{h,code};}
    [[noreturn]] void exit_thread(std::uint32_t code) override {events.push_back("exit");throw Exit{0,code};}
    bool has(const char* name) const {return std::find(events.begin(),events.end(),name)!=events.end();}
};
void test_launch() {
    for (bool fail_module : {false,true}) for (bool no_id : {false,true}) {
        Fake f;f.seen_entry=0x12345;f.seen_argument=0xcafe;f.module_success=!fail_module;
        std::uint32_t id=999;
        auto* sec=reinterpret_cast<void*>(0x321);
        const auto h=begin_thread_05e8e0(sec,0xffffffff,f.seen_entry,f.seen_argument,0x10004,no_id?nullptr:&id,f);
        require(h==f.result,"success returns thread handle");
        require(id==(no_id?999:77),"only caller-provided ID published on success");
        require(f.seen_security==sec && f.seen_stack==0xffffffffULL && f.seen_flags==0x10004,"CreateThread forwards security, unsigned stack, flags");
        require(f.created->handle==0,"returned handle is not assigned into context");
        require(f.created->module==(fail_module?0:f.module),"module lookup failure ignored");
        require(f.events==std::vector<std::string>{"calloc","free-null","get-module","create"},"success transfers context without cleanup");
    }
    for (bool handler_throws : {false,true}) {
        Fake f;f.invalid_throws=handler_throws;std::uint32_t id=999;
        try {require(begin_thread_05e8e0(nullptr,0,0,88,0,&id,f)==0,"null entry returns failure");require(!handler_throws,"handler expected return");}
        catch(const Injected&) {require(handler_throws,"invalid handler exception propagated");}
        require(id==999 && f.error_slot==22,"invalid entry sets observed error slot, leaves ID");
        require(f.events==std::vector<std::string>{"error-slot","invalid"},"invalid before allocation");
    }
    {Fake f;f.fail_allocate=true;std::uint32_t id=999;
     require(begin_thread_05e8e0(nullptr,0,1,2,0,&id,f)==0,"allocation failure");
     require(id==999 && f.events==std::vector<std::string>{"calloc","free-null"},"allocation failure does not map Win32 error");}
    {Fake f;f.destroy_before_return=true;f.seen_entry=8;f.seen_argument=9;std::uint32_t id=999;
     require(begin_thread_05e8e0(nullptr,0,8,9,0,&id,f)==f.result,"successful thread may destroy Context before CreateThread returns");
     require(f.allocated.empty() && id==77,"success publishes ID independently of freed Context");}
    for (Handle saved : {Handle{0},Handle{7},~Handle{0}}) for(bool no_module : {false,true}) {
        Fake f;f.fail_create=true;f.failure_handle=saved;f.module_success=!no_module;f.seen_entry=8;f.seen_argument=9;
        std::uint32_t id=999;
        require(begin_thread_05e8e0(nullptr,0,8,9,0,&id,f)==0,"CreateThread failure");
        std::vector<std::string> expected{"calloc","free-null","get-module","create","last-error","map-error"};
        if(saved)expected.push_back("close");if(!no_module)expected.push_back("free-library");expected.push_back("free-context");
        require(f.events==expected,"failure cleanup order, including nonzero invalid handle");
        require(id==999 && f.mapped==123,"OS error captured before cleanup, ID untouched");
        if(saved)require(f.close_value==saved,"failure closes every nonzero saved handle");
    }
}
void test_end() {
    constexpr std::array<Handle,7> handles{0,1,2,0x7fffffff,0x8000000000000000ULL,~Handle{1},~Handle{0}};
    for(auto handle:handles)for(auto module:handles)for(std::uint8_t initialized:{0,1,255}) {
        Fake f;Context c{};c.handle=handle;c.module=module;c.runtime_initialized=initialized;f.slot=&c;
        try {end_thread_05e9bc(0xfedcba98,f);}catch(const Exit& out) {
            const bool release_module=module!=0 && module!=~Handle{0};
            require(out.code==0xfedcba98 && out.module==(release_module?module:0),"end preserves unsigned code and module sentinel");
            std::vector<std::string> expected{"optional-slot"};
            if(initialized)expected.push_back("uninitialize");
            if(handle!=0 && handle!=~Handle{0})expected.push_back("close");
            expected.push_back(release_module?"free-exit":"exit");
            require(f.events==expected,"thread end cleanup order and all handle sentinels");
            require(!f.has("free-context"),"no inferred explicit free/PTD clear on exit");
        }
    }
    Fake f;
    try {common_end_thread_05e818(17,f);}catch(const Exit& out) {require(out.code==17 && out.module==0,"absent PTD context exits directly");}
    require(f.events==std::vector<std::string>{"optional-slot","exit"},"absent PTD order");
}
void test_entry() {
    for(int policy:{0,1,2,3})for(int init:{-1,0,1}) {
        Fake f;f.policy=policy;f.init_result=init;Context c{};c.entry=88;c.argument=99;c.runtime_initialized=7;
        try {thread_entry_05e7a8(&c,f);}catch(const Exit& out) {require(out.code==f.exit_code,"worker return code passed to thread exit");}
        require(c.runtime_initialized==(policy==2?static_cast<std::uint8_t>(init==0):7),"policy skips init-byte write outside mode2");
        require(f.has("initialize")== (policy==2),"runtime init only policy2");
        require(f.has("uninitialize")== (c.runtime_initialized!=0),"uninit follows stored byte");
        require(f.events.front()=="required-slot" && f.slot==&c,"PTD context set first and not explicitly cleared");
    }
    {Fake f;try {thread_entry_05e7a8(nullptr,f);}catch(const Exit& out){require(out.code==123,"null thread context exits with LastError");}
     require(f.events==std::vector<std::string>{"last-error","exit"},"null context before PTD");}
    {Fake f;f.invoke_throws=true;Context c{};
     try {thread_entry_05e7a8(&c,f);}catch(const Injected&) {require(f.slot==&c,"worker throw preserves observed PTD assignment");}
     require(!f.has("optional-slot")&&!f.has("uninitialize")&&!f.has("exit"),"no invented exception cleanup");}
}
// Two recovered layers are composed without creating an OS thread.
struct Bridge;
Bridge* active;
void first();void second();void extra(bool);
struct Bridge final : BootstrapThreadBridge {
    laureondo::system_services::Bootstrap bootstrap{*this};
    int first_count{},second_count{},true_count{},false_count{},deleted{},thread_cleanup{},between{};
    std::vector<std::unique_ptr<std::byte[]>> payloads;
    explicit Bridge(Effects& e):BootstrapThreadBridge(e) {active=this;}
    std::uintptr_t bind_application_entry(laureondo::system_services::ThreadEntry e) override {
        return e==laureondo::system_services::ThreadEntry::no_argument_0388a0?11:12;
    }
    int register_exit(laureondo::system_services::RegisteredCallback) override{return 0;}
    void install_terminate(laureondo::system_services::RegisteredCallback) override{}
    void setup_environment_019910() override{}
    void setup_console_038760() override{}
    void* log_insert_006880(void* p,std::uintptr_t) override{return p;}
    void log_finish_0088e0(void*) override{}
    void load_configuration_01c870() override{}
    void* allocate_04b0bc(std::size_t n) override {payloads.push_back(std::make_unique<std::byte[]>(n));return payloads.back().get();}
    void delete_04b0f8(void*,std::size_t) override {++deleted;}
    laureondo::system_services::NoArgumentFunction worker(laureondo::system_services::Worker w) override{return w==laureondo::system_services::Worker::first_029ea0?first:second;}
    laureondo::system_services::BooleanFunction boolean_worker_035850() override{return extra;}
    bool close_handle(laureondo::system_services::Handle) override{return true;}
    [[noreturn]] void throw_thread_error_04a648(int) override{throw Injected{};}
    void between_workers_00e310() override{++between;}
    void guard_return_017a30() override{}
    void cleanup_threads_04a47c() override{++thread_cleanup;}
    void cleanup_035ab0() override{}
    void cleanup_0366f0() override{}
    void cleanup_0102b0() override{}
    void terminate_log_038920() override{throw Injected{};}
    [[noreturn]] void unexpected_terminate_return_trap() override{throw Injected{};}
};
void first(){++active->first_count;}void second(){++active->second_count;}
void extra(bool b){if(b)++active->true_count;else ++active->false_count;}
struct IntegratedEffects final : Fake {
    std::vector<Context*> queued;
    bool get_module_handle_ex_w(std::uint32_t flags,std::uintptr_t address,Handle* out) override {
        require(flags==4 && (address==11||address==12),"bootstrap binds recovered application wrapper tokens");*out=module;return true;
    }
    Handle create_thread(void* security,std::uint64_t stack,Context* c,std::uint32_t flags,std::uint32_t* out) override {
        require(!security && stack==0 && flags==0,"bootstrap CRT launch arguments");queued.push_back(c);*out=77;return 99;
    }
    std::uint32_t dispatch(std::uintptr_t fn,std::uintptr_t argument) override {
        if(fn==11)return active->bootstrap.run_no_argument(reinterpret_cast<laureondo::system_services::NoArgumentPayload*>(argument));
        require(fn==12,"boolean trampoline token");return active->bootstrap.run_boolean(reinterpret_cast<laureondo::system_services::BooleanPayload*>(argument));
    }
};
void test_bootstrap_integration() {
    IntegratedEffects f;Bridge b(f);b.bootstrap.initialize();
    require(f.queued.size()==3 && b.true_count==1 && b.false_count==0,"bootstrap synchronous call precedes three queued workers");
    require(b.between==1 && b.first_count==0 && b.deleted==0,"launcher does not invoke/destroy worker payloads");
    for(auto* c:f.queued) {try{thread_entry_05e7a8(c,f);}catch(const Exit& out){require(out.code==0 && out.module==f.module,"both reconstructed trampolines return0 through CRT exit");}}
    require(b.first_count==1 && b.second_count==1 && b.true_count==1 && b.false_count==1,"correct workers and bool payload");
    require(b.thread_cleanup==3 && b.deleted==3,"application cleanup and payload deletion occur before CRT exit");
    require(std::count(f.events.begin(),f.events.end(),"uninitialize")==3,"runtime cleanup for all three synthetic workers");
}
}
int main(){try{test_launch();test_end();test_entry();test_bootstrap_integration();
 std::cout<<"PASS: "<<checks<<" thread/context/bootstrap checks (synthetic; no OS threads)\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected test exception\n";return 1;}}
