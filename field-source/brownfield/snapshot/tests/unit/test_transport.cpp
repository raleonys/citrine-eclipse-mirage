#include "preloader_l/net/socket_transport.hpp"

using namespace preloader_l::net;
namespace {
struct Fixture { unsigned closes; };
TransportStatus rx(void*, uint8_t*, size_t, size_t* n) noexcept { if (n) *n = 0; return TransportStatus::would_block; }
TransportStatus tx(void*, const uint8_t*, size_t n, size_t* sent) noexcept { if (sent) *sent = n; return TransportStatus::ok; }
void close(void* p) noexcept { if (p) ++static_cast<Fixture*>(p)->closes; }
TransportStatus accept(void*, TransportEndpoint* out) noexcept { if (!out) return TransportStatus::invalid_argument; *out = TransportEndpoint{}; return TransportStatus::would_block; }
}
int main() {
    Fixture f{};
    TransportEndpoint ep{&f, rx, tx, close};
    if (!valid_endpoint(ep)) return 1;
    TransportEndpoint bad{}; if (valid_endpoint(bad)) return 2;
    close_endpoint(&ep); if (f.closes != 1 || valid_endpoint(ep)) return 3;
    close_endpoint(&ep); if (f.closes != 1) return 4;
    TransportListener listener{&f, 3226, accept, close};
    if (!valid_listener(listener)) return 5;
    close_listener(&listener); if (f.closes != 2 || valid_listener(listener)) return 6;
    TransportListener bad_listener{&f, 0, accept, close}; if (valid_listener(bad_listener)) return 7;
    return 0;
}
