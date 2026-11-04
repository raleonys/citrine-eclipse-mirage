// SPDX-License-Identifier: GPL-3.0-only

// Standard compiler CRT probe; no recovered/custom CRT implementation.
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {
std::atomic<unsigned> tls_destroyed{0};
struct ThreadToken { ~ThreadToken() { ++tls_destroyed; } };
thread_local ThreadToken token;
struct UnwindToken { unsigned& count; ~UnwindToken() { ++count; } };
}

int main() {
#if defined(_MSC_VER) && (!defined(_MT) || defined(_DLL))
    std::cerr << "FAIL: expected compiler static CRT /MT or /MTd\n";
    return 10;
#endif
    auto* allocation = static_cast<unsigned char*>(std::malloc(4097));
    if (!allocation) return 11;
    for (unsigned i=0; i<4097; ++i) allocation[i]=static_cast<unsigned char>(i);
    for (unsigned i=0; i<4097; ++i) {
        if (allocation[i]!=static_cast<unsigned char>(i)) { std::free(allocation); return 12; }
    }
    std::free(allocation);
    unsigned unwound=0;
    try { UnwindToken scope{unwound}; throw std::runtime_error("CRT unwind probe"); }
    catch (const std::runtime_error&) {}
    if (unwound!=1) return 13;
    std::atomic<bool> worker_ok{false};
    std::thread worker([&] {
        (void)&token;
        std::vector<std::string> values(129,std::string(1025,'x'));
        worker_ok = values.front().size()==1025 && values.back().size()==1025;
    });
    worker.join();
    if (!worker_ok || tls_destroyed!=1) return 14;
    std::cout << "PASS: compiler CRT heap, exception unwind, containers and thread-local destruction; no game acceptance\n";
}
