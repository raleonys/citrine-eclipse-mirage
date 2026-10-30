# Unload guard retained ownership and retry

The continuation fixes the confirmed ResumeThread ownership defect shared by the
previous application and its selected Bluefield unload-guard donor. It is separate
from the already adopted MinHook SAFE-FREEZE patch.

The original scanner suspended a peer, inspected its context/stack, called resume
and then closed its handle even when resume failed. A later attempt reopened the
thread and added another suspension. Removing only that new suspension could not
repay the earlier one. A Windows-API effect model compiled with the actual source
reproduced that failure before this correction.

## Recovery contract

- The scanner stores one current thread handle, whether it owns a suspension,
  and the current snapshot handle in zero-allocation image-owned state.
- Failed resume of a live peer retains both physical handles. No new snapshot
  or suspension is acquired until that exact pending cleanup succeeds.
- Successful resume clears only this scanner's suspension ownership. A failed
  thread close retains the handle and retries close only, without another resume.
  Other owners' suspension counts are preserved.
- A peer proven terminated can be released even if resume is unavailable. An
  invalid-handle result releases an already invalid handle claim; other failures
  remain retained. An unproven live peer is never called safe on resume failure.
- Snapshot-close failure also retains its handle and prevents replacement scans
  until physical release succeeds.
- An atomic try-gate rejects overlapping/reentrant scans without waiting. The
  owner state is accessed only by its holder. No thread can wait for this gate
  while suspended by its holder.

The existing register checks, conservative 64MiB stack-scan bound, SEH read guard,
caller/self exclusions and Toolhelp enumeration remain. A false return still
defers engine destruction and image unload. Existing Application/Hooks cleanup
retries the scanner while retaining callback/trampoline/image owners. Persistent
OS failures can keep shutdown pending; resources are retained rather than
prematurely released. The three production exports and numeric shutdown statuses
are unchanged.

The source relationship to Bluefield is now **adapted scan plus retained cleanup**,
not token-identical scanner code. No other Bluefield application behavior is
imported. Port selection remains the owner-selected 3226–3236 range, and the
MinHook build-copy patch and all four original vendor trees are unchanged.

## Verification

`unload_guard_contracts` includes the actual production translation unit and
substitutes Windows API boundary effects. It is compiled against Windows-target
headers (MinGW-w64 in the cross-build), with the same SEH source enabled. The original source fails the
new retained-handle assertion with exit1; the corrected source passes.

The cases cover persistent resume failure, old-suspension cleanup before a fresh
scan of the same live peer, foreign suspend-count preservation, close-only retry,
snapshot-close retry, failed suspend/context/enumeration, proven termination,
module instruction/return addresses and reentrant rejection. No real thread is
frozen by fault injection. This is not a claim of native Windows kernel failure
reproduction. Separate application/adapter/public lifecycle tests exercise real
Windows API paths through Wine; native MSVC/game acceptance remains separate.

Current evidence: [UNLOAD_RECOVERY.json](../metadata/evidence/UNLOAD_RECOVERY.json).
