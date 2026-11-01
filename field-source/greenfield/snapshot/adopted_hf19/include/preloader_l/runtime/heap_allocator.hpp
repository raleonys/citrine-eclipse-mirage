#pragma once
#include "preloader_l/runtime/memory_allocator.hpp"

namespace preloader_l { namespace runtime {

using RuntimeNewHandler = int (*)(size_t requested_size, void* context);

// Curio process-wide new-handler semantic model. This remains outside
// the normalized shipping target because the proven 18 roots belong to the
// private model hook-object lifecycle; the clean hook backend uses its own
// non-throwing allocator/status path.
RuntimeNewHandler set_runtime_new_handler(RuntimeNewHandler handler,
                                          void* handler_context,
                                          void** previous_context = nullptr) noexcept;
const AllocatorPlatformOps& windows_allocator_platform_ops() noexcept;
// Final-unload only; same-module runtime restart after DllShutdown is not a
// supported lifecycle. Returns false only if the handler lock is still in an
// unexpected transition state.
bool shutdown_allocator_platform_lock_windows() noexcept;

// Optional acceptance/oracle bridge between the curio heap/new-handler
// substrate and the native C++ terminal surface. It is intentionally excluded
// from the normalized shipping target unless explicitly enabled.
#if defined(PRELOADER_L_ENABLE_ALLOCATOR_THROW_BRIDGE)
void* allocate_block_or_throw_windows(size_t size,
                                      AllocationRetryPolicy policy);
#endif

} }
