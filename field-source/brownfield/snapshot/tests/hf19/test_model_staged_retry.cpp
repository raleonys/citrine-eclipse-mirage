#define _WIN32 1
#define PRELOADER_L_RELEASE_BOOTSTRAP 1
#include "src/config/runtime_configuration.cpp"
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <iostream>
namespace { void* owned=nullptr; bool fail_next=true;int attempts=0, frees=0; }
namespace preloader_l { namespace runtime {
bool release_model_service_policy_retryable(model_abi::ReleaseServiceObject548* s) noexcept {
    void* p=nullptr;std::memcpy(&p,s->content.storage,sizeof(p));
    if(!p)return true;
    ++attempts;
    if(fail_next){fail_next=false;return false;}
    assert(p==owned);std::free(owned);owned=nullptr;++frees;
    std::memset(s->content.storage,0,sizeof(p));return true;
}
void swap_model_service_configuration_policy(model_abi::ReleaseServiceObject548* a,
    model_abi::ReleaseServiceObject548* b) noexcept {
    auto save = a->content;a->content=b->content;b->content=save;
    auto doc = a->config.document; a->config.document=b->config.document; b->config.document=doc;
    a->config.document.node.document=reinterpret_cast<uintptr_t>(&a->config.document);
    b->config.document.node.document=reinterpret_cast<uintptr_t>(&b->config.document);
}
} }
int main(){
    using namespace preloader_l::config;
    preloader_l::runtime::model_abi::ReleaseServiceObject548 s{};
    owned=std::malloc(64);assert(owned);std::memcpy(s.content.storage,&owned,sizeof(owned));
    assert(try_begin_policy_write());
    assert(!release_or_defer_staged_model_policy(&s));
    assert(g_deferred_staged_policy_pending&&owned&&attempts==1);
    assert(release_or_defer_staged_model_policy(&s)); // empty original already drained
    assert(retry_deferred_staged_policy_cleanup());
    assert(!owned&&frees==1&&attempts==2&&!g_deferred_staged_policy_pending);
    end_policy_write(false);
    assert(runtime_configuration_generation()==1);
    std::cout<<"HF19_MODEL_STAGED_RETRY_SOURCE_LINKED PASS attempts="<<attempts<<" physical_frees="<<frees<<"\n";
}
