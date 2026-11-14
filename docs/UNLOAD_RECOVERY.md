> The selected engine is now vanilla MithrilHook 1.0.4. Its strict completion and
> retirement checks complement the application ownership protocol below; see
> [current engine integration](MITHRILHOOK_INTEGRATION.md). References to the old
> patched MinHook tree describe the pre-migration checkpoint, not the active build.

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

## Compiler TLS endpoint data

Windows MSVC Debug lifecycle tracing found an idle peer with RIP in ntdll and
R11 equal to the image TLS template endpoint (`_tls_end`), rather than a code
continuation. The linker map binds that address to `.tls$ZZZ`. The scanner now
recognizes only the exact EndAddressOfRawData declared by a bounded PE TLS
directory, inside a non-executable section and a committed non-executable image
page. That marker in a data register or stack slot does not establish execution
in the DLL. RIP inside the image still rejects unload, including at that marker.
All other image pointers, unreadable stacks, scan failures, retained OS owners
and retry rules remain conservative. Invalid or missing TLS metadata grants no
exemption. The native effect tests cover the marker, adjacent data/code values,
RIP, executable protections and malformed TLS ranges.

## Debug refusal diagnostics

Debug builds publish the first conservative unload-scan refusal into one private,
fixed 96-byte image record. It contains a version, sequence, reason, peer ID,
register or stack-slot index, image-relative reference offset, validated TLS
endpoint offset, Windows error and retained-suspension flag. R11 is register
index 10; stack slots count from the suspended peer's RSP. No absolute image
address or machine path is logged. A refusal record diagnoses an observation;
it does not establish that a pointer is harmless or justify a new exemption.

The scan try-gate serializes writers. Publishing uses only lock-free atomic
stores; it does not format text, allocate, call an output API, start a thread or
invoke a callback. The record is committed before releasing the gate, including
when failed resume retains ownership. Successful scans replace stale refusals.
Release builds compile out the record and all diagnostic operations.

The existing lifecycle host locates the versioned record in bounded writable,
non-executable sections of the loaded Debug DLL. It adds no public export or PE
section. Lookup rejects malformed, unreadable, over-budget or ambiguous images.
After a failed DllShutdown call returns, the host makes at most three snapshot
attempts using ReadProcessMemory and matching even sequence values. Moving,
malformed or unreadable snapshots produce no report. A retained scanner-owned
suspension suppresses output. A usable refusal prints UNLOAD_SCAN_BLOCKED;
LastError is preserved and output failure cannot change DLL scan decisions or
release retained owners. Host output is best effort and can still be delayed by
its output consumer; no scanner gate or DLL callback is held for that output.

The manual-only shutdown listener workflow defaults to five Visual Studio
attempts per Debug/Release configuration. A dispatch can select NMake with the
Native profile's testing/source-API configuration and twenty independent
attempts. Every attempt must pass; a failure is counted, retained and fails the
job even if later attempts succeed. Generator, exact checkout, attempt count and
individual exits are recorded alongside per-attempt JUnit artifacts. The workflow
does not add PR triggers or replace the required synchronization validation.

The actual-source unload fixture retains its existing scan, TLS, stack budget,
foreign suspension and recovery assertions. Debug cases additionally check
publication fields, successful-retry replacement, retained-resume suppression,
bounded moving/busy readers, malformed/ambiguous image rejection and output
failure. Compile-time tripwires reject scanner calls to WriteFile/GetStdHandle.
The existing real-DLL lifecycle host requires successful Debug record discovery;
its real Windows execution remains separate from cross-compilation and controlled
failure models. Repeated successful runs describe only the executed attempts; they do not prove
that the historical intermittent status-4 refusal cannot recur.
