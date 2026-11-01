#pragma once
namespace preloader_l { namespace platform {
struct AddonHookSelection {
    unsigned mask; // bit 0: CreateProcessA/W, bit 1: SHGetFolderPathA/W
};
// Called only after the authoritative Config2 model is committed and frozen.
// Missing surge.cfg selects mask 0, not an installation failure.
bool read_committed_addon_selection(AddonHookSelection* out) noexcept;
} }
