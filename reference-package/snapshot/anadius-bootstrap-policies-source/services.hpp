#pragma once
#include "../anadius-bootstrap-owner-source/owner.hpp"
#include "../anadius-thread-domain-source/domain.hpp"

namespace anadius::bootstrap_policies {
// Four selected routes in the existing Services interface. B9 binds thread
// errors to retained code/context data and a rebuilt generic-category exception.
// The lower provider remains mandatory for unresolved logging/cleanup policies.
// ApplicationMemory owns a lease of the actual DomainStorage heap/PTD graph;
// no compiler-new/application-heap mismatch is hidden behind a lifetime token.
// B3/B4 must stop admission and join every worker before provider destruction.
class SourceServices final:public bootstrap_owner::Services {
public:
    SourceServices(std::shared_ptr<thread_domain::ApplicationMemory>,std::unique_ptr<bootstrap_owner::Services>);
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
    // Reverse destruction keeps the heap/domain lease alive through lower
    // provider destruction. Outstanding allocations are a terminal invariant.
    std::shared_ptr<thread_domain::ApplicationMemory> memory_;
    std::unique_ptr<bootstrap_owner::Services> lower_;
};
} // namespace anadius::bootstrap_policies
