#pragma once
#include "../anadius-foreign-callback-source/foreign.hpp"

namespace anadius::host_platform {
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
}
