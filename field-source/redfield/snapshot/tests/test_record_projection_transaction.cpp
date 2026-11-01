// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/config.hpp"
#include <cstdlib>
#include <iostream>
#include <new>

// Fail one ordinary C++ allocation in the synchronous replace call. No
// production fault switch or allocator replacement enters the DLL.
namespace {
thread_local bool injecting=false;
thread_local std::size_t allocation_index=0;
thread_local std::size_t fail_index=0;
thread_local bool failed=false;
}
void* operator new(std::size_t bytes) {
    if(injecting && allocation_index++==fail_index) {
        failed=true;
        throw std::bad_alloc();
    }
    if(void* result=std::malloc(bytes?bytes:1))return result;
    throw std::bad_alloc();
}
void* operator new[](std::size_t bytes) {return ::operator new(bytes);}
void operator delete(void* pointer) noexcept {std::free(pointer);}
void operator delete[](void* pointer) noexcept {::operator delete(pointer);}
void operator delete(void* pointer,std::size_t) noexcept {::operator delete(pointer);}
void operator delete[](void* pointer,std::size_t) noexcept {::operator delete(pointer);}

using namespace preloader_l;
#define CHECK(x) do {if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
int main() {
    const std::string id="record-with-owned-id-longer-than-small-string-storage";
    const std::string large(160,'x');
    std::string source="\"Config2\" {\"Content\" {\""+id+"\" {\"Name\" \""+large+
        "\" \"Name\" \"\" \"Version\" \"2\" \"State\" \"REMOTE\"}"
        "\"nested\" {\"N\" {\"Name\" \"ignored\"}} \"unknown\" {\"Other\" \"x\"}"
        "\"empty\" {} \"\" {\"Name\" \"empty-id\"}}"
        "\"Entitlements\" {\""+id+"\" {\"Group\" \""+large+
        "\" \"Version\" \"2\" \"Type\" \""+large+
        "\" \"EntitlementTag\" \"raw&amp;tag\"} \""+id+
        "\" {\"Group\" \"\" \"Version\" \"\" \"Type\" \"\" \"EntitlementTag\" \"\"}"
        "\"nested\" {\"N\" {\"Group\" \"ignored\"}} \"unknown\" {\"Other\" \"x\"}"
        "\"empty\" {} \"\" {\"Group\" \"empty-id\"}}"
        "\"Achievements\" {\"AchievementNames\" {\""+id+"\" \""+large+
        "\" \""+id+"\" \"ignored-second-name\" \"only\" \"named\"}"
        "\"AchievementTotals\" {\""+id+"\" \"-2147483648\" \""+id+"\" \"+2147483647\"}}"
        "\"content\" {\"bad\" {\"Name\" \"wrong-case\"}}"
        "\"Other\" {\"Entitlements\" {\"bad\" {\"Type\" \"wrong-depth\"}}}}";
    ConfigStore store;
    CHECK(store.replace(R"("Config2" {"Content" {"old" {"Name" "old-name"}} "Entitlements" {"old" {"Group" "old-group"}} "Achievements" {"AchievementNames" {"old" "old-achievement"}}})")==ConfigResult::published);
    const auto old=store.snapshot();
    std::size_t failures=0;
    // Every allocation on the eventual success path is failed once. Keep the
    // old snapshot pinned and require atomic non-publication at every failure.
    for(std::size_t point=0;point<4096;++point) {
        allocation_index=0;fail_index=point;failed=false;injecting=true;
        const auto result=store.replace(source);
        injecting=false;
        if(failed) {
            CHECK(result==ConfigResult::resource && store.snapshot()==old);
            CHECK(old->content[0].display_name=="old-name" && old->entitlements[0].group=="old-group");
            CHECK(old->achievements.size()==1 && old->achievements[0].name=="old-achievement");
            ++failures;
            continue;
        }
        CHECK(result==ConfigResult::published && failures==point && failures>30);
        break;
    }
    const auto current=store.snapshot();CHECK(current!=old);
    CHECK(current->content.size()==4 && current->entitlements.size()==4);
    const auto& c=current->content;const auto& e=current->entitlements;
    const auto& a=current->achievements;
    CHECK(a.size()==3 && a[0].id==id && a[0].name==large && a[0].total==INT32_MIN);
    CHECK(a[1].id==id && a[1].name==large && a[1].total==INT32_MAX);
    CHECK(a[2].id=="only" && a[2].name=="named" && a[2].total==1);
    CHECK(c[0].content_id==id && c[0].display_name==large && c[0].version=="2" && c[0].state=="REMOTE");
    CHECK(e[0].item_id==id && e[0].group.empty() && e[0].version.empty() && e[0].type.empty() && e[0].entitlement_tag.empty());
    CHECK(c[1].content_id=="nested" && c[1].display_name=="GAME" && c[1].version=="0" && c[1].state=="INSTALLED");
    CHECK(e[1].item_id=="nested" && e[1].group.empty() && e[1].version=="0" && e[1].type=="DEFAULT" && e[1].entitlement_tag=="ONLINE_ACCESS");
    CHECK(c[2].content_id=="unknown" && e[2].item_id=="unknown");
    CHECK(c[3].content_id.empty() && c[3].display_name=="empty-id" && e[3].item_id.empty() && e[3].group=="empty-id");
    source.assign(source.size(),'!');CHECK(c[0].display_name==large && c[0].content_id==id);
    CHECK(a[0].id==id && a[0].name==large);
    CHECK(store.replace(R"("Config2" {"Content" {"partial" {"Name" "x"}})")==ConfigResult::invalid && store.snapshot()==current);
    CHECK(store.replace(R"("Config2" {})")==ConfigResult::published);
    CHECK(c[0].display_name==large && e[1].type=="DEFAULT" && old->content[0].content_id=="old");
    CHECK(a[0].name==large && old->achievements[0].name=="old-achievement");
    std::cout<<"CONFIG_PROJECTION_EMPTY_POLICY_OWNED_ORDER_AND_ALLOCATION_TRANSACTION_PASS allocation_points="<<failures<<"\n";
}
