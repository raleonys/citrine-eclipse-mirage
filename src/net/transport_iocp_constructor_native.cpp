// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_iocp_constructor_native.hpp"

namespace citrine_eclipse_mirage::socket {
std::uint64_t NativeIocpConstructorApi::set_version_mask(std::uint64_t mask,std::uint32_t type,std::uint8_t condition){return imports_.set_version_mask(mask,type,condition);}
std::int32_t NativeIocpConstructorApi::verify_version(OSVERSIONINFOEXA& version,std::uint32_t type,std::uint64_t mask){return imports_.verify_version(&version,type,mask);}
std::uintptr_t NativeIocpConstructorApi::create_completion_port(std::uintptr_t file,std::uintptr_t existing,std::uintptr_t key,std::uint32_t threads){return reinterpret_cast<std::uintptr_t>(imports_.create_completion_port(reinterpret_cast<HANDLE>(file),reinterpret_cast<HANDLE>(existing),key,threads));}
std::uint32_t NativeIocpConstructorApi::get_last_error(){return imports_.get_last_error();}
std::int32_t NativeIocpConstructorApi::close_handle(std::uintptr_t handle){return imports_.close_handle(reinterpret_cast<HANDLE>(handle));}
} // namespace citrine_eclipse_mirage::socket
