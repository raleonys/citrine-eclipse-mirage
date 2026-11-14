// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <array>
#include <cstddef>
#include <cstring>
#include <new>
#include <ostream>
#include <vector>

namespace laureondo::tools::listener_diagnostics {
// A bounded snapshot of candidate listener PIDs, not socket-identity proof.
inline constexpr ULONG max_table_bytes=8*1024*1024;
inline constexpr unsigned query_attempts=3;
struct Family {
    DWORD status=ERROR_NOT_READY;
    std::array<DWORD,32> pids{};
    std::size_t count=0;
    bool truncated=false;
};
struct Snapshot { Family ipv4,ipv6; DWORD release_error=NO_ERROR; };
inline std::vector<unsigned char> allocate_table(std::size_t bytes) {
    return std::vector<unsigned char>(bytes);
}
struct Imports {
    decltype(&::LoadLibraryExW) load=&::LoadLibraryExW;
    decltype(&::GetProcAddress) resolve=&::GetProcAddress;
    decltype(&::FreeLibrary) release=&::FreeLibrary;
    decltype(&::GetLastError) last_error=&::GetLastError;
    decltype(&allocate_table) allocate=&allocate_table;
};
using Query=decltype(&::GetExtendedTcpTable);
inline DWORD error_or(const Imports& api,DWORD fallback) {
    const auto error=api.last_error();
    return error?error:fallback;
}
struct ModuleLease {
    HMODULE module;
    const Imports& api;
    DWORD& release_error;
    ~ModuleLease() noexcept {
        try {
            if(!api.release(module))release_error=error_or(api,ERROR_GEN_FAILURE);
        }catch(...) { release_error=ERROR_UNHANDLED_EXCEPTION; }
    }
};
template<class Table,class Row>
Family capture_family(Query query,ULONG family,unsigned short port,const Imports& api) noexcept {
    Family result;
    try {
        ULONG bytes=0;
        auto status=query(nullptr,&bytes,FALSE,family,TCP_TABLE_OWNER_PID_LISTENER,0);
        if(status!=ERROR_INSUFFICIENT_BUFFER) {
            result.status=status==NO_ERROR?ERROR_INVALID_DATA:status;
            return result;
        }
        constexpr auto header=offsetof(Table,table);
        for(unsigned attempt=0;attempt<query_attempts;++attempt) {
            if(bytes<header||bytes>max_table_bytes) {
                result.status=ERROR_INVALID_DATA;
                return result;
            }
            auto storage=api.allocate(bytes);
            if(storage.size()!=bytes) { result.status=ERROR_INVALID_DATA; return result; }
            status=query(storage.data(),&bytes,FALSE,family,TCP_TABLE_OWNER_PID_LISTENER,0);
            if(status==ERROR_INSUFFICIENT_BUFFER)continue;
            if(status!=NO_ERROR) { result.status=status; return result; }
            if(bytes<header||bytes>storage.size()) { result.status=ERROR_INVALID_DATA; return result; }
            DWORD rows=0;
            std::memcpy(&rows,storage.data(),sizeof rows);
            if(rows>(bytes-header)/sizeof(Row)) { result.status=ERROR_INVALID_DATA; return result; }
            for(DWORD index=0;index<rows;++index) {
                Row row{};
                std::memcpy(&row,storage.data()+header+index*sizeof(Row),sizeof row);
                const auto network_port=static_cast<unsigned short>(row.dwLocalPort);
                const auto host_port=static_cast<unsigned short>((network_port>>8)|(network_port<<8));
                if(row.dwState!=MIB_TCP_STATE_LISTEN||host_port!=port)continue;
                bool duplicate=false;
                for(std::size_t i=0;i<result.count;++i)duplicate|=result.pids[i]==row.dwOwningPid;
                if(duplicate)continue;
                if(result.count==result.pids.size())result.truncated=true;
                else result.pids[result.count++]=row.dwOwningPid;
            }
            result.status=NO_ERROR;
            return result;
        }
        result.status=ERROR_INSUFFICIENT_BUFFER;
    }catch(const std::bad_alloc&) { result.status=ERROR_NOT_ENOUGH_MEMORY; }
    catch(...) { result.status=ERROR_UNHANDLED_EXCEPTION; }
    return result;
}
inline Snapshot capture(unsigned short port,const Imports& api=Imports{}) noexcept {
    Snapshot result;
    try {
        const auto module=api.load(L"iphlpapi.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(!module) {
            result.ipv4.status=result.ipv6.status=error_or(api,ERROR_MOD_NOT_FOUND);
        }else {
            // Leave this scope before returning so release failures reach the snapshot.
            ModuleLease lease{module,api,result.release_error};
            const auto query=reinterpret_cast<Query>(api.resolve(module,"GetExtendedTcpTable"));
            if(!query)result.ipv4.status=result.ipv6.status=error_or(api,ERROR_PROC_NOT_FOUND);
            else {
                result.ipv4=capture_family<MIB_TCPTABLE_OWNER_PID,MIB_TCPROW_OWNER_PID>(query,AF_INET,port,api);
                result.ipv6=capture_family<MIB_TCP6TABLE_OWNER_PID,MIB_TCP6ROW_OWNER_PID>(query,AF_INET6,port,api);
            }
        }
    }catch(...) { result.ipv4.status=result.ipv6.status=ERROR_UNHANDLED_EXCEPTION; }
    return result;
}
inline void write_family(std::ostream& out,const Family& family) {
    out<<"status="<<family.status<<" candidate_pids=";
    if(family.status!=NO_ERROR)out<<"unavailable";
    else if(!family.count)out<<"none";
    else for(std::size_t i=0;i<family.count;++i) { if(i)out<<','; out<<family.pids[i]; }
    if(family.truncated)out<<" (truncated)";
}
inline void write_snapshot(std::ostream& out,const Snapshot& snapshot) {
    out<<"ipv4={";write_family(out,snapshot.ipv4);
    out<<"} ipv6={";write_family(out,snapshot.ipv6);
    out<<"} release_error="<<snapshot.release_error;
}
inline void report(std::ostream& out,const char* reason,DWORD status,unsigned short port,
                   DWORD caller_pid,const Snapshot& before,const Snapshot& after) noexcept {
    try {
        out<<"SHUTDOWN_LISTENER_OWNER_DIAGNOSTIC "<<reason<<" status="<<status
           <<" port="<<port<<" caller_pid="<<caller_pid<<" before={";
        write_snapshot(out,before);out<<"} after={";write_snapshot(out,after);out<<"}\n";
    }catch(...) { /* Diagnostic output must never replace the lifecycle failure. */ }
}
} // namespace laureondo::tools::listener_diagnostics
