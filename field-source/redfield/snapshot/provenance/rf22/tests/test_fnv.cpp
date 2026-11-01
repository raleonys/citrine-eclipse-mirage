// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/hash/fnv_hash.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {
struct Vector64 {
    const char* text;
    std::uint64_t expected;
};

bool check64(const Vector64& vector) noexcept
{
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(vector.text);
    return redfield::hash::fnv1a64_signed_bytes(bytes, std::strlen(vector.text)) == vector.expected;
}
}

int main()
{
    // Exact outputs recorded by the bounded original-instruction differential
    // harness in evidence/differential_validation.json.
    constexpr Vector64 vectors[] = {
        {"", UINT64_C(0xcbf29ce484222325)},
        {"GetProfile", UINT64_C(0x27470f1cda405048)},
        {"QueryEntitlements", UINT64_C(0x0a1fca6299fc2113)},
        {"GetAuthCode", UINT64_C(0xf7c634d45f94bb28)},
        {"abc", UINT64_C(0xe71fa2190541574b)},
        {"GetSetting", UINT64_C(0xa925a66a5ac37a7d)},
        {"ShowIGO", UINT64_C(0x2ae153ff0a17a875)},
        {"ShowIGOWindow", UINT64_C(0xc96101f996a3e86d)},
        {"IsProgressiveInstallationAvailable", UINT64_C(0x00f6d5a7471c7cff)},
        {"GetAllGameInfo", UINT64_C(0x05fd32103f8332bc)},
        {"QueryOffers", UINT64_C(0x28415d10fa3aadac)},
        {"GetSettings", UINT64_C(0x2e73d4b83a2865ca)},
        {"GetConfig", UINT64_C(0x5b9d8ac023fbe855)},
        {"StartDownload", UINT64_C(0x854a033ec9f89e8b)},
        {"GetPresence", UINT64_C(0x933b9f371216d496)},
        {"GetGameInfo", UINT64_C(0xe03fcf0117d2c54d)},
        {"GrantAchievement", UINT64_C(0xae37f1ac30db5870)},
        {"CheckPermission", UINT64_C(0xd3dfa32adc5ffd78)},
        {"QueryAchievements", UINT64_C(0xd424e7c76bbcc117)},
        {"GetInternetConnectedState", UINT64_C(0xdd8c6a90fbcfeb52)},
        {"QueryFriends", UINT64_C(0xdecf1968d3efd1fc)},
        {"SetPresence", UINT64_C(0xe0558472a1297182)},
        {"QueryImage", UINT64_C(0xead55cb841534af8)},
        {"QueryContent", UINT64_C(0xfd77504ac92dc304)},
    };

    for (const auto& vector : vectors) {
        if (!check64(vector)) return 1;
    }

    const std::uint8_t high64[] = {0x80, 0xff, 0x7f};
    if (redfield::hash::fnv1a64_signed_bytes(high64, sizeof high64) != UINT64_C(0xad095d1974fb962d)) return 2;

    const std::uint8_t embedded_zero[] = {'A', 0x00, 'B'};
    if (redfield::hash::fnv1a64_signed_bytes(embedded_zero, sizeof embedded_zero) != UINT64_C(0xfb029919a1274bf2)) return 3;

    constexpr std::uint32_t seed = UINT32_C(0x93041074);
    struct Vector32 { const char* text; std::uint32_t expected; };
    constexpr Vector32 vectors32[] = {
        {"GetModuleFileNameW", UINT32_C(0xdd4bc12e)},
        {"GetProcAddress", UINT32_C(0xc57443f8)},
        {"VirtualProtect", UINT32_C(0x512ee726)},
    };
    for (const auto& vector : vectors32) {
        const auto* bytes = reinterpret_cast<const std::uint8_t*>(vector.text);
        if (redfield::hash::fnv1a32_signed_bytes(bytes, std::strlen(vector.text), seed) != vector.expected) return 4;
    }

    const std::uint8_t high32[] = {0x80, 0xff, 'A'};
    if (redfield::hash::fnv1a32_signed_bytes(high32, sizeof high32, seed) != UINT32_C(0xd52e36c8)) return 5;

    return 0;
}
