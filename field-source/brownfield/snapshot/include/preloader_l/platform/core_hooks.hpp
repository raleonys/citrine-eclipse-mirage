#pragma once
namespace preloader_l { namespace platform {
// Stage the seven production core detours in the current MinHook transaction.
// Does not commit or publish a second transaction.
bool stage_core_hooks() noexcept;
} }
