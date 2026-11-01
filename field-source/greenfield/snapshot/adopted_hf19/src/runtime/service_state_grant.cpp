#include "service_state_internal.hpp"
#include <stdint.h>

namespace preloader_l { namespace runtime {
using namespace behavioral_internal;

bool preview_grant_achievement_locked(ServiceTextView id,int32_t increment,AchievementRecordView* out,bool* mutated_now,bool* completed_now) noexcept {
    if(out)*out=AchievementRecordView{};
    if(mutated_now)*mutated_now=false;
    if(completed_now)*completed_now=false;
    if(!id.data||!id.size)return false;
    static constexpr char empty[]="";
    static constexpr char zero_date[]="0000-00-00T00:00:00";
    static constexpr char timestamp_shape[]="2000-01-01T00:00:00";
    AchievementRecordView v{};
    AchievementRecord* r=find_active_achievement(id);
    if(r){export_achievement(*r,&v);}else{
        v=AchievementRecordView{id,{empty,0},{empty,0},{empty,0},{empty,0},
            {zero_date,sizeof(zero_date)-1u},{zero_date,sizeof(zero_date)-1u},increment,0,0};
    }
    const bool was_complete=v.count!=0;
    if(v.progress<v.total){
        if(mutated_now)*mutated_now=true;
        const uint32_t sum=static_cast<uint32_t>(v.progress)+static_cast<uint32_t>(increment);
        v.progress=static_cast<int32_t>(sum);
        v.grant_date={timestamp_shape,sizeof(timestamp_shape)-1u};
        if(v.progress>=v.total){v.count=1;v.progress=v.total;}
    }
    if(completed_now)*completed_now=!was_complete&&v.count!=0;
    if(out)*out=v;
    return true;
}

namespace {
struct GrantTxn {
    AchievementRecord* existing;
    AchievementRecord* fresh_record;
    OwnedText fresh_grant_date;
    int32_t new_count;
    int32_t new_progress;
    bool inserted;
    bool mutated;
};
GrantTxn* as_grant_txn(GrantAchievementTransaction* p) noexcept { return reinterpret_cast<GrantTxn*>(p); }
GrantAchievementTransaction* public_grant_txn(GrantTxn* p) noexcept { return reinterpret_cast<GrantAchievementTransaction*>(p); }
}

bool prepare_grant_achievement_locked(ServiceTextView id,int32_t increment,ServiceTextView timestamp,GrantAchievementTransaction** transaction,AchievementRecordView* out,bool* mutated_now,bool* completed_now,bool* inserted_now) noexcept {
    if(transaction)*transaction=nullptr;
    if(out)*out=AchievementRecordView{};
    if(mutated_now)*mutated_now=false;
    if(completed_now)*completed_now=false;
    if(inserted_now)*inserted_now=false;
    if(!transaction||!id.data||!id.size||(!timestamp.data&&timestamp.size))return false;
    auto* tx=static_cast<GrantTxn*>(alloc(sizeof(GrantTxn)));if(!tx)return false;*tx=GrantTxn{};
    AchievementRecord* r=find_active_achievement(id);
    tx->existing=r;tx->inserted=(r==nullptr);
    if(!r){
        r=create_achievement(id,{"",0},increment);
        if(!r){dealloc(tx,sizeof(*tx));return false;}
        tx->fresh_record=r;
        if(!reserve_active_achievement_slot()){free_achievement(r);dealloc(tx,sizeof(*tx));return false;}
    }
    const bool was_complete=r->count!=0;
    tx->new_count=r->count;tx->new_progress=r->progress;
    if(r->progress<r->total){
        tx->mutated=true;
        const uint32_t sum=static_cast<uint32_t>(r->progress)+static_cast<uint32_t>(increment);
        tx->new_progress=static_cast<int32_t>(sum);
        if(tx->new_progress>=r->total){tx->new_count=1;tx->new_progress=r->total;}
        if(tx->inserted){
            OwnedText fresh{};if(!set_text(fresh,timestamp)){free_achievement(tx->fresh_record);dealloc(tx,sizeof(*tx));return false;}
            free_text(tx->fresh_record->grant_date);tx->fresh_record->grant_date=fresh;
            tx->fresh_record->count=tx->new_count;tx->fresh_record->progress=tx->new_progress;
        }else{
            if(!set_text(tx->fresh_grant_date,timestamp)){dealloc(tx,sizeof(*tx));return false;}
        }
    }
    AchievementRecordView projected{};
    if(tx->inserted){export_achievement(*tx->fresh_record,&projected);}
    else{
        export_achievement(*r,&projected);
        if(tx->mutated){projected.grant_date=view(tx->fresh_grant_date);projected.count=tx->new_count;projected.progress=tx->new_progress;}
    }
    if(mutated_now)*mutated_now=tx->mutated;
    if(completed_now)*completed_now=!was_complete&&projected.count!=0;
    if(inserted_now)*inserted_now=tx->inserted;
    if(out)*out=projected;
    *transaction=public_grant_txn(tx);
    return true;
}

void commit_grant_achievement_locked(GrantAchievementTransaction* transaction) noexcept {
    GrantTxn* tx=as_grant_txn(transaction);if(!tx)return;
    if(tx->inserted){
        append_reserved_active_achievement(tx->fresh_record);
        tx->fresh_record=nullptr;
    }else if(tx->mutated&&tx->existing){
        free_text(tx->existing->grant_date);tx->existing->grant_date=tx->fresh_grant_date;tx->fresh_grant_date=OwnedText{};
        tx->existing->count=tx->new_count;tx->existing->progress=tx->new_progress;
    }
    dealloc(tx,sizeof(*tx));
}
void release_grant_achievement_transaction(GrantAchievementTransaction* transaction) noexcept {
    GrantTxn* tx=as_grant_txn(transaction);if(!tx)return;
    free_achievement(tx->fresh_record);free_text(tx->fresh_grant_date);dealloc(tx,sizeof(*tx));
}

bool grant_achievement_locked(ServiceTextView id,int32_t increment,ServiceTextView timestamp,AchievementRecordView* out,bool* mutated_now,bool* completed_now) noexcept {
    GrantAchievementTransaction* tx=nullptr;bool inserted=false;
    if(!prepare_grant_achievement_locked(id,increment,timestamp,&tx,out,mutated_now,completed_now,&inserted))return false;
    (void)inserted;commit_grant_achievement_locked(tx);return true;
}
size_t achievement_record_count_locked() noexcept {return active_achievement_count();}
bool achievement_record_at_locked(size_t i,AchievementRecordView* out) noexcept {return active_achievement_at(i,out);}

} }
