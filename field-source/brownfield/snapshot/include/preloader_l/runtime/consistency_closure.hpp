#pragma once
#include <stdint.h>

namespace preloader_l { namespace runtime {

enum class ActiveConsistencyClosureDisposition : uint8_t {
    observable_contract_closed_private_identity_nonrequired = 1,
    current_host_not_applicable = 2,
    outside_normalized_dll_target = 3,
};

// Production consistency-open state is derived by evidence-backed scope,
// not by importing foreign implementation. Historical ActiveConsistencyOpenMask bit
// values remain available for trace compatibility; this snapshot describes why
// none of those debts is a production-open requirement for the normalized DLL.
struct ActiveConsistencyClosureSnapshot {
    uint32_t historical_emitted_mask;
    uint32_t production_open_mask;
    uint16_t closure_entry_count;
    uint16_t observable_contract_closed_count;
    uint16_t current_host_not_applicable_count;
    uint16_t outside_target_count;
};

ActiveConsistencyClosureSnapshot active_consistency_closure_snapshot() noexcept;

} }
