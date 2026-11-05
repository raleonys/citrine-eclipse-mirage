# Stable IO owner prerequisite checkpoint — source only

2026-11-05. Twenty-two source/header units changed: eighteen production, four test; one production file is a new neutral dependency header. All before bytes (or explicit absent-before marker) were captured against the86-pending-file checkpoint. No compilation unit was added, no compiler/project tests/link/DLL execution/ZIP operation occurred. Historical build/link/package proofs remain immutable.

## Result and scope

OwnedIoMemory derives its immutable mode from the same Parent policy. In Concurrent mode each stable shared Record owns its descriptor/storage binding and dispatcher packet; index growth does not patch external packet fields. Exact typed storage/OVERLAPPED/CancelOperation resolvers return live owner descriptors. Legacy borrowed union span APIs reject in Concurrent mode. Serialized retains authoritative mutable legacy tables, external field growth updates and old tamper negatives; default actual production still selects Serialized. The new exact APIs are a lower ownership prerequisite, not a completed NativeJobs/Graph concurrency fix.

Brief IO metadata transactions reserve actual per-actor effects and exact per-row lifecycle. Compiler metadata allocation/destruction, Parent heap allocation/free, SDK buffer/control pin operations and source callbacks run outside that gate. Stable shared Record capture retains host metadata, without deferring native storage free. A callback Call token belongs to Record AND generation: source cleanup can free/cache/reproject the old envelope and B can reuse its slot/new generation while A's moved functor remains held. The Call destructor removes only its own token/count, with no postcallback old projection/slot read. No operation-wide Parent byte pin was added to an IO envelope.

OwnedAccept now overlays/resolves the mode-authoritative packet rather than indexing/scanning a movable base table. Its exact storage-match helper keeps the existing Parent/accept admission and checks exact IO resolver identity for Concurrent; Serialized continues span identity/size and rejects a foreign new resolver. Runtime constructs and copies the appropriate resolver-backed Posted/Dispatcher contexts, verifies selected owner identity and rechecks observed completion generation/packet before coherent dispatch. HOST Router retains its original indexed first-match behavior. Profile, Policy, Environment, Native/InlineJobs and CoherentIO use actual owner admission instead of requiring a forbidden Concurrent catalog span.

Runtime and Stop observe CURRENT registry capacity, not just constructor capacity. Application::create may grow the registry before Stop::bind, so replacing this bound by initial_capacity would weaken the previous coverage requirement. The new metadata-only capacity observation and admitted Accept wrapper preserve that actual current bound without borrowing a union span. Existing port/options/receipt ownership is unchanged.

## Known versus unknown completion prefixes

Independent reviews found and corrected three concrete issues before recording:

1. Moving a pending range pin/clearing ownership before validating the nonlast strong decrement could drop it during failed guard unwinding. Now pending/owner/pin/public packet remain retained until successful outside-gate CAS; deliberate consumed-strong input poisons and prevents the native callback without decrement or cleanup.
2. Putting all observation inside an unknown-outcome catch inadvertently poisoned a known duplicate cached completion. All three complete wrappers keep completed/unknown eligibility preflight outside Call/outcome catch, preserving existing duplicate rejection followed by normal cache drain.
3. Same-generation Call ownership does not stop an unrelated row from poisoning the global IO state while A is outside the gate for CAS (or the Posted null-control window). Observation rechecks metadata phase immediately after relock, before pin/routing/completed mutation or callback. A post-CAS unknown retains the real consumed prefix under sticky poison; no replay/rollback is invented.

The final independently reviewed IO hash is812044f220b13cbb32d4d4e821879d80181801b6f1b10066b5a4f3e6ec3b763b. Earlier candidate reports are preserved with their own hashes and are not runtime validation or final acceptance of those superseded bytes.

## Source fixtures and native evidence

The authored lower fixtures use real existing NativeThreads/PreparedMemory/JobScope, one shared Concurrent Parent/IO root and two actual child actors. A is held first in the actual moved posted functor after envelope free on the selected no-cache lower; separately A is held in its physical native allocation effect. B must grow70disjoint rows, resolve the independent live guard, complete/free, reject stale generations, join/FLS-exit and be reaped before A is released. Pointer stability, external alias, packet contents and natural final drain are asserted. These Posted cases do not run actual Runtime/Graph/kernel receive or establish concurrent receive/send loan coverage; existing HOST loan cases are separately preserved.

The isolated consumed-strong sourcechild uses a real armed PinnedReceive then deliberate invalid count1, requiring retained poisoned pending/range/packet state and no callback. Existing unknown-SDK child and duplicate cached-send assertions remain. Two socket tests exercise actual cache021820/021900 and dispatcher027c10 ownership/miss/ambiguity/legacy first-match behavior against deliberately stale tables. ALL assertions are authored and NOT RUN.

Eight fresh REA6.8/Ghidra12.1.4 instruction observations support kernel operation identity/callback and posted cleanup-before-functor ordering. Full replies/EIDs, exported bundle, retained full-body hashes and original-after-close hash are in native-findings/native-evidence-index. Cleanup may cache-transfer or physically free; unconditional native free is not claimed. No new recovered-function credit, native runtime schedule or ABI/fault equivalence follows.

## New header and bookkeeping

memory_lookup.hpp is the neutral shared enum dependency required to avoid a completion_dispatcher/write_finish include cycle. Existing registered .cpp units include it transitively; no build-unit change is needed. The existing fresh-run header checkpoint recursively inventories .hpp/.h files. Future consistent rebuild/source inventory must include this addition; no historical source snapshot is rewritten to imply that it existed in the last build.

A new reviewed metadata-only recorder v3 tracks added sources against explicit absent-before proof, with build_sha256:null and added_since_build:true, preserves unlisted pending source hashes and all frozen artifacts, and hashes the baseline manifest into evidence. v2 and original recorder bytes are unchanged. This is bookkeeping, not compiler/test validation. Subsequent stages must use v3 until the new header enters an actually authorized fresh build snapshot.

## Remaining mandatory integration

Actual Original concurrency is still disabled. Strings authoritative header lookup/pruning and Deques per-call growth contexts, per-message Profile/NativeProfile/Graph/provider ownership and NativeJobs whole-body gate removal/actual policy selection remain mandatory. owned_response.cpp's destroy path also still scans FrameReadContext.allocation_headers directly and must migrate with those header consumers.

OwnedAccept still has its existing accept-body/SDK gate; stable pointer conversion is not proof that every accept-body invocation can overlap. Mutable packet metadata/publication and native receive/send/accept operation ownership remain subject to their actual source actor, generation, kernel/SDK and caller synchronization. Exact resolver pointer stability alone does not make external packet payload writes or arbitrary same-operation concurrent writes safe. Audit those concrete writers/callers during actual upper assembly; do not interpret this stage as universal concurrent IO safety or silently enable partial composition.

The final integrated source cases must exercise both warmed plaintext same-session and distinct-session A-UnknownMessage held modal/B actual GetConfig response-before-A-release, through the real NativeJobs/Graph/IO route. No per-session strand, cloned application/Online roots, all-ready/SDK/unload barrier or synthetic work/readiness substitute closes that native gap. Execution still awaits explicit rebuild authorization. No total DLL completion percentage is inferred from these source prerequisites.
