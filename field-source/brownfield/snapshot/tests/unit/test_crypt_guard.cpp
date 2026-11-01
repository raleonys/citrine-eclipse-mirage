#include "preloader_l/platform/crypt_guard.hpp"

int main()
{
    using preloader_l::platform::should_fail_closed_crypt_msg;
    if (!should_fail_closed_crypt_msg(nullptr, 6u, true)) return 1;
    if (should_fail_closed_crypt_msg(reinterpret_cast<void*>(1), 6u, true)) return 2;
    if (should_fail_closed_crypt_msg(nullptr, 5u, true)) return 3;
    if (should_fail_closed_crypt_msg(nullptr, 6u, false)) return 4;
    if (preloader_l::platform::crypt_msg_null_guard_error != 6u) return 5;
    return 0;
}
