#pragma once
#include "iocp_constructor.hpp"

namespace anadius::socket_recovered {
struct IocpConstructorImports {
    decltype(&::VerSetConditionMask) set_version_mask=&::VerSetConditionMask;
    decltype(&::VerifyVersionInfoA) verify_version=&::VerifyVersionInfoA;
    decltype(&::CreateIoCompletionPort) create_completion_port=&::CreateIoCompletionPort;
    decltype(&::GetLastError) get_last_error=&::GetLastError;
    decltype(&::CloseHandle) close_handle=&::CloseHandle;
};
// Default SDK imports are available; construction itself invokes none. Tests
// inject every call and never create an IOCP, worker, event or native scheduler.
class NativeIocpConstructorApi final:public IocpConstructorApi {
public:
    explicit NativeIocpConstructorApi(IocpConstructorImports imports={}):imports_(imports){}
    std::uint64_t set_version_mask(std::uint64_t,std::uint32_t,std::uint8_t) override;
    std::int32_t verify_version(OSVERSIONINFOEXA&,std::uint32_t,std::uint64_t) override;
    std::uintptr_t create_completion_port(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uint32_t) override;
    std::uint32_t get_last_error() override;
    std::int32_t close_handle(std::uintptr_t) override;
private:IocpConstructorImports imports_;
};
} // namespace anadius::socket_recovered
