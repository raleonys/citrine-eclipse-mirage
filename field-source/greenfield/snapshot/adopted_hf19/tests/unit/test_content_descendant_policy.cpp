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

    constexpr char nested_only[]=R"CFG("Config2" {
      "Content" {
        "A" { "Inner" { "Name" "Nested" "Version" "9" "State" "NEST" } }
      }
    })CFG";
    if(!load(nested_only,sizeof(nested_only)-1u)) return 1;
    // The entry remains a recognized Content record, but descendant leaves are
    // non-authoritative and therefore cannot replace Phase209 defaults.
    if(!expect_one("A","GAME","0","INSTALLED")) return 2;

    constexpr char direct_then_nested[]=R"CFG("Config2" {
      "Content" {
        "A" {
          "Name" "Direct" "Version" "1" "State" "READY"
          "Inner" { "Name" "Nested" "Version" "9" "State" "NEST" }
        }
      }
    })CFG";
    if(!load(direct_then_nested,sizeof(direct_then_nested)-1u)) return 3;
    if(!expect_one("A","Direct","1","READY")) return 4;

    clear_behavioral_service_state_for_test();
    return 0;
}
