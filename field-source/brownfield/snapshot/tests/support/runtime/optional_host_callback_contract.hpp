#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime { namespace optional_host_callback_evidence {

// Evidence-only partial contract for the optional compatibility host callback.
// This does not enable the signature-hit hook path. It records only behavior
// recovered from the original binary and replayed under a coherent post-0x7000
// ReleaseService state.
constexpr uint32_t kNativeCallbackRva = 0x10150u;
constexpr uint32_t kVmProgramCounter = 0x1019Du;
constexpr uint32_t kPayloadBeginReadVmRva = 0x1D9F29u;
constexpr uint32_t kPayloadEndReadVmRva = 0x270CF8u;
constexpr uint32_t kArgument2DwordWriteVmRva = 0x3D43D1u;

// One-based native argument ordinal. The observed destination is reached from
// the native RDX argument and receives a 32-bit store_payload byte count.
constexpr unsigned kPayloadSizeOutputArgumentOrdinal = 2u;
constexpr unsigned kPayloadSizeOutputWidthBits = 32u;

// ReleaseServiceObject548::store_payload is an MSVC vector-shaped byte owner.
constexpr size_t kStorePayloadOffset = 0x530u;
constexpr size_t kStorePayloadBeginOffset = 0x00u;
constexpr size_t kStorePayloadEndOffset = 0x08u;
constexpr size_t kStorePayloadCapacityOffset = 0x10u;

// Recovery scope: for the tested valid vector ranges, the emitted DWORD is
// exactly (end - begin), independent of capacity. No claim is made here for
// invalid vectors or values outside the tested 32-bit range.

// MATRIX tested-path observability. Bit N corresponds to one-based argument N+1.
// These masks describe only the controlled coherent replay and must not be used
// to infer global semantic types or optionality outside that trace.
constexpr uint8_t kTestedRelocationInvariantArgumentMask = 0x3Fu; // args 1..6
constexpr uint8_t kTestedNullInvariantArgumentMask = 0x3Du;       // args 1,3,4,5,6
constexpr uint8_t kTestedPointeeReadArgumentMask = 0x00u;
constexpr uint8_t kTestedPointeeWriteArgumentMask = 0x02u;       // arg2 only
constexpr uint32_t kTestedVmExitFuncletRva = 0x104E4u;

// Replay-visible non-argument writes are lifetime/synchronization bookkeeping,
// not additional curio application-level callback outputs.
constexpr uint32_t kControlBlockStrongRefIncrementVmRva = 0x49F12Cu;
constexpr uint32_t kMutexRecursionCountWriteRva = 0x474E7u;
constexpr uint32_t kMutexOwnerThreadWriteRva = 0x4751Fu;

// BRANCH tested-path dispatch evidence. The selector word at VIP+0x06 is
// 0x013D on the shared VIP 0x4437B7, selecting a DWORD in VM context.
// The curio handler masks bit 0x40 before choosing the tested route.
// These constants are evidence descriptors only; they do not assign a
// semantic type/name to the context field and do not model normal return.
constexpr uint32_t kTestedPostSyncCommonVipRva = 0x4437B7u;
constexpr uint16_t kTestedDispatchContextOffset = 0x013Du;
constexpr uint32_t kTestedDispatchContextMask = 0x40u;
constexpr uint32_t kTestedZeroReturnContextValue = 0x246u;
constexpr uint32_t kTestedOneReturnContextValue = 0x202u;
constexpr uint32_t kTestedImmediateFailureExitRva = 0x104D9u;
constexpr uint32_t kTestedDeferredFailureExitRva = 0x104E4u;
constexpr uint32_t kTestedDispatchMaskInstructionRva = 0x32C2B2u;
constexpr uint32_t kTestedDispatchBranchInstructionRva = 0x32C2B8u;

constexpr uint32_t curio_payload_size_for_tested_valid_range(
    uintptr_t begin, uintptr_t end) noexcept {
    return static_cast<uint32_t>(end - begin);
}

} } }
