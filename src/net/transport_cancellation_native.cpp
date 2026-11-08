// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_cancellation_native.hpp"

namespace citrine_eclipse_mirage::socket {
static_assert(sizeof(void*)==8 && sizeof(OVERLAPPED)==32);
std::int32_t NativeCompletionApi::post(std::uintptr_t port,CancelOperation& operation){
    return imports_.post(reinterpret_cast<HANDLE>(port),0,0,&operation.overlapped);
}
} // namespace citrine_eclipse_mirage::socket
