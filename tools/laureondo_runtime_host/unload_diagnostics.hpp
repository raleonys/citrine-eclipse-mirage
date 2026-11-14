// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#ifndef NDEBUG
#include "../../src/platform/unload_scan_record.hpp"
#include <windows.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <ostream>

namespace laureondo::tools::unload_diagnostics {
namespace wire=laureondo::unload_diagnostics;
struct RestoreError {DWORD value=GetLastError();~RestoreError(){SetLastError(value);}};
inline bool read_image(std::uintptr_t address,void* output,std::size_t size) noexcept {
 SIZE_T copied=0;
 return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<LPCVOID>(address),output,size,&copied)&&copied==size;
}
template<class Read> bool find_record(std::uintptr_t base,Read read,std::uintptr_t& result) noexcept {
 // Search only bounded, writable, non-executable PE sections in the loaded
 // image. A versioned private record avoids adding a management DLL export.
 IMAGE_DOS_HEADER dos{};
 if(!read(base,&dos,sizeof dos)||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<0||dos.e_lfanew>1024*1024||
    base>(std::numeric_limits<std::uintptr_t>::max)()-static_cast<std::uintptr_t>(dos.e_lfanew))return false;
 IMAGE_NT_HEADERS64 nt{};
 if(!read(base+static_cast<std::uintptr_t>(dos.e_lfanew),&nt,sizeof nt)||nt.Signature!=IMAGE_NT_SIGNATURE||
    nt.FileHeader.Machine!=IMAGE_FILE_MACHINE_AMD64||nt.OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR64_MAGIC||
    nt.FileHeader.SizeOfOptionalHeader<sizeof(IMAGE_OPTIONAL_HEADER64)||
    nt.FileHeader.NumberOfSections==0||nt.FileHeader.NumberOfSections>96)return false;
 const std::uint64_t size=nt.OptionalHeader.SizeOfImage;
 const std::uint64_t table=static_cast<std::uint64_t>(dos.e_lfanew)+sizeof(DWORD)+sizeof(IMAGE_FILE_HEADER)+nt.FileHeader.SizeOfOptionalHeader;
 if(size<sizeof nt||size>256ULL*1024*1024||base>(std::numeric_limits<std::uintptr_t>::max)()-size||
    table>size||nt.FileHeader.NumberOfSections>(size-table)/sizeof(IMAGE_SECTION_HEADER))return false;
 std::uintptr_t found=0;std::uint64_t scanned=0;
 for(unsigned i=0;i<nt.FileHeader.NumberOfSections;++i){
  IMAGE_SECTION_HEADER section{};
  if(!read(base+table+i*sizeof section,&section,sizeof section))return false;
  if(!(section.Characteristics&IMAGE_SCN_MEM_WRITE)||(section.Characteristics&IMAGE_SCN_MEM_EXECUTE))continue;
  const auto span=std::max<std::uint64_t>(section.Misc.VirtualSize,section.SizeOfRawData);
  if(section.VirtualAddress>size||span>size-section.VirtualAddress||span>8ULL*1024*1024-scanned)return false;
  scanned+=span;
  if((base+section.VirtualAddress)%alignof(wire::Record))return false;
  for(std::uint64_t offset=0;offset+sizeof(wire::Snapshot)<=span;offset+=4096){
   std::array<unsigned char,4096+sizeof(wire::Snapshot)> block{};
   const auto count=static_cast<std::size_t>(std::min<std::uint64_t>(span-offset,block.size()));
   const auto address=base+section.VirtualAddress+offset;
   if(!read(address,block.data(),count))return false;
   for(std::size_t at=0;at<4096&&at+sizeof(wire::Snapshot)<=count;at+=alignof(wire::Record)){
    wire::Snapshot candidate{};std::memcpy(&candidate,block.data()+at,sizeof candidate);
    if(!wire::valid(candidate))continue;
    if(found)return false;
    found=address+at;
   }
  }
 }
 if(!found)return false;
 result=found;return true;
}
inline bool locate(HMODULE image,std::uintptr_t& address) noexcept {
 RestoreError error;
 return find_record(reinterpret_cast<std::uintptr_t>(image),read_image,address);
}
template<class Read> bool report(std::ostream& output,std::uintptr_t address,Read read) noexcept {
 wire::Snapshot value{};
 if(!wire::read_snapshot(address,read,value)||value.suspended||value.reason==0)return false;
 // Called by the host only after the bounded DllShutdown call returns. A
 // stalled output consumer cannot hold the scanner gate or a DLL callback.
 const auto flags=output.flags();
 try {
  output<<"UNLOAD_SCAN_BLOCKED reason="<<wire::reason_name(value.reason)
        <<" peer="<<std::dec<<value.peer<<" image_rva=0x"<<std::hex<<value.image_rva
        <<" slot="<<std::dec<<value.slot<<" tls_end_rva=0x"<<std::hex<<value.tls_end_rva
        <<" error="<<std::dec<<value.error<<" sequence="<<value.sequence<<'\n';
  output.flags(flags);return bool(output);
 }catch(...){output.flags(flags);return false;}
}
inline void report_after_shutdown(std::ostream& output,std::uintptr_t address) noexcept {
 RestoreError error;
 (void)report(output,address,read_image);
}
}
#endif
