// SPDX-License-Identifier: GPL-3.0-only
#pragma once

// Private Debug image data, read by the lifecycle host without a DLL export.
// Release contains neither the record nor the reader protocol.
#ifndef NDEBUG
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace laureondo::unload_diagnostics {
inline constexpr std::uint64_t magic_first=0x43454d554e4c4431ULL;
inline constexpr std::uint64_t magic_second=0x5350414e54524331ULL;
enum class Reason:std::uint64_t {
 none, resume, thread_close, snapshot_close, image_rip, image_register,
 stack_query, stack_region, stack_budget, stack_protection, image_stack,
 stack_access, image_query, snapshot_open, thread_first, thread_open,
 thread_suspend, thread_context, thread_next
};
struct Snapshot {
 std::uint64_t magic1=magic_first,magic2=magic_second,version=1,bytes=96;
 std::uint64_t sequence=0,reason=0,peer=0,slot=0,image_rva=0,tls_end_rva=0,error=0,suspended=0;
};
static_assert(sizeof(Snapshot)==96);
static_assert(std::is_trivially_copyable_v<Snapshot>);
static_assert(std::atomic<std::uint64_t>::is_always_lock_free);
struct alignas(8) Record {
 const std::uint64_t magic1=magic_first,magic2=magic_second,version=1,bytes=sizeof(Snapshot);
 std::atomic<std::uint64_t> sequence{0},reason{0},peer{0},slot{0},image_rva{0},tls_end_rva{0},error{0},suspended{0};
 void publish(Reason why,std::uint64_t thread,std::uint64_t position,
              std::uint64_t reference,std::uint64_t tls,std::uint64_t code,bool held) volatile noexcept {
  // The scan try-gate serializes writers. All operations are lock-free stores;
  // no output, allocation, callbacks, threads or OS calls occur here.
  const auto prior=sequence.load(std::memory_order_relaxed);
  sequence.store(prior+1,std::memory_order_seq_cst);
  reason.store(static_cast<std::uint64_t>(why),std::memory_order_relaxed);
  peer.store(thread,std::memory_order_relaxed);slot.store(position,std::memory_order_relaxed);
  image_rva.store(reference,std::memory_order_relaxed);tls_end_rva.store(tls,std::memory_order_relaxed);
  error.store(code,std::memory_order_relaxed);suspended.store(held?1:0,std::memory_order_relaxed);
  sequence.store(prior+2,std::memory_order_release);
 }
};
static_assert(sizeof(Record)==sizeof(Snapshot));
static_assert(std::is_standard_layout_v<Record> && std::is_trivially_destructible_v<Record>);
static_assert(offsetof(Record,sequence)==offsetof(Snapshot,sequence));
static_assert(offsetof(Record,suspended)==offsetof(Snapshot,suspended));

inline bool valid(const Snapshot& value) noexcept {
 return value.magic1==magic_first&&value.magic2==magic_second&&value.version==1&&
        value.bytes==sizeof(Snapshot)&&value.reason<=static_cast<std::uint64_t>(Reason::thread_next)&&
        value.suspended<=1&&value.peer<=0xffffffffULL&&value.slot<=0xffffffffULL&&value.error<=0xffffffffULL;
}
template<class Read> bool read_snapshot(std::uintptr_t address,Read read,Snapshot& output) noexcept {
 if(address>(std::numeric_limits<std::uintptr_t>::max)()-sizeof(Snapshot))return false;
 // The host copies image memory through ReadProcessMemory, never dereferences
 // foreign C++ atomics. Reject a moving/torn copy after three bounded attempts.
 for(unsigned attempt=0;attempt<3;++attempt){
  std::uint64_t before=0,after=0;Snapshot candidate{};
  if(!read(address+offsetof(Snapshot,sequence),&before,sizeof before))return false;
  if(before&1)continue;
  if(!read(address,&candidate,sizeof candidate)||
     !read(address+offsetof(Snapshot,sequence),&after,sizeof after))return false;
  if(before==after&&candidate.sequence==before&&!(after&1)&&valid(candidate)){
   output=candidate;return true;
  }
 }
 return false;
}
inline const char* reason_name(std::uint64_t reason) noexcept {
 switch(static_cast<Reason>(reason)){
 case Reason::resume:return "resume";case Reason::thread_close:return "thread-close";
 case Reason::snapshot_close:return "snapshot-close";case Reason::image_rip:return "image-rip";
 case Reason::image_register:return "image-register";case Reason::stack_query:return "stack-query";
 case Reason::stack_region:return "stack-region";case Reason::stack_budget:return "stack-budget";
 case Reason::stack_protection:return "stack-protection";case Reason::image_stack:return "image-stack";
 case Reason::stack_access:return "stack-access";case Reason::image_query:return "image-query";
 case Reason::snapshot_open:return "snapshot-open";case Reason::thread_first:return "thread-first";
 case Reason::thread_open:return "thread-open";case Reason::thread_suspend:return "thread-suspend";
 case Reason::thread_context:return "thread-context";case Reason::thread_next:return "thread-next";
 default:return "none";
 }
}
}
#endif
