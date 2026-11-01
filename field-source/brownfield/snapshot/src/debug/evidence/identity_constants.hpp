#pragma once

namespace preloader_l { namespace platform {

// Identity strings recovered from the compact reconstructed oracle. These are
// diagnostic evidence only. They must not be used to redirect trust or
// certificate verification to a different file.
inline constexpr char reconstructed_identity_ascii[] = "EA Javelin Anticheat";
inline constexpr char reconstructed_alt_module_ascii[] = "EAAntiCheat.GameServiceLauncher.dll";
inline constexpr wchar_t reconstructed_alt_module_name[] = L"EAAntiCheat.GameServiceLauncher.dll";

} }
