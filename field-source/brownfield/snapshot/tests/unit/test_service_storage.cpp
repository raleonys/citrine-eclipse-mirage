#include "preloader_l/runtime/service_storage.hpp"
#include <assert.h>
#include <stdint.h>

using namespace preloader_l::runtime;
namespace {
struct TestControl final : ModelServiceControl558 {
    void destroy() noexcept override {}
    void delete_this() noexcept override { (void)scalar_delete(1u); }
    ReferenceOwnerControl* scalar_delete(uint32_t) noexcept override { return this; }
    void* null_query(const void*) noexcept override { return nullptr; }
};
static_assert(sizeof(TestControl) == 0x558, "test coallocation must be 0x558");
}

int main() {
    TestControl storage{};
    assert(storage.null_query(nullptr) == nullptr);
    ModelServiceStorageView view{};
    assert(model_service_storage_view(&storage, &view) == ModelServiceStorageStatus::ok);
    assert(view.owner == static_cast<ReferenceOwnerControl*>(&storage));
    assert(view.service == &storage.service);
    assert(reinterpret_cast<uint8_t*>(view.service) - reinterpret_cast<uint8_t*>(view.owner) == 0x10u);
    assert(view.storage_size == 0x558u);
    const auto snapshot = reference_owner_snapshot(view.owner);
    assert(snapshot.strong_count == 1);
    assert(snapshot.weak_count == 1);
    assert(model_service_storage_view(nullptr, &view) == ModelServiceStorageStatus::invalid_argument);
    return 0;
}
