#include "runtime/optional_host_callback_contract.hpp"
#include <cassert>
int main() {
  using namespace preloader_l::runtime::optional_host_callback_evidence;
  static_assert(kTestedRelocationInvariantArgumentMask == 0x3Fu, "all six relocation cases");
  static_assert(kTestedNullInvariantArgumentMask == 0x3Du, "arg1/3/4/5/6 null cases");
  static_assert(kTestedPointeeReadArgumentMask == 0x00u, "no tested pointee reads");
  static_assert(kTestedPointeeWriteArgumentMask == 0x02u, "arg2 only");
  static_assert(kTestedVmExitFuncletRva == 0x104E4u, "tested trace exit drift");
  static_assert(kControlBlockStrongRefIncrementVmRva == 0x49F12Cu, "refcount anchor drift");
  static_assert(kMutexRecursionCountWriteRva == 0x474E7u, "mutex count anchor drift");
  static_assert(kMutexOwnerThreadWriteRva == 0x4751Fu, "mutex owner anchor drift");
  assert(curio_payload_size_for_tested_valid_range(0x1000u, 0x1040u) == 0x40u);
  return 0;
}
