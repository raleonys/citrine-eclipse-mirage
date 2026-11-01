#include "greenfield/core/resource_owners.hpp"
#include <cassert>
#include <cstdint>
#include <vector>
#include <iostream>

using greenfield::core::ResourceOwners;
struct Context {
  ResourceOwners* owners{};
  std::vector<std::uintptr_t> visited;
  std::uintptr_t fail_once{};
  bool failed{};
  bool reenter{};
  bool blocked_adopt{};
  bool blocked_drain{};
};
bool close_resource(void* p,std::uintptr_t ticket) noexcept {
  auto& ctx=*static_cast<Context*>(p);
  ctx.visited.push_back(ticket);
  if (ctx.reenter) {
    ctx.blocked_adopt=!ctx.owners->adopt(4567,close_resource,p);
    ctx.blocked_drain=ctx.owners->drain()==ResourceOwners::Drain::busy;
  }
  if(ticket==ctx.fail_once && !ctx.failed) { ctx.failed=true;return false; }
  return true;
}
int main() {
  ResourceOwners owners;
  Context ctx{&owners};
  assert(!owners.adopt(0,close_resource,&ctx));
  assert(!owners.adopt(1,nullptr,&ctx));
  assert(owners.adopt(1,close_resource,&ctx));
  assert(owners.adopt(2,close_resource,&ctx));
  assert(!owners.adopt(2,close_resource,&ctx));
  assert(owners.adopt(3,close_resource,&ctx));
  ctx.fail_once=2;
  ctx.reenter=true;
  assert(owners.drain()==ResourceOwners::Drain::retry_required);
  assert(ctx.visited==std::vector<std::uintptr_t>({3,2,1}));
  assert(ctx.blocked_adopt && ctx.blocked_drain);
  assert(owners.remaining()==1);
  assert(owners.drain()==ResourceOwners::Drain::complete);
  assert(ctx.visited==std::vector<std::uintptr_t>({3,2,1,2}));
  assert(owners.remaining()==0);
  assert(owners.drain()==ResourceOwners::Drain::complete);
  assert(ctx.visited.size()==4);
  for(std::size_t i=0;i<ResourceOwners::capacity;++i)
    assert(owners.adopt(i+100,close_resource,&ctx));
  assert(!owners.adopt(999,close_resource,&ctx));
  assert(owners.remaining()==ResourceOwners::capacity);
  ctx.reenter=false;
  assert(owners.drain()==ResourceOwners::Drain::complete);
  assert(owners.remaining()==0);
  std::cout<<"GF01_OWNER_FAIL_ONCE_RETRY_TEST_PASS\n";
}
