# Information per-call views — source implementation

Only Information effects.hpp/.cpp/effects_tests.cpp were authored. No compiler, project test, linker, executable, DLL, ZIP or REA operation ran. Existing production Graph/Profile selection remains unchanged.

| Source | Captured before SHA256 | Final SHA256 |
| --- | --- | --- |
| effects.hpp | `78f57e125b0f648ace5a42a875a91b6ceb8f792f528d7b67858d48f315b0b739` | `3a77b6470f2e0f0751498a14559c1274f6ea8b469cd8eda60dba53ed8298186b` |
| effects.cpp | `11ae43c5d32eb83d9cd1768fef6d789d7db69ea200a5392bdf731905ab7a0e51` | `a01949674e6f3ac384c31fc534f1e1ed1ff9abc32b824d7423ccaf6715fbdb39` |
| effects_tests.cpp | `c069657527bf25724eae0e6d93958ea8f247c5cb419c97b17b35a4d88d5ca5e5` | `78eea811a3067953f6fac0ed80c6e97c263163b637e63d43752e13f162ee6bc4` |

Root captured the before files under this stage before GO. Root owns the new shared application_io::CallContext header; the Content and actual PreparedMemory fixture authors are separate.

## Public and ownership contracts

`Information::make_call_view(const application_io::CallContext&)` returns a shared narrow `Information::CallView`, implementing actual profile logging/current UTC effects plus explicit `void close()`. The view captures a copy of the immutable exact Parent/JobPermit/thread/fiber/borrowed checkpoint context. It never swaps the root's legacy job/checkpoint or changes lower provider identities. The caller retains raw checkpoint backing through explicit normal close.

The Information root uses a shared Impl backing solely for safe closed-handle metadata lifetime. Its exact original Options/stream/clock/formatter remain singular; views do not copy Options, construct new lower clocks, clone business state or publish another Runtime. Every actual effect dereferences those same owner-held providers. The creator root remains required to outlive open/failed views. Its destructor rejects registered/pending views, sets its existing retirement seal and destroys Options on the original creator actor. A normally closed handle may be destroyed later without calling the old checkpoint or unexpectedly retiring creator-owned providers.

## Admission, execution and retirement

A stack actor admission is linked before list/state/wrapper allocation and caller callbacks. Candidate nodes and wrapper metadata allocate outside the owner gate, then splice/revalidate. Published preparing state covers allocator-handler reentry during wrapper allocation. Each view has independent active/checking/closing/failure flags, actual Clock/Formatter leaf adapters and its own immutable context.

Different admitted actors can use the same owner concurrently. Legacy original methods can also progress while healthy child views are open or executing. Short registry transactions share sticky owner failure health: an unknown view poisons both itself and the root, so root/sibling checkpoints cannot continue on a falsely healthy owner. Healthy activity alone does not poison or reserve the owner globally.

Same-actor reentry rejects across an active view, another already-created view, pending factory admission and the legacy child-job route. The scan examines only execution metadata; it never borrows unrelated payloads or traverses Parent catalogs. Idle open views do not prevent the original worker or another actor from progressing. New exception construction is outside the nonrecursive owner gate. The narrow existing child preflight was likewise changed to unlocked rejection when adding this cross-route check.

No owner gate spans a provider, allocator, shared-log lease, checkpoint, clock, formatter, stream effect or target destruction. Added metadata scopes locally preserve LastError. The shared CallContext's caller checkpoint behavior is retained as defined by that header; this code does not wrap an entire view call to restore pre-effect SDK error.

`close()` runs under its exact still-live context, seals the view, checks caller authority, publishes closed state and removes its row via nonallocating splice. Row retirement occurs after unlock. A closed destructor is metadata-only. An unclosed/failed wrapper destructor marks shared owner failure and leaves its execution registered; it does not consult stale raw checkpoint backing or guess normal cleanup. There is no source payload/file owner in Information; Content separately owns its per-view file inventory.

## Authored validation and bounds

Existing unit cases remain: actual information handlers/XML, counted username logging, supplied UTC, nullable gmtime, denied checkpoints, swallowed reentry, foreign worker/fiber denial and attached provider retirement. The new unit case checks the actual last formatter/flush error survives the legacy facade's metadata gates. It does not assert a test was executed.

The SDK agent's separate actual PreparedMemory scenario is responsible for live CallContext/view integration, concurrent Content hold plus Information UTC/log, exact bad/stale contexts, open views during original methods, explicit close and later closed metadata destruction. This author did not add another thread framework or fake permit to the Information unit file.

Shared custom lower providers are not automatically made thread-safe, and this stage claims no complete arbitrary business-state concurrency, native MSVC/CRT/EH ABI, licensing facade completion or actual production Graph concurrency. Original execution policy remains disabled until all coupled upper layers are integrated.
