#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/runtime/achievement_watcher.hpp"
#include "preloader_l/runtime/service_state.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstdlib>
#include <cstring>

using namespace preloader_l;

namespace {
size_t g_heap_calls=0;
size_t g_fail_heap_call=0;
size_t g_watcher_calls=0;
bool g_watcher_result=true;
bool g_watcher_reenter=false;
bool g_watcher_reentry_ok=true;

bool parse(const char* s,xml::RequestView* out){
    return xml::parse_request_view(s,std::strlen(s),out)==xml::RequestStatus::ok;
}
bool same(runtime::ServiceTextView v,const char* s){
    const size_t n=std::strlen(s); if(v.size!=n)return false;
    for(size_t i=0;i<n;++i)if(v.data[i]!=s[i])return false;
    return true;
}
bool watcher_sink(const char* xml,size_t length) noexcept {
    ++g_watcher_calls;
    if(!xml||!length)return false;
    if(g_watcher_reenter){
        // S0320: read-only API reentry is admitted while the outer projected
        // mutation is still pre-commit. It must observe committed progress 0.
        constexpr char query[]="<LSX><Request id=\"q\"><QueryAchievements><All>true</All></QueryAchievements></Request></LSX>";
        xml::RequestView qr{};char qout[2048]{};size_t qn=0;
        if(!parse(query,&qr)||handlers::handle_query_achievements(qr,qout,sizeof(qout),&qn)!=handlers::HandlerStatus::ok||
           !std::strstr(qout,"Progress=\"0\""))g_watcher_reentry_ok=false;
        // Recursive mutation is explicitly rejected before the nonrecursive
        // delivery gate rather than self-deadlocking or corrupting outer state.
        constexpr char nested[]="<LSX><Request id=\"nested\"><GrantAchievement><AchievementId>A1</AchievementId></GrantAchievement></Request></LSX>";
        xml::RequestView nr{};char nout[256]={'X','\0'};size_t nn=77;
        if(!parse(nested,&nr)||handlers::handle_grant_achievement(nr,nout,sizeof(nout),&nn)!=handlers::HandlerStatus::response_error||nn!=0u)
            g_watcher_reentry_ok=false;
    }
    return g_watcher_result;
}
void reset_heap_failure(size_t fail_call){ g_heap_calls=0; g_fail_heap_call=fail_call; }

bool reset_state(){
    runtime::clear_achievement_watcher_sink_for_test();
    runtime::clear_queued_events_for_test();
    runtime::clear_behavioral_service_state_for_test();
    config::clear_runtime_overrides();
    config::set_runtime_online_override(config::literal_value("true"));
    constexpr char cfg[]="\"Config2\" { \"Achievements\" { \"AchievementTotals\" { \"A1\" \"1\" } } }";
    runtime::BehavioralServiceStateCandidate* candidate=nullptr;
    if(!runtime::prepare_behavioral_service_state_cfg(cfg,sizeof(cfg)-1u,&candidate)||!candidate)return false;
    runtime::commit_behavioral_service_state_candidate(candidate);
    return true;
}
bool stored_is(int count,int progress){
    runtime::acquire_behavioral_service_state();
    runtime::AchievementRecordView r{};
    const bool ok=runtime::achievement_record_count_locked()==1u &&
                  runtime::achievement_record_at_locked(0,&r) && same(r.id,"A1") &&
                  r.count==count && r.progress==progress;
    runtime::release_behavioral_service_state();
    return ok;
}
}

extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(1); }
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){
    ++g_heap_calls;
    if(g_fail_heap_call && g_heap_calls==g_fail_heap_call)return nullptr;
    return std::malloc(n);
}
extern "C" int HeapFree(void*,unsigned long,void* p){ std::free(p); return 1; }
extern "C" void GetSystemTimeAsFileTime(void* p){
    struct FT{ unsigned low,high; };
    // 2024-01-01-ish fixed FILETIME; only determinism matters here.
    auto* ft=static_cast<FT*>(p); const unsigned long long ticks=133485408000000000ULL;
    ft->low=static_cast<unsigned>(ticks); ft->high=static_cast<unsigned>(ticks>>32);
}

namespace preloader_l { namespace config {
// The handler only needs the Online override in this regression. Keep the
// runtime-configuration fallback absent so this test does not depend on the
// full configuration-file subsystem.
ValueView runtime_configuration_setting(const char*) noexcept { return missing_value(); }
} }

int main(){
    constexpr char req[]="<LSX><Request id=\"ga\"><GrantAchievement><AchievementId>A1</AchievementId></GrantAchievement></Request></LSX>";
    xml::RequestView r{}; if(!parse(req,&r))return 1;
    char out[2048]{}; size_t need=0;

    // F319-P111-001: raw invalid XML characters in semantic child text are
    // rejected before mutation and classified as an invalid request.
    const char bad_req[]="<LSX><Request id=\"bad\"><GrantAchievement><AchievementId>A\x01" "B</AchievementId></GrantAchievement></Request></LSX>";
    xml::RequestView bad{}; if(!parse(bad_req,&bad))return 25;
    if(!reset_state())return 26;
    size_t bad_n=9; out[0]='X';
    if(handlers::handle_grant_achievement(bad,out,sizeof(out),&bad_n)!=handlers::HandlerStatus::invalid_request||bad_n!=0)return 27;
    if(!stored_is(0,0)||runtime::queued_event_count_for_test()!=0u)return 28;

    // Size query is side-effect free and provides the exact logical response size.
    if(!reset_state())return 2;
    if(handlers::handle_grant_achievement(r,nullptr,0,&need)!=handlers::HandlerStatus::response_error||need==0)return 3;
    if(!stored_is(0,0)||runtime::queued_event_count_for_test()!=0u)return 4;

    // Tiny and exact-capacity render failures must not mutate state or FIFO.
    size_t n=777; char tiny[2]={'X','\0'};
    if(handlers::handle_grant_achievement(r,tiny,sizeof(tiny),&n)!=handlers::HandlerStatus::response_error)return 5;
    if(!stored_is(0,0)||runtime::queued_event_count_for_test()!=0u)return 6;
    char* exact=static_cast<char*>(std::malloc(need)); if(!exact)return 7;
    std::memset(exact,'X',need); n=0;
    if(handlers::handle_grant_achievement(r,exact,need,&n)!=handlers::HandlerStatus::response_error||n!=need){std::free(exact);return 8;}
    if(!stored_is(0,0)||runtime::queued_event_count_for_test()!=0u){std::free(exact);return 9;}
    std::free(exact);

    // Retry after failed render commits exactly once and enqueues exactly one completion event.
    reset_heap_failure(0); n=0;
    if(handlers::handle_grant_achievement(r,out,sizeof(out),&n)!=handlers::HandlerStatus::ok||!n)return 10;
    if(!stored_is(1,1)||runtime::queued_event_count_for_test()!=1u)return 11;
    const size_t first_n=n;
    if(handlers::handle_grant_achievement(r,out,sizeof(out),&n)!=handlers::HandlerStatus::ok||n!=first_n)return 12;
    if(!stored_is(1,1)||runtime::queued_event_count_for_test()!=1u)return 13;

    // Failure while allocating the rendered completion-event staging buffer is atomic.
    if(!reset_state())return 14;
    reset_heap_failure(1); n=0;
    if(handlers::handle_grant_achievement(r,out,sizeof(out),&n)!=handlers::HandlerStatus::response_error||n!=0||out[0]!='\0')return 15;
    if(!stored_is(0,0)||runtime::queued_event_count_for_test()!=0u)return 16;

    // Failure in prepare_achievement_event() itself is also atomic.
    if(!reset_state())return 17;
    reset_heap_failure(2); n=0;
    if(handlers::handle_grant_achievement(r,out,sizeof(out),&n)!=handlers::HandlerStatus::response_error||n!=0||out[0]!='\0')return 18;
    if(!stored_is(0,0)||runtime::queued_event_count_for_test()!=0u)return 19;

    // Watcher rejection is externally observable (one callback) but must leave
    // internal persistent state and the completion FIFO untouched.
    if(!reset_state())return 20;
    reset_heap_failure(0); g_watcher_calls=0; g_watcher_result=false;
    runtime::bind_achievement_watcher_sink(watcher_sink); n=0;
    if(handlers::handle_grant_achievement(r,out,sizeof(out),&n)!=handlers::HandlerStatus::response_error||n!=0||out[0]!='\0')return 21;
    if(g_watcher_calls!=1u||!stored_is(0,0)||runtime::queued_event_count_for_test()!=0u)return 22;
    runtime::clear_achievement_watcher_sink_for_test();

    // Retry after watcher rejection commits once.
    n=0;
    if(handlers::handle_grant_achievement(r,out,sizeof(out),&n)!=handlers::HandlerStatus::ok)return 23;
    if(!stored_is(1,1)||runtime::queued_event_count_for_test()!=1u)return 24;

    // Audit3 S0320: callback reentry does not self-deadlock. Read-only query
    // sees committed pre-outer state; recursive GrantAchievement fails fast;
    // the outer projected snapshot and final mutation still agree at 1/1.
    if(!reset_state())return 29;
    reset_heap_failure(0);g_watcher_calls=0;g_watcher_result=true;g_watcher_reenter=true;g_watcher_reentry_ok=true;
    runtime::bind_achievement_watcher_sink(watcher_sink);n=0;
    if(handlers::handle_grant_achievement(r,out,sizeof(out),&n)!=handlers::HandlerStatus::ok||!n)return 30;
    if(g_watcher_calls!=1u||!g_watcher_reentry_ok||!stored_is(1,1)||runtime::queued_event_count_for_test()!=1u)return 31;
    g_watcher_reenter=false;runtime::clear_achievement_watcher_sink_for_test();

    runtime::clear_queued_events_for_test(); runtime::clear_behavioral_service_state_for_test();
    config::clear_runtime_overrides();
    return 0;
}
