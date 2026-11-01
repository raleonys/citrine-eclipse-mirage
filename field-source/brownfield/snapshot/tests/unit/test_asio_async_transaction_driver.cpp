// Stage04B: real production owner+completion+async driver, bounded external
// store/event recorders. This does NOT simulate a full Windows IOCP backend.
#include "socket_exchange_transaction_owner_private.hpp"
#include "socket_exchange_async_transaction.hpp"
#include <cstdlib>
#include <cstdio>
#include <new>
namespace {
int stores,finishes,rollbacks,events,retries,event_rollbacks,social_commits, social_rollbacks;
void require(bool value, const char* label) {
 if(!value){std::fprintf(stderr,"FAIL %s\n",label);std::exit(1);}
}
#define CHECK(x) do{++checks;require((x),#x);}while(0)
int checks=0;
void reset_counters(){stores=finishes=rollbacks=events=retries=event_rollbacks=social_commits=social_rollbacks=0;}
}
namespace preloader_l {namespace handlers {
void commit_grant_achievement_delivery_store(GrantAchievementDelivery* g) noexcept {++stores;g->active=false;}
void finish_grant_achievement_delivery(GrantAchievementDelivery*) noexcept {++finishes;}
void rollback_grant_achievement_delivery(GrantAchievementDelivery* g) noexcept {++rollbacks;g->active=false;}
}}
namespace preloader_l {namespace runtime {
void commit_queued_event_delivery(QueuedEventDelivery* e) noexcept {if(e->active){++events;e->active=false;}}
void rollback_queued_event_delivery(QueuedEventDelivery* e) noexcept {if(e->active){++event_rollbacks;e->active=false;}}
void resolve_queued_event_delivery_for_retry(QueuedEventDelivery* e) noexcept {if(e->active){++retries;e->active=false;}}
LocalSocialResult commit_local_social_mutation(const LocalSocialSession&,LocalSocialPending* p) noexcept {++social_commits;p->active=false;return LocalSocialResult::ok;}
void rollback_local_social_mutation(LocalSocialPending* p) noexcept {++social_rollbacks;p->active=false;}
}}
namespace w=preloader_l::protocol::detail;
struct DriverContext {
 const uint8_t* frames[2]{};
 size_t lengths[2]{};
 int queued=0,done=0;
 bool fail_start=false;
 w::DispatchTransactionResult outcome{};
 static bool start(void* ctx,const uint8_t* bytes,size_t n) noexcept {
    auto* s=static_cast<DriverContext*>(ctx);
    if(s->fail_start)return false;
    if(s->queued>=2)return false;
    s->frames[s->queued]=bytes;s->lengths[s->queued]=n;++s->queued;
    return true;
 }
 static void finish(void* ctx,const w::DispatchTransactionResult& r) noexcept {
    auto* s=static_cast<DriverContext*>(ctx);s->outcome=r;++s->done;
 }
};
static w::PreparedResponseTransaction* make_tx(bool event, bool social) {
 void* storage=std::malloc(sizeof(w::PreparedResponseTransaction));
 CHECK(storage!=nullptr);
 auto* t=new(storage) w::PreparedResponseTransaction{};
 static char owned_bytes[32]{};
 t->wire.data=owned_bytes;
 t->response_required=7;t->event_required=event?5:0;
 CHECK(w::begin_delivery_writes(&t->cursor,7,event?5:0));
 t->grant_delivery.active=true;t->grant_delivery.grant.active=true;
 t->generic_event_delivery.queue.active=event;
 t->social_delivery.active=social;
 t->social_delivery.pending.active=social;
 return t;
}
// Linux tests can't call the production Win32 transient allocator; the
// actual owner destructor is exercised, and allocation is reclaimed in test.
static void scenario(bool event,int branch,bool social=false) {
 reset_counters();
 DriverContext ctx{};
 auto* t=make_tx(event,social);
 w::PreparedAsyncTransaction op(t,&ctx,&DriverContext::start,&DriverContext::finish);
 if(branch==6) ctx.fail_start=true;
 CHECK(op.begin()==(branch!=6));
 if(branch==6) {
    CHECK(op.is_finished() && ctx.done==1 && stores==0 && retries==0);
    // driver calls source owner delete through production allocator on Windows;
    // this test runs on POSIX, where it only calls the destructor.
    return;
 }
 CHECK(ctx.queued==1&&op.has_pending_write()&&ctx.lengths[0]==7u);
 CHECK(stores==0&&events==0);
 if(branch==1) op.request_stop();
 if(branch==2) {op.completed(false,7);}
 else if(branch==3) {op.completed(true,3);}
 else op.completed(true,7);
 if(branch==2||branch==3){
    CHECK(op.is_finished() && stores==0 && ctx.done==1);
    op.completed(true,7);CHECK(ctx.done==1);
    return;
 }
 if(branch==1){
    CHECK(stores==1 && events==0 && retries==(event?1:0));
    CHECK(ctx.queued==1&&op.is_finished()&&ctx.done==1);
    CHECK(social_commits==0);
    return;
 }
 CHECK(stores==1);
 if(!event){CHECK(op.is_finished()&&ctx.done==1&&events==0);return;}
 CHECK(ctx.queued==2 && op.has_pending_write() && ctx.lengths[1]==5u);
 CHECK(events==0 && retries==0 && social_commits==0);
 if(branch==4){op.completed(false,5);CHECK(retries==1&&events==0);}
 else if(branch==5){op.completed(true,2);CHECK(retries==1&&events==0);}
 else if(branch==7){op.request_stop();op.completed(false,0);CHECK(retries==1&&events==0);}
 else {op.completed(true,5);CHECK(events==1&&retries==0);}
 CHECK(op.is_finished()&&ctx.done==1);
 CHECK(social_commits==(social && branch==0?1:0));
 op.completed(true,5);CHECK(ctx.done==1&&stores==1);
}
int main(){
 scenario(false,0);scenario(false,1);scenario(false,2);scenario(false,3);
 scenario(true,0,true);scenario(true,1,true);scenario(true,2);scenario(true,3);
 scenario(true,4,true);scenario(true,5,true);scenario(true,6);scenario(true,7);
 std::printf("HF12_STAGE04B_ASYNC_TRANSACTION_DRIVER_MODEL PASS assertions=%d\n",checks);
}
