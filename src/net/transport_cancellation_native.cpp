#include "preloader_l/net/transport_cancellation_native.hpp"

namespace preloader_l::socket_recovered {
static_assert(sizeof(void*)==8 && sizeof(OVERLAPPED)==32);
std::int32_t NativeCompletionApi::post(std::uintptr_t port,CancelOperation& operation){
    return imports_.post(reinterpret_cast<HANDLE>(port),0,0,&operation.overlapped);
}
} // namespace preloader_l::socket_recovered
