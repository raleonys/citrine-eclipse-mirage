#include "preloader_l/runtime/service_storage.hpp"
#include "preloader_l/runtime/reference_owner.hpp"
#include <assert.h>
#include <stdint.h>

using namespace preloader_l::runtime;
namespace {
uint32_t destroyed=0, deleted=0, scalar=0;
struct TestControl final : ModelServiceControl558 {
    void destroy() noexcept override { ++destroyed; }
    void delete_this() noexcept override { ++deleted; (void)scalar_delete(1u); }
    ReferenceOwnerControl* scalar_delete(uint32_t flags) noexcept override {
        assert(flags==1u); ++scalar; return this;
    }
    void* null_query(const void*) noexcept override { return nullptr; }
};
static_assert(sizeof(TestControl) == 0x558, "model coallocation extent expected");
}

int main() {
    TestControl storage{};
    assert(storage.null_query(nullptr) == nullptr);
    ModelServiceStorageView view{};
    assert(model_service_storage_view(&storage,&view)==ModelServiceStorageStatus::ok);
    assert(view.owner == static_cast<ReferenceOwnerControl*>(&storage));
    assert(view.service == &storage.service);
    assert(reinterpret_cast<uint8_t*>(view.service)-reinterpret_cast<uint8_t*>(view.owner)==0x10);
    assert(view.storage_size==0x558u);

    ReferenceAliasOwner published{};
    ReferenceAliasOwner local{view.service,view.owner};
    reference_owner_move_publish(&published,&local);
    assert(local.alias==nullptr && local.control==nullptr);
    assert(published.alias==view.service && published.control==view.owner);

    ReferenceAliasOwner lease{};
    assert(reference_owner_copy_acquire(&published,&lease));
    auto snap=reference_owner_snapshot(view.owner);
    assert(snap.strong_count==2 && snap.weak_count==1);
    reference_owner_release_alias(&lease);
    snap=reference_owner_snapshot(view.owner);
    assert(snap.strong_count==1 && snap.weak_count==1);
    assert(destroyed==0 && deleted==0 && scalar==0);

    reference_owner_release_alias(&published);
    snap=reference_owner_snapshot(view.owner);
    assert(snap.strong_count==0 && snap.weak_count==0);
    assert(destroyed==1 && deleted==1 && scalar==1);
    return 0;
}
