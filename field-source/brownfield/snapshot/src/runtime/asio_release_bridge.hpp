#pragma once
// HF13: Windows-native ReleaseServer / AsioServiceOwner binding.
// Does not alter public exports, 0x548 ABI, or injectable SocketApi tests.
#include <cstdint>
namespace preloader_l { namespace runtime {
struct ReleaseServerContext;
bool start_release_asio_windows(ReleaseServerContext*, uint16_t*) noexcept;
bool drive_release_asio_windows(ReleaseServerContext*) noexcept;
bool stop_release_asio_windows(ReleaseServerContext*) noexcept;
bool shutdown_release_asio_windows(ReleaseServerContext*) noexcept;
} }
