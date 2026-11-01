// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/bootstrap.hpp"
#include <new>

namespace preloader_l::system_services {
// Evidence: ev_0a232e746d99beddba99fe362a4f132625939b77bd747e39ad749975c6f32a06
bool Bootstrap::application_dll_main(std::uint32_t reason) {
    if (reason == 1) initialize(); // DLL_PROCESS_ATTACH; others have no app action
    return true;
}

// 18004a458; asm ev_218bf3eda319b5a7858921c077bd0fd00cadc3c2d550d8952be3eef945886923
int Bootstrap::detach(const ThreadHandle& thread) {
    return deps_.close_handle(thread.handle) ? 0 : 4;
}

void Bootstrap::start_detached(ThreadEntry entry, void* payload) {
    ThreadHandle thread{};
    thread.handle = deps_.begin_thread_05e8e0(entry, payload, &thread.id);
    if (thread.handle == 0) deps_.throw_thread_error_04a648(6);
    if (thread.id == 0 || detach(thread) != 0) deps_.throw_thread_error_04a648(1);
    // Original clears its local {handle,id}; it does not join or wait for worker.
}

// 180038970; asm ev_77887ea2e7b744e62e52eaf81cb8a07abf2a4f50498001ba81b2fcf2496e8789
void Bootstrap::initialize() {
    (void)deps_.register_exit(RegisteredCallback::exit_038950); // return ignored
    deps_.install_terminate(RegisteredCallback::terminate_038890);
    deps_.setup_environment_019910();
    deps_.setup_console_038760();
    auto* stream = deps_.log_insert_006880(reinterpret_cast<void*>(0x1800a4130), 0x180083cd8);
    stream = deps_.log_insert_006880(stream, 0x180086720);
    stream = deps_.log_insert_006880(stream, 0x180086718);
    deps_.log_finish_0088e0(stream);
    deps_.load_configuration_01c870();

    auto* first = static_cast<NoArgumentPayload*>(deps_.allocate_04b0bc(8));
    if (first) ::new (first) NoArgumentPayload{deps_.worker(Worker::first_029ea0)};
    start_detached(ThreadEntry::no_argument_0388a0, first);
    deps_.between_workers_00e310();
    auto* second = static_cast<NoArgumentPayload*>(deps_.allocate_04b0bc(8));
    if (second) ::new (second) NoArgumentPayload{deps_.worker(Worker::second_035b60)};
    start_detached(ThreadEntry::no_argument_0388a0, second);
    deps_.boolean_worker_035850()(true); // synchronous call precedes third allocation
    auto* third = static_cast<BooleanPayload*>(deps_.allocate_04b0bc(16));
    // Keep padding untouched as in original (only bytes 0 and 8..15 are assigned).
    if (third) { ::new (third) BooleanPayload; third->value = false;
                 third->function = deps_.boolean_worker_035850(); }
    start_detached(ThreadEntry::boolean_0388d0, third);
    deps_.guard_return_017a30();
}

// 1800388a0; asm ev_3022378b3b333555b2f1f63fa59a85ea675e756369871f825e3fd805143160eb
unsigned Bootstrap::run_no_argument(NoArgumentPayload* payload) {
    payload->function();
    deps_.cleanup_threads_04a47c();
    deps_.delete_04b0f8(payload, 8);
    return 0;
}

// 1800388d0; asm ev_220d092f92df6b8c1c7cbb91ffd4ee02e5b5af3f3f6fd15d599e710c8adf353e
unsigned Bootstrap::run_boolean(BooleanPayload* payload) {
    payload->function(payload->value);
    deps_.cleanup_threads_04a47c();
    if (payload) deps_.delete_04b0f8(payload, 16);
    return 0;
}

// 180038950; asm ev_9e5bed8eb620590b20a44651d47fd66e0d4d00f7cc3caf952f79bc4a73f22049
void Bootstrap::on_exit() {
    deps_.cleanup_035ab0();
    deps_.cleanup_0366f0();
    deps_.cleanup_0102b0();
}

// 180038890; asm ev_a73f817561c1dc663846bb56938e55cea83e98470e9af8c91186939f6bbbe144
[[noreturn]] void Bootstrap::on_terminate() {
    deps_.terminate_log_038920();
    deps_.unexpected_terminate_return_trap();
}

// 18004a47c; asm ev_b17cc6887bbf8d03f11ab86e0b2d89608cc3671d3da2122e64441f45bb292dc9
void cleanup_exit_waits(ExitWaitBlock& head, ExitWaitDependencies& deps) {
    const auto thread_id = deps.current_thread_id(); // before critical-section entry
    deps.lock_047df8();
    auto* block = &head;
    do {
        for (std::size_t i = 0; i < 20 && block->active_count != 0; ++i) {
            auto& entry = block->entries[i];
            if (entry.mutex && entry.thread_id == thread_id) {
                if (entry.ready) *entry.ready = 1;
                deps.unlock_mutex_04a6b4(entry.mutex);
                deps.notify_all_04ae28(entry.condition);
                entry.mutex = nullptr;
                --block->active_count;
            }
        }
        block = block->next;
    } while (block);
    deps.unlock_047e04();
}
} // namespace preloader_l::system_services
