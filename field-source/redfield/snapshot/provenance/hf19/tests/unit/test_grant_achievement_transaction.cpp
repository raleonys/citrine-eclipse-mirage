#include "preloader_l/runtime/service_state.hpp"
#include <cstring>
using namespace preloader_l::runtime;

static bool same(ServiceTextView v,const char* s){
    const size_t n=std::strlen(s); if(v.size!=n) return false;
    for(size_t i=0;i<n;++i) if(v.data[i]!=s[i]) return false;
    return true;
}

int main(){
    clear_behavioral_service_state_for_test();
    constexpr char cfg[]="\"Config2\" { \"Achievements\" { \"AchievementTotals\" { \"A1\" \"3\" } } }";
    BehavioralServiceStateCandidate* candidate=nullptr;
    if(!prepare_behavioral_service_state_cfg(cfg,sizeof(cfg)-1u,&candidate)||!candidate) return 1;
    commit_behavioral_service_state_candidate(candidate);

    acquire_behavioral_service_state();
    AchievementRecordView stored{};
    if(achievement_record_count_locked()!=1u||!achievement_record_at_locked(0,&stored)) return 2;
    if(!same(stored.id,"A1")||stored.total!=3||stored.count!=0||stored.progress!=0) return 3;

    GrantAchievementTransaction* tx=nullptr;
    AchievementRecordView projected{}; bool mutated=false,completed=false,inserted=false;
    const ServiceTextView id{"A1",2}, stamp{"2026-09-25T00:00:00",19};

    // Abort a non-completing mutation: projected progress changes, store does not.
    if(!prepare_grant_achievement_locked(id,2,stamp,&tx,&projected,&mutated,&completed,&inserted)) return 4;
    if(!tx||inserted||!mutated||completed||projected.total!=3||projected.count!=0||projected.progress!=2) return 5;
    release_grant_achievement_transaction(tx); tx=nullptr;
    if(!achievement_record_at_locked(0,&stored)||stored.count!=0||stored.progress!=0) return 6;

    // Commit the same mutation once.
    if(!prepare_grant_achievement_locked(id,2,stamp,&tx,&projected,&mutated,&completed,&inserted)) return 7;
    commit_grant_achievement_locked(tx); tx=nullptr;
    if(!achievement_record_at_locked(0,&stored)||stored.count!=0||stored.progress!=2||!same(stored.grant_date,"2026-09-25T00:00:00")) return 8;

    // Abort the completing mutation: retry-visible store remains Progress=2.
    if(!prepare_grant_achievement_locked(id,1,stamp,&tx,&projected,&mutated,&completed,&inserted)) return 9;
    if(inserted||!mutated||!completed||projected.count!=1||projected.progress!=3) return 10;
    release_grant_achievement_transaction(tx); tx=nullptr;
    if(!achievement_record_at_locked(0,&stored)||stored.count!=0||stored.progress!=2) return 11;

    // Commit completion once, then a retry is a no-op.
    if(!prepare_grant_achievement_locked(id,1,stamp,&tx,&projected,&mutated,&completed,&inserted)) return 12;
    commit_grant_achievement_locked(tx); tx=nullptr;
    if(!achievement_record_at_locked(0,&stored)||stored.count!=1||stored.progress!=3) return 13;
    if(!prepare_grant_achievement_locked(id,1,stamp,&tx,&projected,&mutated,&completed,&inserted)) return 14;
    if(inserted||mutated||completed||projected.count!=1||projected.progress!=3) return 15;
    release_grant_achievement_transaction(tx); tx=nullptr;

    // A staged new zero-total record also disappears on abort.
    const size_t before=achievement_record_count_locked();
    const ServiceTextView zero{"ZERO",4};
    if(!prepare_grant_achievement_locked(zero,0,stamp,&tx,&projected,&mutated,&completed,&inserted)) return 16;
    if(!inserted||mutated||completed||projected.total!=0||projected.count!=0||projected.progress!=0) return 17;
    release_grant_achievement_transaction(tx); tx=nullptr;
    if(achievement_record_count_locked()!=before) return 18;

    release_behavioral_service_state();
    clear_behavioral_service_state_for_test();
    return 0;
}
