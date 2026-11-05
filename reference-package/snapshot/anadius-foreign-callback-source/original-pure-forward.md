# Original pure forwarding and nested callbacks — 2026-11-04

Six argument-only Original callback paths now dispatch through the existing typed OriginalEffects under counted, automatic linked frames. Nested frames mask the outer callback's general/wait/redirect permissions and restore its unchanged state on exact LIFO leave. No private CallbackThreads entry, recovered PTD allocation or shared registry-set access occurs in these selected source paths. This is source implementation and static review; no compiler/runtime equivalence is claimed.

## Fresh native evidence

REA6.8.0 and Ghidra12.1.4 were verified for original `anadius64.dll`, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. The independent native auditor saved22 complete fresh instruction/pseudocode replies across11 functions in `pure-forward-native-20261104/`, with session identity, active provider, open/close results, findings and an Evidence index. The parent independently reopened the original and queried helpers0086b0 and008db0; complete replies are in `pure-forward-native-root-20261104/`. Both sessions were closed after export. Repeated content-addressed Evidence IDs do not imply a new auto-analysis run.

| Selected input | Native fact | Source dispatch |
| --- | --- | --- |
| Registry open A/W: null subkey | Originals at `0x18000f823`/`0x18000f983`; helpers return1 at `0x1800086e4`/`0x180008824` before resolver/CRT. | Exact typed original open and output cell. |
| Process A/W: null application | Null helper path reaches originals at `0x18000f4ac`/`0x18000f58c`. | All ten arguments passed unchanged; command line not inspected by selector. |
| Folder A/W: full supplied id !=5 | Full integer comparisons at `0x180008dfb`/`0x180008feb`; originals at `0x18000fc53`/`0x18000fcd3`. | Exact id, flags, token and caller output pointer. No flag masking. |

Null registry paths still perform an inert native language-enabled byte read and local SSO/security bookkeeping. The source shortcut omits those operations; null comparisons cannot match or fabricate/insert a handle. This exception is explicit and does not establish byte-identical instructions or original CRT ABI.

The historical general-callback audit is retained comparison. Its proposed SDK-only nesting gate was explicitly a source policy, not a native observation; this implementation does not add that gate. Neither the fresh static evidence nor the synthetic scenarios establish that Windows itself reenters these hooks.

## Source integration

- Native wrapper selection inspects only nullness or the complete folder integer under the existing Context lease. Every other open/process/folder input and all close/query calls retain their existing ordinary path. Cooperative/default admissions decline the optional route and retain ordinary behavior.
- A noncopyable automatic OriginalForwardFrame lives in the wrapper's nonmoving AdmissionLease. Domain links it above the current actor frame and counts it before querying the actual per-hook publication receipt. Checking frames mask all parent permissions. Successfully admitted top frames grant only original_scope.
- Each frame retains exact Domain, callback, thread, fiber and parent identity. Exact leave pops only the current token and clears it. Known missing publication or stop unwinds the pending count and restores the parent; an opaque proof exception additionally poisons the Domain. Native Context ownership outlives frame leave on normal and exception paths.
- Pure frames may overlap a general actor and nest above Full, connect wait/redirect/original or another pure frame. They never enter/leave CallbackThreads or probe the outer private PTD. Suspended connect mutations reject and connect permission queries returnfalse until the pure frame leaves. Outer general PTD checks resume through their existing path after restoration.
- Pending forward counts participate in active/drain/pause, activation, setup-begin, port-binding and destructor guards. Existing Original setup-completion behavior remains; no additional unload requirement was added.
- Dispatch remains through typed OriginalEffects and its exact-slot provider/checkpoint ownership, including checks before provider resolution and around the SDK call. Raw caller pointer values, writable outputs, lower return bits and incoming/outgoing LastError are preserved by these source paths.

One subagent collected fresh native evidence and independently reviewed production integration, one implemented the Domain side, and one updated/reviewed test sources. The parent implemented the native frame/selector/wrapper integration and reviewed the combined source. The implementation report's original foreign.hpp hash predates a final comment-only clarification; the independent production review records current final bytes.

## Validation boundary and remaining work

Test sources cover linked frames and checking-proof nesting, parent permission restoration, clean/dirty tokens, hook receipts, pause/drain and wrong-LIFO/fiber rejection. Synthetic typed SDK fixtures nest all six wrappers, including a connect SDK parent, while a separate Full actor and controller setup/pause remain active. They inspect arguments, outputs, return bits, LastError, Context counts and private-PTD exclusion. A thrown-leaf source case inspects restoration to the retained parent; nine excluded argument modes retain ordinary overlap rejection. These sources were **not compiled or run**. Existing live-port/concurrency/terminal scenarios remain source-only.

Four production and two test source/header files changed in this stage. Seventy source/header differences remain pending against the last real full build; all six files already belonged to that pending set. This count measures changed files, not total DLL completion. No new normal native body recovery, native ABI closure or numerical completion increase is claimed.

No compile, project tests, link, DLL execution, original mutation or ZIP update occurred. Frozen build/link/package proofs retain their historical meaning, and no new production DLL was generated.

The host Admission vtable and source layout changed. All consumers require a consistent future rebuild before using these bytes; original SDK signatures remain the existing typed interfaces.

Next native/source review should inspect the remaining general registry bodies and their shared handle-set/CRT boundaries, including actual hook-publication ordering and any early callback paths that still reject. Close/query and nonnull/matching general bodies are not made concurrent by this stage. New stages still require fresh REA evidence and scoped delegation; rebuilding remains gated by an explicit user instruction.
