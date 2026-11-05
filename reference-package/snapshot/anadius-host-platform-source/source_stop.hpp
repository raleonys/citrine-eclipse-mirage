#pragma once
#include "../anadius-foreign-callback-source/foreign.hpp"
#include <atomic>

namespace anadius::host_platform {
struct SourceStopState;
// A bounded receipt from this exact callback-first source stopper. Completion
// means the supplied stopper returned true and admitted callback bodies drained.
// It does not cover native entry epilogues, unregistered sources, raw jobs,
// kernel IO finality or executable-image lifetime. New admission stays sealed.
class SourceStopReceipt {
    friend struct SourceStopState;
    // Retired last, after any state/source capture retaining this receipt.
    std::shared_ptr<foreign_callback::Domain> callbacks_;
    std::atomic<bool> complete_{false};
    explicit SourceStopReceipt(std::shared_ptr<foreign_callback::Domain>);
public:
    bool completed() const noexcept;
    const foreign_callback::Domain* callbacks_identity() const noexcept;
    const thread_domain::DomainStorage* memory_domain() const noexcept;
};
struct BoundSourceStop {
    std::function<bool()> stop;
    std::shared_ptr<const SourceStopReceipt> prerequisite;
};
// Selected HOST shutdown composition, not a recovered native function.
// Permanently seals new callbacks BEFORE invoking the existing source stopper.
// The stopper may signal/cancel existing calls, but must not retire providers;
// pending workers/callbacks must be able to finish without new hooked calls.
// A true stopper result is memoized; active callbacks still keep stop false.
// No waits or provider calls under a metadata lock. One controller, no reentry.
// Unknown stopper outcomes poison and retain this owner: later calls reject,
// destruction while poisoned is terminal rather than guessed cleanup/retry.
// Construction is passive and consumes source's callable donor while its new
// shared owner is alive. Copies of the result share one stop state.
// Construct on the Domain controller. Source's borrowed provider/code backing
// must outlive all copies; this gate does not unload or retain arbitrary code.
std::function<bool()> bind_source_stop(std::shared_ptr<foreign_callback::Domain>,std::function<bool()>& source);
// Same passive construction, with an unforgeable completion receipt for the
// selected composition. Allocations finish before source is consumed by swap.
BoundSourceStop bind_source_stop_with_receipt(std::shared_ptr<foreign_callback::Domain>,std::function<bool()>& source);
}
