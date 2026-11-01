#include "preloader_l/runtime/service_abi.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>

int main() {
    using namespace preloader_l::runtime::model_abi;
    assert(sizeof(ReleaseServiceObject548) == 0x548);
    assert(sizeof(ReleaseServiceAllocation558) == 0x558);
    assert(offsetof(ReleaseServiceObject548, content) == 0x468);
    assert(offsetof(ReleaseServiceObject548, store_payload) == 0x530);
    assert(sizeof(ModelSettingsNode50) == 0x50);
    assert(offsetof(ModelSettingsNode50, key) == 0x10);
    assert(offsetof(ModelSettingsNode50, value) == 0x30);
    assert(sizeof(ModelSettingsUnorderedMap40) == 0x40);
    assert(offsetof(ModelSettingsUnorderedMap40, sentinel) == 0x08);
    assert(offsetof(ModelSettingsUnorderedMap40, size) == 0x10);
    assert(offsetof(ModelSettingsUnorderedMap40, bucket_pairs) == 0x18);
    assert(offsetof(ModelSettingsUnorderedMap40, mask) == 0x30);
    assert(offsetof(ModelSettingsUnorderedMap40, bucket_count) == 0x38);
    assert(kModelServiceConstructorRva == 0xF020u);
    assert(kModelServiceRunRva == 0x37750u);
    std::puts("MERLORYX release-service semantic ABI: PASS");
    return 0;
}
