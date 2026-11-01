#pragma once
// Windows native-acceptance-only injection. This interface deliberately avoids
// private Winsock/Win32 ABI declarations: Asio includes the real Windows SDK.
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
namespace preloader_l { namespace runtime {
// Acquires a single session-owned (not thread-owned) artificial delivery stall.
// Other Asio sessions must continue through the normal async write path.
bool release_acceptance_should_hold_asio_send_windows(const void* session) noexcept;
void release_acceptance_note_send_idle_timeout_windows() noexcept;
} }
#endif
