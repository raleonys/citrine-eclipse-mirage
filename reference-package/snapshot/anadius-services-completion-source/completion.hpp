#pragma once
#include "../anadius-bootstrap-owner-source/owner.hpp"
#include "exit_wait.hpp"
#include <cstddef>
#include <memory>
#include <mutex>
#include <ostream>
#include <string_view>

namespace anadius::services_completion {
// Selected compiler-ostream representation of the recovered bootstrap logger.
// This is a bounded identity projection, not an original MSVC basic_ios layout.
// The supplied shared owner must keep the stream AND its streambuf alive.
// The logger is immovable; returned tokens expire when this owner dies.
class BootstrapLogger final {
public:
    static constexpr std::uintptr_t stream_identity=0x1800a4130ULL;
    explicit BootstrapLogger(std::shared_ptr<std::ostream>);
    BootstrapLogger(const BootstrapLogger&)=delete;
    BootstrapLogger& operator=(const BootstrapLogger&)=delete;
    BootstrapLogger(BootstrapLogger&&)=delete;
    BootstrapLogger& operator=(BootstrapLogger&&)=delete;
    static std::string_view literal(std::uintptr_t);
    void* token() noexcept;
    void* insert(void* stream,std::uintptr_t literal_va);
    void finish(void* stream);
    [[noreturn]] void terminate_log();
private:
    void validate(void*) const;
    std::shared_ptr<std::ostream> stream_;
    // Serialize callers through this logger; same-thread streambuf callbacks
    // may reenter it. Other users of the supplied stream need external sync.
    std::recursive_mutex gate_;
    std::byte token_{};
};

// Selects the five policies remaining after B3/B4/B7/B9 composition. The other
// twelve methods forward to their mandatory owned lower implementation.
// B3/B4 stop/join must precede destruction; a shared owner is not a worker join.
class CompletionServices final:public bootstrap_owner::Services {
public:
    CompletionServices(std::shared_ptr<BootstrapLogger>,std::shared_ptr<ExitWaitRegistry>,
        std::unique_ptr<bootstrap_owner::Services>);
    int register_exit(bootstrap_native_recovered::ExitCallback) override;
    void install_terminate(std::terminate_handler) override;
    void* log_insert(void*,std::uintptr_t) override;
    void log_finish(void*) override;
    void* allocate(std::size_t) override;
    void deallocate(void*,std::size_t) override;
    recovered::NoArgumentFunction worker(recovered::Worker) override;
    recovered::BooleanFunction boolean_worker() override;
    [[noreturn]] void thread_error(int) override;
    void between_workers() override;
    void guard_return() override;
    void cleanup_threads() override;
    void cleanup_first() override;
    void cleanup_second() override;
    void cleanup_network() override;
    void terminate_log() override;
    [[noreturn]] void unexpected_terminate_return() override;
private:
    // Reverse destruction keeps both selected policy owners alive while the
    // lower provider is destroyed. The stream remains owned by its logger.
    std::shared_ptr<BootstrapLogger> logger_;
    std::shared_ptr<ExitWaitRegistry> exits_;
    std::unique_ptr<bootstrap_owner::Services> lower_;
};
} // namespace anadius::services_completion
