#pragma once
#include <cstddef>
#include <cstdint>

// Reconstructed from SHA-256 9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.
// These dependency contracts are mandatory; this is not a standalone replacement DLL.
namespace anadius::recovered {
using Handle = std::uintptr_t;
using NoArgumentFunction = void (*)();
using BooleanFunction = void (*)(bool);
enum class Worker { first_029ea0, second_035b60 };
enum class ThreadEntry { no_argument_0388a0, boolean_0388d0 };
enum class RegisteredCallback { exit_038950, terminate_038890 };

struct NoArgumentPayload { NoArgumentFunction function; };
struct BooleanPayload { bool value; BooleanFunction function; };
static_assert(sizeof(void*) == 8, "This reconstruction targets x64");
static_assert(sizeof(NoArgumentPayload) == 8);
static_assert(sizeof(BooleanPayload) == 16);
static_assert(offsetof(BooleanPayload, function) == 8);
struct ThreadHandle { Handle handle; std::uint32_t id; };

// Every method below is an unresolved external integration boundary, not a stub.
// The tests provide mocks. No implementation here invokes the original binary.
class BootstrapDependencies {
public:
    virtual ~BootstrapDependencies() = default;
    virtual int register_exit(RegisteredCallback) = 0;              // atexit 04b770
    virtual void install_terminate(RegisteredCallback) = 0;       // set_terminate 05e6c8
    virtual void setup_environment_019910() = 0;
    virtual void setup_console_038760() = 0;
    virtual void* log_insert_006880(void* stream, std::uintptr_t literal_va) = 0;
    virtual void log_finish_0088e0(void* stream) = 0;
    virtual void load_configuration_01c870() = 0;
    virtual void* allocate_04b0bc(std::size_t) = 0; // throwing operator new; see notes
    virtual void delete_04b0f8(void*, std::size_t) = 0; // sized delete, ignores size in binary
    virtual NoArgumentFunction worker(Worker) = 0;
    virtual BooleanFunction boolean_worker_035850() = 0;
    // Must implement the CRT wrapper at 05e8e0, not raw CreateThread. On success
    // ownership of payload passes to its trampoline. On failure original caller
    // does not explicitly delete it. Output ID is meaningful only on success.
    virtual Handle begin_thread_05e8e0(ThreadEntry, void* payload,
                                      std::uint32_t* id) = 0;
    virtual bool close_handle(Handle) = 0;
    // 04a648 throws a Microsoft CRT C++ system_error. Numeric values are table
    // indices, not Win32 GetLastError values. Implementations must not return.
    [[noreturn]] virtual void throw_thread_error_04a648(int index) = 0;
    virtual void between_workers_00e310() = 0;
    virtual void guard_return_017a30() = 0; // observed final tail jump (_guard_check_icall)
    virtual void cleanup_threads_04a47c() = 0;
    virtual void cleanup_035ab0() = 0;
    virtual void cleanup_0366f0() = 0;
    virtual void cleanup_0102b0() = 0;
    virtual void terminate_log_038920() = 0; // log then abort; exact exception ABI unresolved
    [[noreturn]] virtual void unexpected_terminate_return_trap() = 0; // INT3 at 03889b
};

class Bootstrap {
public:
    explicit Bootstrap(BootstrapDependencies& dependencies) : deps_(dependencies) {}
    bool application_dll_main(std::uint32_t reason);               // 180038870
    void initialize();                                           // 180038970
    void on_exit();                                              // 180038950
    [[noreturn]] void on_terminate();                             // 180038890
    unsigned run_no_argument(NoArgumentPayload*);                 // 1800388a0
    unsigned run_boolean(BooleanPayload*);                       // 1800388d0
    int detach(const ThreadHandle&);                             // 18004a458
private:
    void start_detached(ThreadEntry, void*);
    BootstrapDependencies& deps_;
};

// 04a47c's global list (head VA 1800a42b0). Only accessed fields are recovered.
struct ExitWaitEntry {
    std::uint64_t unobserved_00;
    std::uint32_t thread_id;
    std::uint32_t unobserved_0c;
    void* mutex;
    void* condition;
    std::int32_t* ready;
};
struct ExitWaitBlock {
    ExitWaitEntry entries[20];
    std::int32_t active_count;
    std::uint32_t unobserved_324;
    ExitWaitBlock* next;
};
static_assert(sizeof(ExitWaitEntry) == 0x28);
static_assert(offsetof(ExitWaitEntry, mutex) == 0x10);
static_assert(offsetof(ExitWaitBlock, active_count) == 0x320);
static_assert(offsetof(ExitWaitBlock, next) == 0x328);

class ExitWaitDependencies {
public:
    virtual ~ExitWaitDependencies() = default;
    virtual std::uint32_t current_thread_id() = 0;
    virtual void lock_047df8() = 0;   // critical section VA 1800a4010
    virtual void unlock_047e04() = 0;
    virtual void unlock_mutex_04a6b4(void*) = 0;
    virtual void notify_all_04ae28(void*) = 0;
};
void cleanup_exit_waits(ExitWaitBlock& head, ExitWaitDependencies&);
} // namespace anadius::recovered
