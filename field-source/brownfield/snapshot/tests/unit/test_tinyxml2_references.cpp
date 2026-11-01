#include "../../third_party/tinyxml2/tinyxml2.h"
#include <cassert>
#include <cstring>

int main() {
    using tinyxml2::XMLUtil;
    char out[8]{};
    int length = -1;

    const char valid_dec[] = "&#65;";
    const char* next = XMLUtil::GetCharacterRef(valid_dec, out, &length);
    assert(next == valid_dec + 5);
    assert(length == 1 && out[0] == 'A');

    std::memset(out, 0, sizeof(out));
    const char valid_hex[] = "&#x1F600;";
    next = XMLUtil::GetCharacterRef(valid_hex, out, &length);
    assert(next == valid_hex + sizeof(valid_hex) - 1u);
    assert(length == 4);

    const char leading_zero[] = "&#00000000065;";
    next = XMLUtil::GetCharacterRef(leading_zero, out, &length);
    assert(next == leading_zero + sizeof(leading_zero) - 1u);
    assert(length == 1 && out[0] == 'A');

    // F14 / CVE-2024-50615 class: oversized character references must fail
    // without relying on debug assertions or overflowing multiplier arithmetic.
    const char too_large_hex[] = "&#x110000;";
    length = -1;
    assert(XMLUtil::GetCharacterRef(too_large_hex, out, &length) == nullptr);
    assert(length == 0);

    const char pathological_dec[] = "&#999999999999999999999999999999999999999999999999999999;";
    length = -1;
    assert(XMLUtil::GetCharacterRef(pathological_dec, out, &length) == nullptr);
    assert(length == 0);

    const char pathological_hex[] = "&#xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF;";
    length = -1;
    assert(XMLUtil::GetCharacterRef(pathological_hex, out, &length) == nullptr);
    assert(length == 0);
    return 0;
}
