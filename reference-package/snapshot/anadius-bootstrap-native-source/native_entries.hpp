#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <exception>
#include "../anadius-thread-source/threads.hpp"

namespace anadius::bootstrap_native_recovered {
enum class Entry { Application, NoArgument, Boolean, Exit, Terminate };
enum class LifecycleFault { AlreadyBound, Busy, WrongOwner };
class Errors {
public:
    virtual ~Errors() = default;
    [[noreturn]] virtual void lifecycle(LifecycleFault) = 0;
    [[noreturn]] virtual void callback_exception(Entry,std::exception_ptr) noexcept = 0;
};
// The binding does not register callbacks, start threads, or install a DLL entry.
// Bootstrap and dependencies must remain stable until close succeeds. Disable
// external sources of entries and obtain quiescence before closing/rebinding.
// Errors outlives every entry attempt, including calls after close. One control
// thread owns each Binding; close rejects active callbacks without waiting.
// active_calls()==0 does not cover suspended or not-yet-admitted workers.
// Caller must stop or join those future entry sources before close/rebinding.
class Binding final {
public:
    Binding(recovered::Bootstrap&,Errors&);
    ~Binding() noexcept;
    Binding(const Binding&)=delete;
    Binding& operator=(const Binding&)=delete;
    // Sticky HOST stop policy for this binding. Only NEW Application entries
    // are refused (FALSE, no provider/error callback); existing Application
    // leases and worker/exit/terminate routes remain live. False means an
    // admitted Application call still owns its lease: retry, never wait/join
    // its workers yet. This is not external/foreign or all-callback quiescence.
    bool pause_application_admission();
    void close();
private:
    recovered::Bootstrap* bootstrap_;
    Errors& errors_;
};
bool is_bound();
std::size_t active_calls();

using ThreadStart = unsigned (__stdcall *)(void*);
using ExitCallback = void (*)();
using ApplicationEntry = BOOL (WINAPI *)(HINSTANCE,DWORD,LPVOID);
unsigned __stdcall no_argument_entry(void*) noexcept;
unsigned __stdcall boolean_entry(void*) noexcept;
void exit_entry() noexcept;
[[noreturn]] void terminate_entry() noexcept;
BOOL WINAPI application_entry(HINSTANCE,DWORD,LPVOID) noexcept;

class Resolver final {
public:
    ThreadStart thread_entry(recovered::ThreadEntry) const;
    std::uintptr_t registered_address(recovered::RegisteredCallback) const;
    ExitCallback exit_callback() const { return &exit_entry; }
    std::terminate_handler terminate_callback() const { return &terminate_entry; }
    ApplicationEntry application() const { return &application_entry; }
};

// Concrete enum-to-code binding for the existing recovered CRT begin wrapper.
// All remaining BootstrapDependencies are deliberately still pure virtual.
class ThreadBridge : public threads_recovered::BootstrapThreadBridge {
public:
    explicit ThreadBridge(threads_recovered::Effects& e)
        : BootstrapThreadBridge(e) {}
protected:
    std::uintptr_t bind_application_entry(recovered::ThreadEntry) final;
};
}
