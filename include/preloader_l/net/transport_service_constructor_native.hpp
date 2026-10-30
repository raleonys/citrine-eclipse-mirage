#pragma once
#include "preloader_l/net/transport_service_constructor.hpp"

namespace preloader_l::socket_recovered {
struct ServiceGateImports {
    decltype(&::InitializeCriticalSectionAndSpinCount) initialize=&::InitializeCriticalSectionAndSpinCount;
    decltype(&::GetLastError) get_last_error=&::GetLastError;
};
// Merely constructing this adapter calls no Win32 API. The SDK section is
// borrowed; native deletion/ownership and entry rollback remain caller effects.
class NativeServiceGateApi final:public ServiceGateApi {
public:
    explicit NativeServiceGateApi(ServiceGateImports imports={}):imports_(imports){}
    std::int32_t initialize(CRITICAL_SECTION&,std::uint32_t spin_count) override;
    std::uint32_t get_last_error() override;
private:ServiceGateImports imports_;
};
} // namespace preloader_l::socket_recovered
