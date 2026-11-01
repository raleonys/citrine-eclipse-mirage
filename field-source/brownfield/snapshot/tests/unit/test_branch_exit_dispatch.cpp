#include "runtime/optional_host_callback_contract.hpp"
int main() {
  using namespace preloader_l::runtime::optional_host_callback_evidence;
  static_assert(kTestedPostSyncCommonVipRva == 0x4437B7u, "common VIP drift");
  static_assert(kTestedDispatchContextOffset == 0x013Du, "selector drift");
  static_assert(kTestedDispatchContextMask == 0x40u, "mask drift");
  static_assert((kTestedZeroReturnContextValue & kTestedDispatchContextMask) != 0u, "zero tested branch bit");
  static_assert((kTestedOneReturnContextValue & kTestedDispatchContextMask) == 0u, "one tested branch bit");
  static_assert(kTestedImmediateFailureExitRva == 0x104D9u, "immediate exit drift");
  static_assert(kTestedDeferredFailureExitRva == 0x104E4u, "deferred exit drift");
  static_assert(kTestedDispatchMaskInstructionRva == 0x32C2B2u, "mask instruction drift");
  static_assert(kTestedDispatchBranchInstructionRva == 0x32C2B8u, "branch instruction drift");
  return 0;
}
