#pragma once
namespace preloader_l { namespace platform {
#if defined(_WIN32)
// HF18: Stage 0/2/4 addon API detours inside the same core transaction.
// mask bit0 selects CreateProcessA/W, bit1 selects SHGetFolderPathA/W.
bool stage_addon_hooks(unsigned mask) noexcept;
// Must be called only after the shared transaction was proven restored.
bool uninstall_addon_hooks() noexcept;
#endif
} }
