// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_cancellation.hpp"

namespace citrine_eclipse_mirage::socket {
struct CompletionImports {
    decltype(&::PostQueuedCompletionStatus) post=&::PostQueuedCompletionStatus;
};
// Typed SDK call only. Caller owns the port, operation/OVERLAPPED lifetime and
// completion consumer. This does not install an original ASIO IOCP dispatcher.
class NativeCompletionApi final:public CompletionApi {
public:
    explicit NativeCompletionApi(CompletionImports imports={}):imports_(imports){}
    std::int32_t post(std::uintptr_t port,CancelOperation&) override;
private:CompletionImports imports_;
};
} // namespace citrine_eclipse_mirage::socket
