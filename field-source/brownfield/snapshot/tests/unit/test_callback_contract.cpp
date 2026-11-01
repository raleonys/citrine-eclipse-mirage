#include "runtime/optional_host_callback_contract.hpp"
#include "preloader_l/runtime/service_abi.hpp"
#include <cassert>
#include <cstddef>
using namespace preloader_l::runtime;
int main() {
    using namespace optional_host_callback_evidence;
    using model_abi::ReleaseServiceObject548;
    static_assert(kNativeCallbackRva == 0x10150u, "callback RVA drift");
    static_assert(kVmProgramCounter == 0x1019Du, "VM PC drift");
    static_assert(kPayloadSizeOutputArgumentOrdinal == 2u, "arg ordinal drift");
    static_assert(kPayloadSizeOutputWidthBits == 32u, "write width drift");
    static_assert(kStorePayloadOffset == offsetof(ReleaseServiceObject548, store_payload), "service ABI drift");
    static_assert(kStorePayloadBeginOffset == 0u && kStorePayloadEndOffset == 8u && kStorePayloadCapacityOffset == 16u, "vector ABI drift");
    assert(curio_payload_size_for_tested_valid_range(0x520000000ull,0x520000000ull)==0u);
    assert(curio_payload_size_for_tested_valid_range(0x520000000ull,0x520000001ull)==1u);
    assert(curio_payload_size_for_tested_valid_range(0x520000000ull,0x520000040ull)==64u);
    assert(curio_payload_size_for_tested_valid_range(0x520001230ull,0x520001270ull)==64u);
    assert(curio_payload_size_for_tested_valid_range(0x520000000ull,0x520000100ull)==256u);
    return 0;
}
