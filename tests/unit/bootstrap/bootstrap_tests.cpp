// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/bootstrap.hpp"
#include <array>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

using namespace preloader_l::system_services;
namespace {
int checks = 0;
void require(bool ok, const char* message) {
    ++checks;
    if (!ok) throw std::runtime_error(message);
}
struct ThreadError { int index; };
struct CallbackError {};
struct Trap {};
struct Fake;
Fake* active;
void first_worker();
void second_worker();
void bool_worker(bool);

struct Fake final : BootstrapDependencies {
    struct Launch { ThreadEntry entry; void* payload; };
    std::vector<std::string> events;
    std::vector<void*> allocated;
    std::vector<Launch> launches;
    std::vector<std::uintptr_t> literals;
    int fail_begin = 0, zero_id = 0, fail_close = 0, null_allocate = 0;
    int allocation_calls = 0, close_calls = 0;
    bool worker_throws = false, allocation_throws = false;
    Fake() { active = this; }
    ~Fake() override { for (auto* p : allocated) ::operator delete(p); }
    int register_exit(RegisteredCallback cb) override {
        require(cb == RegisteredCallback::exit_038950, "registered exit callback");
        events.emplace_back("register_exit"); return -1; // original ignores failure
    }
    void install_terminate(RegisteredCallback cb) override {
        require(cb == RegisteredCallback::terminate_038890, "terminate callback");
        events.emplace_back("install_terminate");
    }
    void setup_environment_019910() override { events.emplace_back("environment"); }
    void setup_console_038760() override { events.emplace_back("console"); }
    void* log_insert_006880(void* p, std::uintptr_t va) override {
        require(p == reinterpret_cast<void*>(0x1800a4130 + literals.size()*16), "stream chain input");
        literals.push_back(va); events.emplace_back("log_insert");
        return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+16);
    }
    void log_finish_0088e0(void* p) override {
        require(p == reinterpret_cast<void*>(0x1800a4160), "log final stream");
        events.emplace_back("log_finish");
    }
    void load_configuration_01c870() override { events.emplace_back("configuration"); }
    void* allocate_04b0bc(std::size_t n) override {
        events.emplace_back("allocate" + std::to_string(n)); ++allocation_calls;
        if (allocation_throws) throw std::bad_alloc();
        if (allocation_calls == null_allocate) return nullptr;
        auto* p = ::operator new(n); allocated.push_back(p); return p;
    }
    void delete_04b0f8(void* p, std::size_t n) override {
        events.emplace_back("delete" + std::to_string(n));
        auto it = std::find(allocated.begin(), allocated.end(), p);
        require(it != allocated.end(), "delete owns payload");
        ::operator delete(p); allocated.erase(it);
    }
    NoArgumentFunction worker(Worker id) override {
        return id == Worker::first_029ea0 ? first_worker : second_worker;
    }
    BooleanFunction boolean_worker_035850() override { return bool_worker; }
    Handle begin_thread_05e8e0(ThreadEntry e, void* p, std::uint32_t* id) override {
        launches.push_back({e,p}); int ordinal=static_cast<int>(launches.size());
        events.emplace_back("begin" + std::to_string(ordinal));
        if (ordinal == fail_begin) return 0;
        *id = ordinal == zero_id ? 0 : 100U+static_cast<unsigned>(ordinal);
        return 1000U+static_cast<unsigned>(ordinal);
    }
    bool close_handle(Handle h) override {
        ++close_calls;
        require(h == 1000U+static_cast<unsigned>(close_calls), "closed launched handle");
        events.emplace_back("close" + std::to_string(close_calls));
        return close_calls != fail_close;
    }
    [[noreturn]] void throw_thread_error_04a648(int index) override {
        events.emplace_back("error" + std::to_string(index)); throw ThreadError{index};
    }
    void between_workers_00e310() override { events.emplace_back("between_workers"); }
    void guard_return_017a30() override { events.emplace_back("guard_return"); }
    void cleanup_threads_04a47c() override { events.emplace_back("thread_cleanup"); }
    void cleanup_035ab0() override { events.emplace_back("cleanup_a"); }
    void cleanup_0366f0() override { events.emplace_back("cleanup_b"); }
    void cleanup_0102b0() override { events.emplace_back("cleanup_c"); }
    void terminate_log_038920() override { events.emplace_back("terminate_log"); }
    [[noreturn]] void unexpected_terminate_return_trap() override { throw Trap{}; }
};
void first_worker() {
    active->events.emplace_back("worker_first");
    if(active->worker_throws) throw CallbackError{};
}
void second_worker() { active->events.emplace_back("worker_second"); }
void bool_worker(bool value) {
    active->events.emplace_back(value ? "worker_true" : "worker_false");
    if(active->worker_throws) throw CallbackError{};
}
void test_success() {
    Fake f; Bootstrap b(f);
    require(b.application_dll_main(1), "attach returns true");
    const std::vector<std::string> expected{
        "register_exit","install_terminate","environment","console",
        "log_insert","log_insert","log_insert","log_finish","configuration",
        "allocate8","begin1","close1","between_workers","allocate8","begin2","close2",
        "worker_true","allocate16","begin3","close3","guard_return"};
    require(f.events == expected, "entire startup call ordering");
    require(f.literals == std::vector<std::uintptr_t>{0x180083cd8,0x180086720,0x180086718}, "literal addresses");
    require(f.launches.size() == 3, "three detached launches");
    require(f.launches[0].entry == ThreadEntry::no_argument_0388a0 &&
            f.launches[1].entry == ThreadEntry::no_argument_0388a0 &&
            f.launches[2].entry == ThreadEntry::boolean_0388d0, "trampoline selection");
    auto* first = static_cast<NoArgumentPayload*>(f.launches[0].payload);
    auto* second = static_cast<NoArgumentPayload*>(f.launches[1].payload);
    auto* third = static_cast<BooleanPayload*>(f.launches[2].payload);
    require(first->function == first_worker && second->function == second_worker,
            "worker functions retained in payloads");
    require(!third->value && third->function == bool_worker, "boolean payload false");
    f.events.clear();
    require(b.run_no_argument(first) == 0 && b.run_no_argument(second) == 0 &&
            b.run_boolean(third) == 0, "trampoline return values");
    require(f.events == std::vector<std::string>{"worker_first","thread_cleanup","delete8",
            "worker_second","thread_cleanup","delete8","worker_false","thread_cleanup","delete16"},
            "callback cleanup and delete order");
    require(f.allocated.empty(), "successful callback payloads deleted");
}
void test_error_paths() {
    for (int launch=1; launch<=3; ++launch) {
        for(int kind=0;kind<3;++kind) {
            Fake f; Bootstrap b(f);
            if(kind==0) f.fail_begin=launch;
            if(kind==1) f.zero_id=launch;
            if(kind==2) f.fail_close=launch;
            try { b.initialize(); require(false,"expected thread error"); }
            catch (ThreadError e) { require(e.index==(kind==0?6:1),"exact thread error index"); }
            require(f.launches.size()==static_cast<std::size_t>(launch),"failure blocks later launch");
            require(f.close_calls==(kind==2?launch:launch-1),"zero ID does not close handle");
            require(f.allocated.size()==static_cast<std::size_t>(launch),
                    "original caller has no extra payload deletion on launch error");
            require(std::find(f.events.begin(),f.events.end(),"guard_return")==f.events.end(),
                    "error blocks final tail call");
        }
    }
    for(int launch=1; launch<=3; ++launch) {
        Fake f; Bootstrap b(f); f.null_allocate=launch;
        b.initialize();
        require(f.launches[launch-1].payload==nullptr,"original null allocation branch forwarded");
        // Do not invoke null payload; original trampoline dereferences before null check.
    }
    { Fake f; Bootstrap b(f); f.allocation_throws=true;
      try { b.initialize(); require(false,"expected allocator exception"); }
      catch(const std::bad_alloc&) {}
      require(f.launches.empty(),"throwing allocation blocks launch"); }
}
void test_callbacks_and_notifications() {
    for (auto reason : {0U,2U,3U,999U}) {
        Fake f; Bootstrap b(f);
        require(b.application_dll_main(reason),"non-attach returns true");
        require(f.events.empty(),"non-attach no app initialization");
    }
    { Fake f; Bootstrap b(f); b.on_exit();
      require(f.events==std::vector<std::string>{"cleanup_a","cleanup_b","cleanup_c"},"atexit ordering"); }
    { Fake f; Bootstrap b(f);
      try { b.on_terminate(); require(false,"expected terminate trap"); } catch(Trap) {}
      require(f.events==std::vector<std::string>{"terminate_log"},"terminate log before trap"); }
    for(bool boolean : {false,true}) {
        Fake f; Bootstrap b(f); f.worker_throws=true;
        if(boolean) {
            auto* p = ::new(f.allocate_04b0bc(16)) BooleanPayload{false,bool_worker};
            f.events.clear();
            try { b.run_boolean(p); require(false,"expected callback throw"); } catch(CallbackError) {}
        } else {
            auto* p = ::new(f.allocate_04b0bc(8)) NoArgumentPayload{first_worker};
            f.events.clear();
            try { b.run_no_argument(p); require(false,"expected callback throw"); } catch(CallbackError) {}
        }
        require(f.events.size()==1 && f.allocated.size()==1,"no invented cleanup on callback unwind");
    }
}
struct ExitFake final : ExitWaitDependencies {
    std::vector<std::string> events;
    std::int32_t* ready{};
    std::uint32_t current_thread_id() override {events.emplace_back("id");return 7;}
    void lock_047df8() override {events.emplace_back("lock");}
    void unlock_047e04() override {events.emplace_back("unlock");}
    void unlock_mutex_04a6b4(void*) override {
        if(ready) require(*ready==1,"ready set before unlock");events.emplace_back("mutex");
    }
    void notify_all_04ae28(void*) override {events.emplace_back("notify");}
};
void test_exit_registry() {
    ExitWaitBlock head{}, next{}; head.next=&next;
    std::int32_t ready=0; ExitFake f; f.ready=&ready;
    head.active_count=3;
    head.entries[0]={0,7,0,reinterpret_cast<void*>(1),reinterpret_cast<void*>(2),&ready};
    head.entries[1]={0,8,0,reinterpret_cast<void*>(3),reinterpret_cast<void*>(4),nullptr};
    head.entries[19]={0,7,0,reinterpret_cast<void*>(5),reinterpret_cast<void*>(6),nullptr};
    next.active_count=1;
    next.entries[0]={0,7,0,reinterpret_cast<void*>(7),reinterpret_cast<void*>(8),nullptr};
    cleanup_exit_waits(head,f);
    require(f.events==std::vector<std::string>{"id","lock","mutex","notify","mutex","notify",
            "mutex","notify","unlock"},"20-entry and next-block registry scan");
    require(head.active_count==1 && next.active_count==0,"matching active counters decreased");
    require(head.entries[0].mutex==nullptr && head.entries[19].mutex==nullptr &&
            next.entries[0].mutex==nullptr,"matching mutex entries cleared");
    require(head.entries[1].mutex==reinterpret_cast<void*>(3),"other thread unchanged");
    require(head.entries[0].condition==reinterpret_cast<void*>(2) && head.entries[0].ready==&ready,
            "only mutex field is cleared");
    ExitWaitBlock empty{}; ExitFake ef;
    cleanup_exit_waits(empty,ef);
    require(ef.events==std::vector<std::string>{"id","lock","unlock"},"empty registry still locks");
}
} // namespace
int main() {
    try {
        test_success(); test_error_paths(); test_callbacks_and_notifications(); test_exit_registry();
        std::cout << "PASS: " << checks << " mock contract assertions; original DLL was not loaded or executed.\n";
        return 0;
    } catch(const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
