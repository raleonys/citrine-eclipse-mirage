#include "local_social_delivery_model.hpp"
#include <cassert>
using namespace preloader_l::runtime;
namespace {
constexpr uint8_t frame[]={0x3c,0x52,0x20,0x43,0x6f,0x64,0x65,0x3d,0x30,0x3e};
struct Writer {
    LocalSocialWireReceipt reply;
    LocalSocialStore* store;
    bool invalidate_epoch;
    unsigned calls;
};
LocalSocialWireReceipt callback(void* ctx,const uint8_t* buf,size_t n) noexcept {
    auto* p=static_cast<Writer*>(ctx);
    assert(buf==frame && n==sizeof(frame));
    ++p->calls;
    if(p->invalidate_epoch)p->store->reset_epoch();
    return p->reply;
}
void fresh(LocalSocialStore& s){
    assert(s.register_local_actor(10));assert(s.register_local_actor(20));
    s.set_enabled_for_test_only(true);
}
}
int main(){
    LocalSocialStore store;
    fresh(store);
    const auto trusted=store.trusted_lease_for_test_only(10);
    LocalSocialResult detail=LocalSocialResult::ok;
    Writer writer{{LocalSocialWireOutcome::complete_bytes,sizeof(frame)},&store,false,0};
    auto go=[&](LocalPrincipalLease lease,LocalSocialAction action){
        return model_social_delivery(store,lease,action,20,frame,sizeof(frame),callback,&writer,&detail);
    };
    const uint64_t start=store.generation();
    assert(model_social_delivery(store,trusted,LocalSocialAction::block_user,20,nullptr,
        sizeof(frame),callback,&writer,&detail)==LocalSocialModelDelivery::invalid_input);
    assert(writer.calls==0 && store.generation()==start && !store.blocked(10,20));
    assert(go({10,trusted.epoch,false},LocalSocialAction::block_user)==LocalSocialModelDelivery::prepare_rejected);
    assert(detail==LocalSocialResult::no_principal && writer.calls==0);
    writer.reply={LocalSocialWireOutcome::no_bytes,0};
    assert(go(trusted,LocalSocialAction::block_user)==LocalSocialModelDelivery::no_bytes_rollback);
    assert(!store.blocked(10,20) && store.generation()==start);
    writer.reply={LocalSocialWireOutcome::partial_bytes,3};
    assert(go(trusted,LocalSocialAction::block_user)==LocalSocialModelDelivery::partial_wire_ambiguous);
    assert(!store.blocked(10,20) && store.generation()==start);
    writer.reply={LocalSocialWireOutcome::complete_bytes,1};
    assert(go(trusted,LocalSocialAction::block_user)==LocalSocialModelDelivery::inconsistent_wire_receipt);
    assert(!store.blocked(10,20) && store.generation()==start);
    writer.reply={LocalSocialWireOutcome::partial_bytes,sizeof(frame)};
    assert(go(trusted,LocalSocialAction::block_user)==LocalSocialModelDelivery::inconsistent_wire_receipt);
    assert(!store.blocked(10,20) && store.generation()==start);
    writer.reply={LocalSocialWireOutcome::no_bytes,1};
    assert(go(trusted,LocalSocialAction::block_user)==LocalSocialModelDelivery::inconsistent_wire_receipt);
    writer.reply={LocalSocialWireOutcome::complete_bytes,sizeof(frame)};
    assert(go(trusted,LocalSocialAction::block_user)==LocalSocialModelDelivery::committed_local_only);
    assert(store.blocked(10,20) && store.generation()>start && detail==LocalSocialResult::ok);
    assert(go(trusted,LocalSocialAction::block_user)==LocalSocialModelDelivery::prepare_rejected);
    assert(detail==LocalSocialResult::already_exists);
    assert(go(trusted,LocalSocialAction::unblock_user)==LocalSocialModelDelivery::committed_local_only);
    assert(!store.blocked(10,20));
    assert(go(trusted,LocalSocialAction::request_friend)==LocalSocialModelDelivery::committed_local_only);
    assert(store.pending(10,20));
    assert(go(trusted,LocalSocialAction::send_invite)==LocalSocialModelDelivery::committed_local_only);
    assert(store.inbox(20,10));
    // Session reset after a complete write but before commit is not success:
    // the caller must not report consistent, committed local state.
    writer.invalidate_epoch=true;
    const auto result=go(trusted,LocalSocialAction::block_user);
    assert(result==LocalSocialModelDelivery::commit_after_wire_failed);
    assert(!store.blocked(10,20));
    assert(go(trusted,LocalSocialAction::block_user)==LocalSocialModelDelivery::prepare_rejected);
    assert(detail==LocalSocialResult::disabled);
    // New session/lease isolation.
    fresh(store);
    const auto current=store.trusted_lease_for_test_only(10);
    writer.invalidate_epoch=false;
    assert(go(current,LocalSocialAction::block_user)==LocalSocialModelDelivery::committed_local_only);
    return 0;
}
