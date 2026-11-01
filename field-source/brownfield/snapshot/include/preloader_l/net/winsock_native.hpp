#pragma once
#include "preloader_l/net/socket_runtime.hpp"

namespace preloader_l { namespace net {

// Windows implementation of SocketApi. The v4 pre-.surge baseline binds the
// listener to 0.0.0.0 (INADDR_ANY); caller-selected port policy remains separate.
const SocketApi* native_winsock_api() noexcept;

} }
