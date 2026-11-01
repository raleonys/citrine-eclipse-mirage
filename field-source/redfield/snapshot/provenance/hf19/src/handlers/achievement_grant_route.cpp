#include "achievement_routes_internal.hpp"
#include "route_response_support.hpp"

namespace preloader_l { namespace handlers { namespace detail {
namespace {
struct GrantAchievementDeliveryReservationScope {
    bool held;
    GrantAchievementDeliveryReservationScope() noexcept : held(true) { preloader_l::runtime::acquire_grant_achievement_delivery(); }
    ~GrantAchievementDeliveryReservationScope() noexcept { if(held)preloader_l::runtime::release_grant_achievement_delivery(); }
    void transfer() noexcept { held=false; }
    GrantAchievementDeliveryReservationScope(const GrantAchievementDeliveryReservationScope&)=delete;
    GrantAchievementDeliveryReservationScope& operator=(const GrantAchievementDeliveryReservationScope&)=delete;
};
struct GrantBehavioralStateScope {
    bool held;
    GrantBehavioralStateScope() noexcept : held(true) { preloader_l::runtime::acquire_behavioral_service_state(); }
    ~GrantBehavioralStateScope() noexcept { if(held)preloader_l::runtime::release_behavioral_service_state(); }
    void release() noexcept { if(held){preloader_l::runtime::release_behavioral_service_state();held=false;} }
    void acquire() noexcept { if(!held){preloader_l::runtime::acquire_behavioral_service_state();held=true;} }
    GrantBehavioralStateScope(const GrantBehavioralStateScope&)=delete;
    GrantBehavioralStateScope& operator=(const GrantBehavioralStateScope&)=delete;
};
struct GrantAchievementTransactionScope {
    preloader_l::runtime::GrantAchievementTransaction* transaction;
    GrantAchievementTransactionScope() noexcept : transaction(nullptr) {}
    ~GrantAchievementTransactionScope() noexcept { if(transaction)preloader_l::runtime::release_grant_achievement_transaction(transaction); }
    void commit() noexcept { if(transaction){preloader_l::runtime::commit_grant_achievement_locked(transaction);transaction=nullptr;} }
    preloader_l::runtime::GrantAchievementTransaction* detach() noexcept { auto* p=transaction;transaction=nullptr;return p; }
    GrantAchievementTransactionScope(const GrantAchievementTransactionScope&)=delete;
    GrantAchievementTransactionScope& operator=(const GrantAchievementTransactionScope&)=delete;
};
struct PreparedAchievementEventScope {
    preloader_l::runtime::PreparedQueuedEvent* event;
    PreparedAchievementEventScope() noexcept : event(nullptr) {}
    ~PreparedAchievementEventScope() noexcept { if(event)preloader_l::runtime::release_prepared_achievement_event(event); }
    void commit() noexcept { if(event){preloader_l::runtime::commit_prepared_achievement_event(event);event=nullptr;} }
    preloader_l::runtime::PreparedQueuedEvent* detach() noexcept { auto* p=event;event=nullptr;return p; }
    PreparedAchievementEventScope(const PreparedAchievementEventScope&)=delete;
    PreparedAchievementEventScope& operator=(const PreparedAchievementEventScope&)=delete;
};
bool parse_i32(preloader_l::xml::TextSpan v,int32_t* out) noexcept {
    if(!out||!v.data||!v.size)return false;
    size_t i=0; bool neg=false;
    if(v.data[0]=='-'||v.data[0]=='+'){ neg=v.data[0]=='-'; if(++i==v.size)return false; }
    uint32_t x=0;const uint32_t limit=neg?UINT32_C(2147483648):UINT32_C(2147483647);
    for(;i<v.size;++i){const unsigned char c=static_cast<unsigned char>(v.data[i]);if(c<'0'||c>'9')return false;const uint32_t d=c-'0';if(x>(limit-d)/10u)return false;x=x*10u+d;}
    *out=neg?(x==UINT32_C(2147483648)?INT32_MIN:-static_cast<int32_t>(x)):static_cast<int32_t>(x);return true;
}
bool build_completion_event(const preloader_l::runtime::AchievementRecordView& a,
                            char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||!safe_service_text(a.id)||!safe_service_text(a.name)||
       !safe_service_text(a.description)||!safe_service_text(a.how_to)||
       !safe_service_text(a.image_id)||!safe_service_text(a.grant_date)||
       !safe_service_text(a.expiration))return false;
    char total[32]{},count[32]{},progress[32]{};
    const size_t tn=i32_text(a.total,total),cn=i32_text(a.count,count),pn=i32_text(a.progress,progress);
    StreamWriter w(output,capacity);
    w.append("<LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"");w.append_semantic(a.id);
    w.append("\" Total=\"");w.append(total,tn);w.append("\" Count=\"");w.append(count,cn);
    w.append("\" Progress=\"");w.append(progress,pn);w.append("\" GrantDate=\"");w.append_semantic(a.grant_date);
    w.append("\" Expiration=\"");w.append_semantic(a.expiration);w.append("\" ImageId=\"");w.append_semantic(a.image_id);
    w.append("\" Name=\"");w.append_semantic(a.name);w.append("\" Description=\"");w.append_semantic(a.description);
    w.append("\" HowTo=\"");w.append_semantic(a.how_to);w.append("\"/></Event></LSX>");
    return w.finish(written);
}

bool same_service_text(preloader_l::runtime::ServiceTextView a,preloader_l::runtime::ServiceTextView b) noexcept {
    if(a.size!=b.size||(!a.data&&a.size)||(!b.data&&b.size))return false;
    for(size_t i=0;i<a.size;++i)if(a.data[i]!=b.data[i])return false;
    return true;
}
bool append_achievement_watcher_record(StreamWriter& w,const preloader_l::runtime::AchievementRecordView& a) noexcept {
    if(a.progress==0)return true;
    if(!safe_service_text(a.id)||!safe_service_text(a.name)||!safe_service_text(a.grant_date))return false;
    char cnt[32]{},prog[32]{};const size_t cn=i32_text(a.count,cnt),pn=i32_text(a.progress,prog);
    w.append("<Achievement Id=\"");w.append_semantic(a.id);w.append("\" Count=\"");w.append(cnt,cn);
    w.append("\" Progress=\"");w.append(prog,pn);w.append("\" Grant=\"");w.append_semantic(a.grant_date);
    w.append("\" Name=\"");w.append_semantic(a.name);w.append("\"/>");
    return true;
}
bool build_achievement_watcher_projected_locked(const preloader_l::runtime::AchievementRecordView* projected,bool inserted,
                                                char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written)return false;
    const Span game_name=gameinfo_span("DISPLAY_NAME");
    const Span content_id=cfg_or("ContentId","");
    if(!safe_semantic_attr_value({game_name.data,game_name.size})||
       !safe_semantic_attr_value({content_id.data,content_id.size}))return false;
    StreamWriter w(output,capacity);
    w.append("<Achievements GameName=\"");w.append_semantic(game_name);w.append("\" ContentId=\"");w.append_semantic(content_id);w.append("\">");
    const size_t count=preloader_l::runtime::achievement_record_count_locked();
    bool used_projected=false;
    for(size_t i=0;i<count;++i){
        preloader_l::runtime::AchievementRecordView a{};
        if(!preloader_l::runtime::achievement_record_at_locked(i,&a))return false;
        if(projected&&!inserted&&same_service_text(a.id,projected->id)){a=*projected;used_projected=true;}
        if(!append_achievement_watcher_record(w,a))return false;
    }
    if(projected&&inserted&&!used_projected){if(!append_achievement_watcher_record(w,*projected))return false;}
    w.append("</Achievements>");
    return w.finish(written);
}


}

HandlerStatus invalidate_rendered_response(char* out,size_t cap,size_t* written) noexcept {
    if(out&&cap)out[0]='\0';
    if(written)*written=0;
    return HandlerStatus::response_error;
}

HandlerStatus format_grant_achievement_record(const preloader_l::xml::RequestView& r,
                                              const preloader_l::runtime::AchievementRecordView& rec,
                                              char* out,size_t cap,size_t* written) noexcept {
    if(!safe_service_text(rec.id)||!safe_service_text(rec.name)||!safe_service_text(rec.description)||
       !safe_service_text(rec.how_to)||!safe_service_text(rec.image_id)||!safe_service_text(rec.grant_date)||
       !safe_service_text(rec.expiration)){if(written)*written=0;return HandlerStatus::response_error;}
    char total[32]{},count[32]{},progress[32]{};
    const size_t tn=i32_text(rec.total,total),cn=i32_text(rec.count,count),pn=i32_text(rec.progress,progress);
    const Span parts[]={lit(" <LSX>\n  <Achievement Id=\""),semantic_span(rec.id),lit("\" Total=\""),Span{total,tn},lit("\" Count=\""),Span{count,cn},lit("\" Progress=\""),Span{progress,pn},lit("\" GrantDate=\""),semantic_span(rec.grant_date),lit("\" Expiration=\""),semantic_span(rec.expiration),lit("\" ImageId=\""),semantic_span(rec.image_id),lit("\" Name=\""),semantic_span(rec.name),lit("\" Description=\""),semantic_span(rec.description),lit("\" HowTo=\""),semantic_span(rec.how_to),lit("\"/>\n </LSX>\n")};
    return format_parts(r,parts,sizeof(parts)/sizeof(parts[0]),out,cap,written);
}

namespace {
HandlerStatus handle_grant_achievement_impl(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written,
                                            GrantAchievementDelivery* deferred) noexcept {
    if(deferred)*deferred=GrantAchievementDelivery{nullptr,nullptr,false};
    if(!eq(r.command,"GrantAchievement")){if(written)*written=0;return HandlerStatus::wrong_command;}
    // Audit3 S0320: a watcher may re-enter read-only achievement APIs, but a
    // recursive GrantAchievement would invalidate the outer prepared mutation.
    // Reject it before touching the nonrecursive delivery gate.
    if(preloader_l::runtime::achievement_watcher_callback_active()){
        if(written)*written=0;
        if(out&&cap)out[0]='\0';
        return HandlerStatus::response_error;
    }
    preloader_l::xml::TextSpan id{}; preloader_l::xml::TextSpan progress_text{};
    const auto id_status=preloader_l::xml::find_direct_child_text(r.command_content,"AchievementId",&id);
    if(id_status==preloader_l::xml::ChildTextStatus::malformed||id_status==preloader_l::xml::ChildTextStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
    if(id_status!=preloader_l::xml::ChildTextStatus::found){if(written)*written=0;return HandlerStatus::unsupported;}
    preloader_l::xml::SemanticValue semantic_id;
    const auto id_decode=semantic_id.assign_parsed(id);
    if(id_decode==preloader_l::xml::ValueCodecStatus::malformed_entity||id_decode==preloader_l::xml::ValueCodecStatus::invalid_character||id_decode==preloader_l::xml::ValueCodecStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
    if(id_decode!=preloader_l::xml::ValueCodecStatus::ok){if(written)*written=0;return HandlerStatus::response_error;}
    const auto id_value=semantic_id.view();
    int32_t increment=1;
    const auto progress_status=preloader_l::xml::find_direct_child_text(r.command_content,"Progress",&progress_text);
    if(progress_status==preloader_l::xml::ChildTextStatus::malformed||progress_status==preloader_l::xml::ChildTextStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
    preloader_l::xml::SemanticValue semantic_progress;
    if(progress_status==preloader_l::xml::ChildTextStatus::found){
        const auto progress_decode=semantic_progress.assign_parsed(progress_text);
        if(progress_decode==preloader_l::xml::ValueCodecStatus::malformed_entity||progress_decode==preloader_l::xml::ValueCodecStatus::invalid_character||progress_decode==preloader_l::xml::ValueCodecStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
        if(progress_decode!=preloader_l::xml::ValueCodecStatus::ok){if(written)*written=0;return HandlerStatus::response_error;}
        if(!parse_i32(semantic_progress.view(),&increment)){if(written)*written=0;return HandlerStatus::unsupported;}
    }

    // W7: side-effect-free measure preview participates in the same delivery
    // serialization boundary as render/commit. This prevents a measure pass
    // from straddling another session's post-send GrantAchievement commit.
    if(!out&&cap==0u){
        GrantAchievementDeliveryReservationScope delivery_reservation;
        BehavioralServiceStateScope store_scope;
        preloader_l::runtime::AchievementRecordView preview{};bool mutated=false,completed=false;
        if(!preloader_l::runtime::preview_grant_achievement_locked({id_value.data,id_value.size},increment,&preview,&mutated,&completed))return HandlerStatus::response_error;
        (void)mutated;(void)completed;
        return format_grant_achievement_record(r,preview,nullptr,0,written);
    }

    // Actual render paths reserve GrantAchievement mutation policy. For the
    // socket delivery path this reservation survives encode/send and prevents a
    // competing GrantAchievement or store replacement from invalidating the
    // prepared transaction while read-only queries continue to see committed state.
    GrantAchievementDeliveryReservationScope delivery_reservation;
    char now[20]{}; size_t now_n=0;
    if(!preloader_l::runtime::current_utc_timestamp(now,sizeof(now),&now_n))return HandlerStatus::response_error;
    GrantBehavioralStateScope store_scope;
    GrantAchievementTransactionScope mutation;
    preloader_l::runtime::AchievementRecordView rec{}; bool mutated=false; bool completed=false; bool inserted=false;
    if(!preloader_l::runtime::prepare_grant_achievement_locked({id_value.data,id_value.size},increment,{now,now_n},
           &mutation.transaction,&rec,&mutated,&completed,&inserted))return HandlerStatus::response_error;
    if(!safe_service_text(rec.id)||!safe_service_text(rec.name)||!safe_service_text(rec.description)||!safe_service_text(rec.how_to)||!safe_service_text(rec.image_id)||!safe_service_text(rec.grant_date)||!safe_service_text(rec.expiration))return HandlerStatus::response_error;

    const HandlerStatus rendered=format_grant_achievement_record(r,rec,out,cap,written);
    if(rendered!=HandlerStatus::ok)return rendered;

    PreparedAchievementEventScope prepared_event;
    if(completed){
        size_t event_n=0;(void)build_completion_event(rec,nullptr,0,&event_n);
        if(!event_n||event_n==static_cast<size_t>(-1))return invalidate_rendered_response(out,cap,written);
        HeapBytes event_storage;if(!event_storage.allocate(event_n+1u))return invalidate_rendered_response(out,cap,written);
        size_t actual=0;
        if(!build_completion_event(rec,event_storage.data,event_storage.capacity,&actual)||actual!=event_n||
           !preloader_l::runtime::prepare_achievement_event(event_storage.data,actual,&prepared_event.event))return invalidate_rendered_response(out,cap,written);
    }

    // The watcher is externally observable and cannot be rolled back. Preserve
    // the established pre-commit projected-snapshot contract, but do not call
    // external code while the behavioral-state lock is held. The outer grant
    // delivery reservation remains held, so the prepared transaction stays
    // stable. QueryAchievements may borrow that reservation during this exact
    // callback; recursive GrantAchievement is rejected above.
    if(mutated&&preloader_l::runtime::achievement_watcher_active()){
        size_t needed=0;(void)build_achievement_watcher_projected_locked(&rec,inserted,nullptr,0,&needed);
        if(!needed||needed==static_cast<size_t>(-1))return invalidate_rendered_response(out,cap,written);
        HeapBytes watcher_storage;if(!watcher_storage.allocate(needed+1u))return invalidate_rendered_response(out,cap,written);
        size_t actual=0;
        if(!build_achievement_watcher_projected_locked(&rec,inserted,watcher_storage.data,watcher_storage.capacity,&actual)||actual!=needed)
            return invalidate_rendered_response(out,cap,written);
        store_scope.release();
        if(!preloader_l::runtime::publish_achievement_watcher_snapshot(watcher_storage.data,actual))
            return invalidate_rendered_response(out,cap,written);
    }

    if(deferred){
        deferred->transaction=mutation.detach();
        deferred->event=prepared_event.detach();
        deferred->active=true;
        delivery_reservation.transfer();
        return HandlerStatus::ok;
    }

    // Direct/non-socket callers retain historical immediate-commit semantics.
    // A watcher callback may have temporarily released the state lock.
    store_scope.acquire();
    mutation.commit();
    prepared_event.commit();
    return HandlerStatus::ok;
}
}

HandlerStatus handle_grant_achievement(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    return handle_grant_achievement_impl(r,out,cap,written,nullptr);
}

HandlerStatus prepare_grant_achievement_delivery(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written,
                                                 GrantAchievementDelivery* delivery) noexcept {
    if(!delivery){if(written)*written=0;return HandlerStatus::invalid_request;}
    return handle_grant_achievement_impl(r,out,cap,written,delivery);
}

preloader_l::runtime::PreparedQueuedEvent* take_grant_achievement_delivery_event(GrantAchievementDelivery* delivery) noexcept {
    if(!delivery||!delivery->active)return nullptr;
    auto* event=delivery->event;delivery->event=nullptr;return event;
}

void commit_grant_achievement_delivery_store(GrantAchievementDelivery* delivery) noexcept {
    if(!delivery||!delivery->active||!delivery->transaction)return;
    preloader_l::runtime::acquire_behavioral_service_state();
    preloader_l::runtime::commit_grant_achievement_locked(delivery->transaction);
    preloader_l::runtime::release_behavioral_service_state();
    delivery->transaction=nullptr;
}

void finish_grant_achievement_delivery(GrantAchievementDelivery* delivery) noexcept {
    if(!delivery||!delivery->active)return;
    if(delivery->event){preloader_l::runtime::commit_prepared_achievement_event(delivery->event);delivery->event=nullptr;}
    delivery->active=false;
    preloader_l::runtime::release_grant_achievement_delivery();
}

void rollback_grant_achievement_delivery(GrantAchievementDelivery* delivery) noexcept {
    if(!delivery||!delivery->active)return;
    if(delivery->transaction){preloader_l::runtime::release_grant_achievement_transaction(delivery->transaction);delivery->transaction=nullptr;}
    if(delivery->event){preloader_l::runtime::release_prepared_achievement_event(delivery->event);delivery->event=nullptr;}
    delivery->active=false;
    preloader_l::runtime::release_grant_achievement_delivery();
}

size_t preview_grant_achievement_event_size(const preloader_l::xml::RequestView& r) noexcept {
    if(!eq(r.command,"GrantAchievement"))return 0;
    preloader_l::xml::TextSpan id{},progress_text{};
    const auto id_status=preloader_l::xml::find_direct_child_text(r.command_content,"AchievementId",&id);
    if(id_status!=preloader_l::xml::ChildTextStatus::found)return 0;
    preloader_l::xml::SemanticValue semantic_id;
    if(semantic_id.assign_parsed(id)!=preloader_l::xml::ValueCodecStatus::ok)return 0;
    const auto id_value=semantic_id.view();
    int32_t increment=1;
    const auto progress_status=preloader_l::xml::find_direct_child_text(r.command_content,"Progress",&progress_text);
    if(progress_status==preloader_l::xml::ChildTextStatus::malformed||progress_status==preloader_l::xml::ChildTextStatus::invalid_argument)return 0;
    preloader_l::xml::SemanticValue semantic_progress;
    if(progress_status==preloader_l::xml::ChildTextStatus::found){if(semantic_progress.assign_parsed(progress_text)!=preloader_l::xml::ValueCodecStatus::ok||!parse_i32(semantic_progress.view(),&increment))return 0;}
    BehavioralServiceStateScope store_scope;
    preloader_l::runtime::AchievementRecordView preview{};bool mutated=false,completed=false;
    if(!preloader_l::runtime::preview_grant_achievement_locked({id_value.data,id_value.size},increment,&preview,&mutated,&completed)||!completed)return 0;
    size_t n=0;(void)build_completion_event(preview,nullptr,0,&n);return n;
}


} } }
