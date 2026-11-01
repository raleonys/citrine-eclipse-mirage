#include "preloader_l/runtime/service_storage.hpp"

namespace preloader_l { namespace runtime {

ModelServiceStorageStatus model_service_storage_view(
    ModelServiceControl558* control,
    ModelServiceStorageView* output) noexcept {
    if (!control || !output)
        return ModelServiceStorageStatus::invalid_argument;

    output->owner = control;
    output->service = &control->service;
    output->storage_size = sizeof(ModelServiceControl558);
    return ModelServiceStorageStatus::ok;
}

} }
