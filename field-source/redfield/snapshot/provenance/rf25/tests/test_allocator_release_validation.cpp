// SPDX-License-Identifier: GPL-3.0-only
#include "allocator_ptd_support.hpp"
#include <cassert>
#include <cstdint>
using namespace redfield::runtime;

int main(){
 d2test::PtdFake p; d2test::start_ptd(p);
 d2test::HeapFake h;
 alignas(32) unsigned char storage[0x1300]{};

 // Valid large allocation still releases its recorded raw heap pointer.
 h.scripted_allocs={storage+5};
 auto good=allocate_block(0x1000u,AllocationRetryPolicy::outer_retry_only,d2test::ops(h));
 assert(good.pointer && good.aligned32);
 assert(release_block(good.pointer,0x1000u,d2test::ops(h))==AllocationReleaseStatus::ok);
 assert(h.frees.size()==1 && h.frees.back()==storage+5);

 // Bogus metadata must be rejected and never forwarded to heap_free.
 alignas(32) unsigned char fake_user[64]{};
 void* user=fake_user+32;
 reinterpret_cast<void**>(user)[-1]=reinterpret_cast<void*>(uintptr_t{0x12345678u});
 const auto before=h.frees.size();
 assert(release_block(user,0x1000u,d2test::ops(h))==AllocationReleaseStatus::malformed_large_allocation);
 assert(h.frees.size()==before);

 // Null, forward/equal and excessive-padding metadata are all rejected.
 reinterpret_cast<void**>(user)[-1]=nullptr;
 assert(release_block(user,0x1000u,d2test::ops(h))==AllocationReleaseStatus::malformed_large_allocation);
 reinterpret_cast<void**>(user)[-1]=user;
 assert(release_block(user,0x1000u,d2test::ops(h))==AllocationReleaseStatus::malformed_large_allocation);
 reinterpret_cast<void**>(user)[-1]=reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(user)-8u-0x20u);
 assert(release_block(user,0x1000u,d2test::ops(h))==AllocationReleaseStatus::malformed_large_allocation);
 assert(h.frees.size()==before);

 // Overflow/accounting mismatch is rejected before any free.
 assert(release_block(user,static_cast<size_t>(-1),d2test::ops(h))==AllocationReleaseStatus::malformed_large_allocation);
 assert(h.frees.size()==before);
 d2test::stop_ptd(p); return 0;
}
