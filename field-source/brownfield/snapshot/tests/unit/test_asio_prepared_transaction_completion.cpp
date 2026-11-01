// Bounded source-linked Stage04A completion-owner test. Does not claim actual
// dispatcher/achievement store integration or Windows IOCP acceptance.
// Including the real shipping translation unit exercises actual owner logic;
// the narrow log-only mocks below stand in for external subsystems.
#include "socket_exchange_transaction_owner_private.hpp"
#include <cstdlib>
#include <cstdio>
namespace {
int g_store=0,g_grant_finish=0,g_grant_rollback=0;
int g_event_commit=0,g_event_retry=0,g_event_rollback=0;
int g_social_commit=0,g_social_rollback=0;
void expect(bool b,const char* message){if(!b){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
void reset(){g_store=g_grant_finish=g_grant_rollback=g_event_commit=g_event_retry=g_event_rollback=g_social_commit=g_social_rollback=0;}
}
namespace preloader_l { namespace handlers {
void commit_grant_achievement_delivery_store(GrantAchievementDelivery* g) noexcept {++g_store;g->active=false;}
void finish_grant_achievement_delivery(GrantAchievementDelivery*) noexcept {++g_grant_finish;}
void rollback_grant_achievement_delivery(GrantAchievementDelivery* g) noexcept {++g_grant_rollback;g->active=false;}
}}
namespace preloader_l { namespace runtime {
void commit_queued_event_delivery(QueuedEventDelivery* e) noexcept {if(e->active){++g_event_commit;e->active=false;}}
void rollback_queued_event_delivery(QueuedEventDelivery* e) noexcept {if(e->active){++g_event_rollback;e->active=false;}}
void resolve_queued_event_delivery_for_retry(QueuedEventDelivery* e) noexcept {if(e->active){++g_event_retry;e->active=false;}}
LocalSocialResult commit_local_social_mutation(const LocalSocialSession&,LocalSocialPending* s) noexcept {++g_social_commit;s->active=false;return LocalSocialResult::ok;}
void rollback_local_social_mutation(LocalSocialPending* s) noexcept {++g_social_rollback;s->active=false;}
}}
namespace w=preloader_l::protocol::detail;
static int assertions=0;
#define MUST(expr) do{++assertions;expect((expr),#expr);}while(0)
static void scenario(bool event_present,int failure_point,bool destroy_pending=false){
    reset();
    w::PreparedResponseTransaction t{};
    char bytes[16]={}; t.wire.data=bytes; // Linux test: destructor does not physically free, buffer is stack-owned.
    t.response_required=7; t.event_required=event_present?5:0;
    MUST(w::begin_delivery_writes(&t.cursor,t.response_required,t.event_required));
    t.grant_delivery.active=true;t.grant_delivery.grant.active=true;
    t.generic_event_delivery.queue.active=event_present;
    w::PreparedWireFrame out{};
    MUST(w::prepared_response_frame(&t,&out)&&out.length==7&&out.bytes==(const uint8_t*)bytes);
    w::DispatchTransactionResult result{};
    if(failure_point==1){
        MUST(w::complete_prepared_transaction_write(&t,false,7,&out,&result)==w::PreparedWriteDisposition::rejected);
        MUST(!result.response_sent&&g_store==0&&g_event_retry==0);
        MUST(w::complete_prepared_transaction_write(&t,true,7,&out,&result)==w::PreparedWriteDisposition::rejected);
        return;
    }
    if(failure_point==2){
        MUST(w::complete_prepared_transaction_write(&t,true,3,&out,&result)==w::PreparedWriteDisposition::rejected);
        MUST(!result.response_sent&&g_store==0);
        return;
    }
    auto a=w::complete_prepared_transaction_write(&t,true,7,&out,&result);
    MUST(result.response_sent&&result.bytes_sent==7&&g_store==1&&g_grant_finish==1);
    if(!event_present){MUST(a==w::PreparedWriteDisposition::done&&g_event_commit==0);return;}
    MUST(a==w::PreparedWriteDisposition::send_event&&out.length==5&&out.bytes==(uint8_t*)bytes+7);
    MUST(g_event_commit==0 && g_event_retry==0);
    if(destroy_pending){
        // stack destruction models callback owner going away between writes
        return;
    }
    if(failure_point==3){
        MUST(w::complete_prepared_transaction_write(&t,false,5,&out,&result)==w::PreparedWriteDisposition::rejected);
        MUST(result.response_sent&&result.bytes_sent==12&&g_event_retry==1&&g_event_commit==0);
    }else if(failure_point==4){
        MUST(w::complete_prepared_transaction_write(&t,true,2,&out,&result)==w::PreparedWriteDisposition::rejected);
        MUST(result.response_sent&&g_event_retry==1&&g_event_commit==0);
        MUST(result.bytes_sent==9);
    }else{
        MUST(w::complete_prepared_transaction_write(&t,true,5,&out,&result)==w::PreparedWriteDisposition::done);
        MUST(result.status==preloader_l::protocol::ExchangeStatus::ok&&result.bytes_sent==12);
        MUST(g_event_commit==1&&g_event_retry==0);
    }
    MUST(w::complete_prepared_transaction_write(&t,true,5,&out,&result)==w::PreparedWriteDisposition::rejected);
}
static void owner_teardown(bool response_complete){
    reset();
    void* memory=std::malloc(sizeof(w::PreparedResponseTransaction));
    MUST(memory!=nullptr);
    auto* t=new(memory) w::PreparedResponseTransaction{};
    char frame_bytes[20]={}; t->wire.data=frame_bytes;
    t->response_required=7;t->event_required=5;
    MUST(w::begin_delivery_writes(&t->cursor,7,5));
    t->grant_delivery.active=true;t->grant_delivery.grant.active=true;
    t->generic_event_delivery.queue.active=true;
    t->social_delivery.active=true;
    w::PreparedWireFrame view{};
    w::DispatchTransactionResult out{};
    if(response_complete){
        MUST(w::complete_prepared_transaction_write(t,true,7,&view,&out)
            ==w::PreparedWriteDisposition::send_event);
        MUST(g_store==1&&g_event_retry==0&&g_social_commit==0);
    }else{
        MUST(w::complete_prepared_transaction_write(t,false,7,&view,&out)
            ==w::PreparedWriteDisposition::rejected);
        MUST(g_store==0&&g_event_rollback==0);
    }
    w::destroy_prepared_response_transaction(t);
    std::free(memory);
    if(response_complete){
        MUST(g_event_retry==1&&g_event_rollback==0&&g_store==1);
        MUST(g_social_commit==0&&g_social_rollback==1);
    }else{
        MUST(g_event_retry==0&&g_event_rollback==1&&g_store==0);
        MUST(g_grant_rollback==1&&g_social_rollback==1);
    }
}
static void social_commit_only_after_full_transaction(){
    reset();
    w::PreparedResponseTransaction t{};
    char wire[12]={};t.wire.data=wire;t.response_required=7;t.event_required=5;
    MUST(w::begin_delivery_writes(&t.cursor,7,5));
    t.social_delivery.active=true;t.social_delivery.pending.active=true;
    w::PreparedWireFrame next{};w::DispatchTransactionResult result{};
    MUST(w::complete_prepared_transaction_write(&t,true,7,&next,&result)
        ==w::PreparedWriteDisposition::send_event);
    MUST(g_social_commit==0);
    MUST(w::complete_prepared_transaction_write(&t,true,5,&next,&result)
        ==w::PreparedWriteDisposition::done);
    MUST(g_social_commit==1&&result.bytes_sent==12);
}
int main(){
    scenario(false,0); scenario(false,1);scenario(false,2);
    scenario(true,0);scenario(true,1);scenario(true,2);scenario(true,3);scenario(true,4);
    owner_teardown(false);owner_teardown(true);
    social_commit_only_after_full_transaction();
    std::printf("HF12_STAGE04_PREPARED_TRANSACTION_MODEL PASS assertions=%d\n",assertions);
}
