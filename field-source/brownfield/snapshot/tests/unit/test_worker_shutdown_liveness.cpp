#include "preloader_l/runtime/release_server_io.hpp"
#include <cassert>
#include <cstdint>

namespace {
struct NativeFdSetMirror {
    unsigned int fd_count;
    uintptr_t fd_array[64];
};
struct NativeTimevalMirror {
    long tv_sec;
    long tv_usec;
};

int g_ioctl_result = 0;
uintptr_t g_ioctl_socket = 0;
long g_ioctl_command = 0;
uint32_t g_ioctl_value = 0;

int g_select_result = 0;
uintptr_t g_select_socket = 0;
bool g_select_writing = false;
long g_select_sec = -1;
long g_select_usec = -1;
}

extern "C" int ioctlsocket(uintptr_t socket, long command, uint32_t* value) {
    g_ioctl_socket = socket;
    g_ioctl_command = command;
    g_ioctl_value = value ? *value : 0u;
    return g_ioctl_result;
}

extern "C" int select(int, void* read_set, void* write_set, void*, const void* timeout) {
    auto* set = static_cast<NativeFdSetMirror*>(write_set ? write_set : read_set);
    assert(set != nullptr);
    assert(set->fd_count == 1u);
    g_select_socket = set->fd_array[0];
    g_select_writing = write_set != nullptr;
    auto* tv = static_cast<const NativeTimevalMirror*>(timeout);
    assert(tv != nullptr);
    g_select_sec = tv->tv_sec;
    g_select_usec = tv->tv_usec;
    return g_select_result;
}

int main() {
    using namespace preloader_l::runtime;

    g_ioctl_result = 0;
    assert(configure_release_socket_nonblocking_windows(0x1234u));
    assert(g_ioctl_socket == 0x1234u);
    assert(static_cast<uint32_t>(g_ioctl_command) == 0x8004667eu);
    assert(g_ioctl_value == 1u);

    g_ioctl_result = -1;
    assert(!configure_release_socket_nonblocking_windows(0x1234u));
    assert(!configure_release_socket_nonblocking_windows(~uintptr_t{0}));

    g_select_result = 1;
    assert(wait_release_socket_windows(0x4321u, false) == ReleaseSocketWaitStatus::ready);
    assert(g_select_socket == 0x4321u && !g_select_writing);
    assert(g_select_sec == 0 && g_select_usec == 100000);

    g_select_result = 0;
    assert(wait_release_socket_windows(0x4321u, true) == ReleaseSocketWaitStatus::timeout);
    assert(g_select_socket == 0x4321u && g_select_writing);
    assert(g_select_sec == 0 && g_select_usec == 100000);

    g_select_result = -1;
    assert(wait_release_socket_windows(0x4321u, false) == ReleaseSocketWaitStatus::error);
    assert(wait_release_socket_windows(~uintptr_t{0}, false) == ReleaseSocketWaitStatus::error);
    return 0;
}
