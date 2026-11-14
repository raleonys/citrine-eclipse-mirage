// SPDX-License-Identifier: GPL-3.0-only

#include "tools/laureondo_runtime_host/listener_owners.hpp"
#include <cwchar>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace diag=laureondo::tools::listener_diagnostics;
namespace {
enum class Fault { none,load,resolve,resolve_throw,probe,query,query_throw,allocation,
                   growth,growth_forever,oversize,short_table,bad_count,bad_size,
                   release,release_throw,empty,many };
struct State {
    Fault fault=Fault::none;
    unsigned loads=0,releases=0,v4=0,v6=0,allocations=0;
    bool valid_arguments=true;
} state;
unsigned checks=0;
void require(bool value,const char* message) {
    ++checks;
    if(!value)throw std::runtime_error(message);
}
HMODULE WINAPI load(LPCWSTR name,HANDLE file,DWORD flags) {
    ++state.loads;
    state.valid_arguments&=std::wcscmp(name,L"iphlpapi.dll")==0&&!file&&flags==LOAD_LIBRARY_SEARCH_SYSTEM32;
    return state.fault==Fault::load?nullptr:reinterpret_cast<HMODULE>(1);
}
DWORD WINAPI last_error() { return ERROR_ACCESS_DENIED; }
BOOL WINAPI release(HMODULE module) {
    ++state.releases;
    state.valid_arguments&=module==reinterpret_cast<HMODULE>(1);
    if(state.fault==Fault::release_throw)throw std::runtime_error("release");
    return state.fault!=Fault::release;
}
std::vector<unsigned char> allocate(std::size_t bytes) {
    ++state.allocations;
    if(state.fault==Fault::allocation&&state.allocations==1)throw std::bad_alloc{};
    return std::vector<unsigned char>(bytes);
}
template<class Table,class Row>
DWORD table(PVOID output,PDWORD bytes,unsigned call) {
    const auto rows=state.fault==Fault::empty?0u:state.fault==Fault::many?40u:4u;
    const auto size=static_cast<DWORD>(offsetof(Table,table)+rows*sizeof(Row));
    if(!output) {
        *bytes=state.fault==Fault::oversize?diag::max_table_bytes+1:size;
        if(state.fault==Fault::probe)return ERROR_ACCESS_DENIED;
        return ERROR_INSUFFICIENT_BUFFER;
    }
    if(state.fault==Fault::query)return ERROR_ACCESS_DENIED;
    if(state.fault==Fault::query_throw)throw std::runtime_error("query");
    if(state.fault==Fault::growth_forever||(state.fault==Fault::growth&&call==2)) {
        *bytes+=sizeof(Row);
        return ERROR_INSUFFICIENT_BUFFER;
    }
    const DWORD declared=state.fault==Fault::bad_count?rows+1:rows;
    std::memcpy(output,&declared,sizeof declared);
    auto* data=static_cast<unsigned char*>(output);
    for(unsigned i=0;i<rows;++i) {
        Row row{};
        row.dwState=MIB_TCP_STATE_LISTEN;
        // Network-order bytes for 3216, independent of Winsock linkage.
        row.dwLocalPort=0x900c;
        row.dwOwningPid=(sizeof(Row)==sizeof(MIB_TCPROW_OWNER_PID)?100u:200u)+i;
        if(state.fault!=Fault::many) {
            if(i==1)row.dwLocalPort=0x910c; // Another port.
            if(i==2)row.dwState=MIB_TCP_STATE_ESTAB;
            if(i==3)row.dwOwningPid-=3; // A duplicate PID.
        }
        std::memcpy(data+offsetof(Table,table)+i*sizeof row,&row,sizeof row);
    }
    if(state.fault==Fault::short_table)*bytes=sizeof(DWORD)-1;
    else if(state.fault==Fault::bad_size)*bytes+=1;
    else *bytes=size;
    return NO_ERROR;
}
DWORD WINAPI query(PVOID output,PDWORD bytes,BOOL ordered,ULONG family,TCP_TABLE_CLASS kind,ULONG reserved) {
    state.valid_arguments&=!ordered&&kind==TCP_TABLE_OWNER_PID_LISTENER&&!reserved;
    if(family==AF_INET)return table<MIB_TCPTABLE_OWNER_PID,MIB_TCPROW_OWNER_PID>(output,bytes,++state.v4);
    if(family==AF_INET6)return table<MIB_TCP6TABLE_OWNER_PID,MIB_TCP6ROW_OWNER_PID>(output,bytes,++state.v6);
    state.valid_arguments=false;
    return ERROR_INVALID_PARAMETER;
}
FARPROC WINAPI resolve(HMODULE module,LPCSTR name) {
    state.valid_arguments&=module==reinterpret_cast<HMODULE>(1)&&std::strcmp(name,"GetExtendedTcpTable")==0;
    if(state.fault==Fault::resolve_throw)throw std::runtime_error("resolve");
    return state.fault==Fault::resolve?nullptr:reinterpret_cast<FARPROC>(&query);
}
diag::Snapshot capture(Fault fault=Fault::none) {
    state={};state.fault=fault;
    const diag::Imports api{&load,&resolve,&release,&last_error,&allocate};
    return diag::capture(3216,api);
}
void unavailable(Fault fault,DWORD expected) {
    const auto result=capture(fault);
    require(result.ipv4.status==expected&&result.ipv6.status==expected,"wrong unavailable status");
    require(result.ipv4.count==0&&result.ipv6.count==0,"failed snapshot advertised PIDs");
    require(state.releases==(fault==Fault::load?0u:1u),"diagnostic module lease leaked or over-released");
    require(state.valid_arguments,"invalid Windows API arguments");
}
struct ThrowingBuffer:std::streambuf {
    int_type overflow(int_type) override { throw std::runtime_error("output"); }
};
}
int main() {
    try {
        const auto normal=capture();
        require(normal.ipv4.status==NO_ERROR&&normal.ipv6.status==NO_ERROR,"both families must be queried");
        require(normal.ipv4.count==1&&normal.ipv4.pids[0]==100,"IPv4 filtering/deduplication failed");
        require(normal.ipv6.count==1&&normal.ipv6.pids[0]==200,"IPv6 filtering/deduplication failed");
        require(state.loads==1&&state.releases==1&&state.valid_arguments,"module/API ownership mismatch");
        const auto empty=capture(Fault::empty);
        require(empty.ipv4.status==NO_ERROR&&empty.ipv6.status==NO_ERROR&&!empty.ipv4.count&&!empty.ipv6.count,
                "empty successful table must differ from query failure");
        const auto many=capture(Fault::many);
        require(many.ipv4.count==32&&many.ipv6.count==32&&many.ipv4.truncated&&many.ipv6.truncated,
                "PID storage is not bounded or truncation is hidden");
        unavailable(Fault::load,ERROR_ACCESS_DENIED);
        unavailable(Fault::resolve,ERROR_ACCESS_DENIED);
        unavailable(Fault::resolve_throw,ERROR_UNHANDLED_EXCEPTION);
        unavailable(Fault::probe,ERROR_ACCESS_DENIED);
        unavailable(Fault::query,ERROR_ACCESS_DENIED);
        unavailable(Fault::query_throw,ERROR_UNHANDLED_EXCEPTION);
        unavailable(Fault::oversize,ERROR_INVALID_DATA);
        require(state.allocations==0,"oversized table caused an allocation");
        unavailable(Fault::short_table,ERROR_INVALID_DATA);
        unavailable(Fault::bad_count,ERROR_INVALID_DATA);
        unavailable(Fault::bad_size,ERROR_INVALID_DATA);
        const auto allocation=capture(Fault::allocation);
        require(allocation.ipv4.status==ERROR_NOT_ENOUGH_MEMORY&&allocation.ipv4.count==0,
                "allocation failure escaped or advertised a valid table");
        require(allocation.ipv6.status==NO_ERROR&&allocation.ipv6.pids[0]==200&&state.releases==1,
                "one family failure prevented the other query or leaked the module");
        const auto growth=capture(Fault::growth);
        require(growth.ipv4.status==NO_ERROR&&growth.ipv6.status==NO_ERROR&&state.v4==3&&state.v6==3,
                "table growth retry failed");
        unavailable(Fault::growth_forever,ERROR_INSUFFICIENT_BUFFER);
        require(state.v4==1+diag::query_attempts&&state.v6==1+diag::query_attempts,
                "table growth exceeded the query budget");
        const auto release_failure=capture(Fault::release);
        require(release_failure.release_error==ERROR_ACCESS_DENIED&&state.releases==1,
                "release failure was hidden or retried without a bound");
        const auto release_throw=capture(Fault::release_throw);
        require(release_throw.release_error==ERROR_UNHANDLED_EXCEPTION&&state.releases==1,
                "release exception escaped the lease destructor");
        std::ostringstream text;
        diag::report(text,"shutdown_failed",4,3216,77,normal,allocation);
        const auto output=text.str();
        require(output.find("status=4 port=3216 caller_pid=77")!=std::string::npos,"authoritative failure context lost");
        require(output.find("ipv4={status=0 candidate_pids=100}")!=std::string::npos&&
                output.find("ipv6={status=0 candidate_pids=200}")!=std::string::npos,"dual-family context missing");
        require(output.find("candidate_pids=unavailable")!=std::string::npos,"query failure masquerades as no listener");
        ThrowingBuffer buffer;std::ostream broken(&buffer);broken.exceptions(std::ios::badbit);
        diag::report(broken,"shutdown_failed",4,3216,77,normal,normal);
        require(broken.bad(),"throwing output fault was not exercised");
        std::cout<<"PASS: listener diagnostics IPv4/IPv6; bounded queries/PIDs; API, allocation, malformed table and output faults; module lease ("<<checks<<" checks)\n";
        return 0;
    }catch(const std::exception& error) { std::cerr<<"FAIL: "<<error.what()<<'\n';return 1; }
}
