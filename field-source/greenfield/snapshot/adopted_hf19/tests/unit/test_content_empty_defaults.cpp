#include "preloader_l/runtime/service_state.hpp"
#include <cstddef>
#include <cstring>

using namespace preloader_l::runtime;

static bool eq(ServiceTextView v,const char* s) noexcept {
    const std::size_t n=std::strlen(s);
    return v.size==n && (n==0u || (v.data && std::memcmp(v.data,s,n)==0));
}

static bool load(const char* cfg,std::size_t len) noexcept {
    BehavioralServiceStateCandidate* candidate=nullptr;
    if(!prepare_behavioral_service_state_cfg(cfg,len,&candidate) || !candidate) return false;
    commit_behavioral_service_state_candidate(candidate);
    return true;
}

static bool expect_one(const char* id,const char* name,const char* version,const char* state) noexcept {
    acquire_behavioral_service_state();
    const std::size_t count=content_record_count_locked();
    ContentRecordView r{};
    const bool ok=count==1u && content_record_at_locked(0u,&r) &&
                  eq(r.content_id,id) && eq(r.display_name,name) &&
                  eq(r.version,version) && eq(r.state,state);
    release_behavioral_service_state();
    return ok;
}

int main() {
    clear_behavioral_service_state_for_test();

    constexpr char empty_cfg[]=R"CFG("Config2" {
      "Content" {
        "OFFER_EMPTY" { "Name" "" "Version" "" "State" "" }
      }
    })CFG";
    if(!load(empty_cfg,sizeof(empty_cfg)-1u)) return 1;
    if(!expect_one("OFFER_EMPTY","GAME","0","INSTALLED")) return 2;

    constexpr char mixed_cfg[]=R"CFG("Config2" {
      "Content" {
        "OFFER_MIXED" { "Name" "Named" "Version" "" "State" "READY" }
      }
    })CFG";
    if(!load(mixed_cfg,sizeof(mixed_cfg)-1u)) return 3;
    if(!expect_one("OFFER_MIXED","Named","0","READY")) return 4;

    clear_behavioral_service_state_for_test();
    return 0;
}
