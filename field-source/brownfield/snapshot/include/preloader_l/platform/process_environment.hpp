#pragma once
#include <stdint.h>
#include "preloader_l/platform/environment_policy.hpp"
namespace preloader_l { namespace platform {
#if defined(_WIN32)
EnvironmentStatus apply_process_environment(uint32_t* written_mask,
                                            uint32_t* preserved_mask) noexcept;
EnvironmentStatus restore_process_environment(uint32_t written_mask,
                                              uint32_t* restored_mask) noexcept;
#endif
} }
