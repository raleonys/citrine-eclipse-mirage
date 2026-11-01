#include "native_entries.hpp"
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace n=anadius::bootstrap_native_recovered;
namespace b=anadius::recovered;
namespace t=anadius::threads_recovered;
namespace {
unsigned checks;
void check(bool value) { ++checks; if(!value) throw std::runtime_error("bootstrap native check failed"); }
std::vector<int> events;
std::atomic<bool> entered{},release_worker{};
void first() { events.push_back(1); SetLastError(41); }
void second() { events.push_back(2); SetLastError(42); }
void boolean(bool value) { events.push_back(value?3:4); SetLastError(value?43:44); }
void throws_worker() { throw std::runtime_error("synthetic worker exception"); }
void blocked_worker() { entered.store(true); while(!release_worker.load()) std::this_thread::yield(); }
struct Fault { n::LifecycleFault fault; };
struct Errors : n::Errors {
    [[noreturn]] void lifecycle(n::LifecycleFault f) override { throw Fault{f}; }
    [[noreturn]] void callback_exception(n::Entry entry,std::exception_ptr exception) noexcept override {
        if(!exception) std::_Exit(97);
        try { std::rethrow_exception(exception); } catch(const std::exception&) {} catch(...) { std::_Exit(98); }
        // Terminal checks also verify callback classification and no live lease.
        const auto expected=expected_entry;
        std::_Exit(entry==expected&&n::active_calls()==0?83:96);
    }
    n::Entry expected_entry=n::Entry::NoArgument;
};
struct Threads : t::Effects {
    std::vector<t::Context*> contexts;
    unsigned null_frees{};
    ~Threads() override { for(auto* c:contexts) delete c; }
    t::Context* calloc_context_066848() override { auto* c=new t::Context{}; contexts.push_back(c); return c; }
    void free_0668c0(t::Context* c) override { check(!c); ++null_frees; }
    bool get_module_handle_ex_w(std::uint32_t f,std::uintptr_t a,t::Handle* out) override {
        check(f==4&&a!=0&&*out==0); return false;
    }
    t::Handle create_thread(void* security,std::uint64_t stack,t::Context* c,std::uint32_t flags,std::uint32_t* id) override {
        check(!security&&stack==0&&flags==0&&c==contexts.back()); *id=static_cast<std::uint32_t>(contexts.size()+12); return contexts.size()+100;
    }
    std::uint32_t last_error() override { throw std::logic_error("unused last_error"); }
    void write_error_slot_05d724(std::uint32_t) override { throw std::logic_error("unused error_slot"); }
    void invalid_parameter_05c954() override { throw std::logic_error("unused invalid_parameter"); }
    void map_os_error_05d698(std::uint32_t) override { throw std::logic_error("unused map_error"); }
    bool close_handle(t::Handle) override { throw std::logic_error("unused close"); }
    bool free_library(t::Handle) override { throw std::logic_error("unused module"); }
    t::Context*& required_thread_slot_064b90() override { throw std::logic_error("unused required PTD"); }
    t::Context* optional_thread_context_064d08() override { throw std::logic_error("unused optional PTD"); }
    int runtime_policy_06c0d0() override { throw std::logic_error("unused runtime policy"); }
    int initialize_runtime_067320(int) override { throw std::logic_error("unused initialize runtime"); }
    void uninitialize_runtime_06736c() override { throw std::logic_error("unused uninitialize runtime"); }
    std::uint32_t invoke_entry(std::uintptr_t,std::uintptr_t) override { throw std::logic_error("unused invoke"); }
    [[noreturn]] void free_library_and_exit_thread(t::Handle,std::uint32_t) override { throw std::logic_error("unused free and exit"); }
    [[noreturn]] void exit_thread(std::uint32_t) override { throw std::logic_error("unused thread exit"); }
};
struct Dependencies : n::ThreadBridge {
    explicit Dependencies(Threads& threads) : ThreadBridge(threads) {}
    unsigned cleanups{},deletions{},exits{},registrations{},terminations{},environment{},console{},configuration{},between{},guard{};
    std::vector<std::size_t> deleted_sizes;
    std::vector<b::Handle> closed;
    int register_exit(b::RegisteredCallback c) override { check(c==b::RegisteredCallback::exit_038950);check(n::Resolver{}.registered_address(c)==reinterpret_cast<std::uintptr_t>(n::Resolver{}.exit_callback())); ++registrations; return -1; }
    void install_terminate(b::RegisteredCallback c) override { check(c==b::RegisteredCallback::terminate_038890);check(n::Resolver{}.registered_address(c)==reinterpret_cast<std::uintptr_t>(n::Resolver{}.terminate_callback())); ++terminations; }
    void setup_environment_019910() override { ++environment; }
    void setup_console_038760() override { ++console; }
    void* log_insert_006880(void* stream,std::uintptr_t) override { return stream; }
    void log_finish_0088e0(void*) override {}
    void load_configuration_01c870() override { ++configuration; }
    void* allocate_04b0bc(std::size_t size) override { return ::operator new(size); }
    void delete_04b0f8(void* p,std::size_t size) override { ++deletions;deleted_sizes.push_back(size);::operator delete(p); }
    b::NoArgumentFunction worker(b::Worker w) override { return w==b::Worker::first_029ea0?&first:&second; }
    b::BooleanFunction boolean_worker_035850() override { return &boolean; }
    bool close_handle(b::Handle h) override { closed.push_back(h); return true; }
    [[noreturn]] void throw_thread_error_04a648(int) override { throw std::logic_error("synthetic thread failure"); }
    void between_workers_00e310() override { ++between; }
    void guard_return_017a30() override { ++guard; }
    void cleanup_threads_04a47c() override { ++cleanups; }
    void cleanup_035ab0() override { check(exits==0);++exits; }
    void cleanup_0366f0() override { check(exits==1);++exits; }
    void cleanup_0102b0() override { check(exits==2);++exits;SetLastError(71); }
    void terminate_log_038920() override { throw std::runtime_error("synthetic termination backend"); }
    [[noreturn]] void unexpected_terminate_return_trap() override { throw std::logic_error("unexpected terminal return"); }
};
}
int main(int argc,char** argv) {
    if(argc>1&&std::string(argv[1])=="--unconfigured") { std::set_terminate([] { std::_Exit(84); });n::exit_entry(); }
    Threads threads;
    Dependencies dependencies(threads);
    b::Bootstrap bootstrap(dependencies);
    Errors errors;
    n::Binding binding(bootstrap,errors);
    const std::string mode=argc>1?argv[1]:"";
    if(mode=="--closed") { errors.expected_entry=n::Entry::Exit;binding.close();n::exit_entry(); }
    if(mode=="--throw-worker") { b::NoArgumentPayload payload{&throws_worker};n::no_argument_entry(&payload); }
    if(mode=="--terminate") { errors.expected_entry=n::Entry::Terminate;n::terminate_entry(); }
    try {
        SetLastError(19);
        check(n::is_bound()&&n::active_calls()==0&&GetLastError()==19);
        try { n::Binding duplicate(bootstrap,errors);check(false); } catch(Fault f) { check(f.fault==n::LifecycleFault::AlreadyBound); }
        check(GetLastError()==19);
        n::Resolver resolver;
        check(resolver.thread_entry(b::ThreadEntry::no_argument_0388a0)==&n::no_argument_entry);
        check(resolver.thread_entry(b::ThreadEntry::boolean_0388d0)==&n::boolean_entry);
        check(resolver.application()==&n::application_entry);
        try { (void)resolver.thread_entry(static_cast<b::ThreadEntry>(-1));check(false); } catch(const std::out_of_range&) { check(true); }
        try { (void)resolver.registered_address(static_cast<b::RegisteredCallback>(-1));check(false); } catch(const std::out_of_range&) { check(true); }
        for(DWORD reason:{0ul,2ul,3ul,0xfffffffful}) { check(resolver.application()(nullptr,reason,nullptr)==TRUE);check(dependencies.registrations==0);check(GetLastError()==19); }
        check(resolver.application()(reinterpret_cast<HINSTANCE>(17),1,reinterpret_cast<void*>(18))==TRUE);
        check(dependencies.registrations==1&&dependencies.terminations==1);
        check(dependencies.environment==1&&dependencies.console==1&&dependencies.configuration==1);
        check(dependencies.between==1&&dependencies.guard==1);
        check(threads.contexts.size()==3&&threads.null_frees==3);
        check(dependencies.closed==std::vector<b::Handle>({101,102,103}));
        check(events==std::vector<int>({3}));
        for(std::size_t i=0;i<3;++i) {
            const auto* c=threads.contexts[i];
            const auto fn=reinterpret_cast<n::ThreadStart>(c->entry);
            check(fn==(i<2?&n::no_argument_entry:&n::boolean_entry));
            check(c->argument!=0&&c->module==0&&c->handle==0);
            SetLastError(80);
            check(fn(reinterpret_cast<void*>(c->argument))==0);
            check(GetLastError()==(i==0?41ul:i==1?42ul:44ul));
            check(n::active_calls()==0);
        }
        check(events==std::vector<int>({3,1,2,4}));
        check(dependencies.cleanups==3&&dependencies.deletions==3);
        check(dependencies.deleted_sizes==std::vector<std::size_t>({8,8,16}));
        resolver.exit_callback()();
        check(dependencies.exits==3&&GetLastError()==71);

        auto* payload=new b::NoArgumentPayload{&blocked_worker};
        std::thread worker([&] { check(n::no_argument_entry(payload)==0); });
        while(!entered.load()) std::this_thread::yield();
        check(n::active_calls()==1);
        try { binding.close();check(false); } catch(Fault f) { check(f.fault==n::LifecycleFault::Busy); }
        check(n::is_bound());
        release_worker.store(true);
        worker.join();
        check(n::active_calls()==0&&dependencies.cleanups==4&&dependencies.deletions==4);
        SetLastError(92);binding.close();check(!n::is_bound()&&GetLastError()==92);binding.close();
        Errors next_errors;
        { n::Binding rebound(bootstrap,next_errors);check(n::is_bound()); }
        check(!n::is_bound());
        std::cout<<"PASS "<<checks<<" checks: rebuilt Bootstrap entry/resolver/CRT bridge and owned synthetic worker; no target execution\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }
}
