#include "preloader_l/protocol/socket_exchange.hpp"
#include <stddef.h>

int main()
{
    preloader_l::protocol::ExchangeState state{};
    if (sizeof(state.decoded) != 2048u) return 1;
    if (sizeof(state.plain_response) != 4096u) return 2;
    if (sizeof(state.wire_response) != 4096u) return 3;
    // Keep enough slack for implementation-defined bool/layout details while
    // preventing the retired 8 KiB ingress accumulator from returning.
    if (sizeof(state) > 10300u) return 4;
    return 0;
}
