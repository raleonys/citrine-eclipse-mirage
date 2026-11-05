# Pure forwarding test source update, 2026-11-04

Changed only foreign_tests.cpp and foreign_integration_tests.cpp. Preserved the live-port, concurrent connect, counter, wait, terminal15, and prior callback scenarios. All new cases are source assertions using synthetic SDK fixtures. They were not compiled or executed; no linker, DLL execution, ZIP operation, or original binary modification occurred. This report makes no claim that Windows itself performed the synthetic reentry chain.

## Exact source scope

| Changed test | SHA-256 |
|---|---|
| foreign_tests.cpp | 0fbd95fe74f96de23774a3452701bd7ce83af2ec6e4c38da48e8eef886e6b5d1 |
| foreign_integration_tests.cpp | 8e25b409a2668230c1741312723a73ee848993151c59fd5a53bee391d4005b1b |

source-hashes.json records absolute paths, UTC modification times, lengths, and identities for these tests and the production headers/implementations inspected. The production files were not edited by this reviewer. Root owns the fresh REA/Ghidra grounding for the six native branch predicates; this subtask did not create a second native session or claim new shipped-artifact observations.

## Direct frame contracts

original_forward_frames checks the following against the landed OriginalForwardFrame/Admission API:

- Cooperative decline and unsupported close/query/connect enum decline leave a default frame untouched.
- Missing actual requested-hook permission throws and clears the pending token/count. A different admitted hook cannot satisfy a missing nested hook's proof; the failed nested request restores its exact parent.
- A selected clean automatic frame records exact owner, thread, fiber, callback and parent. Reusing an admitted frame is rejected. Metadata preserves incoming/outgoing LastError.
- An admitted pure frame masks a controlled Full parent's general/wait/body permissions and retains only original permission. The existing controller PTD pointer remains identical.
- While an outer frame is still Checking, a synthetic publication callback sees no inherited Full/original/wait/body permission. It can admit and leave an independent nested selected pure frame; leave restores the permissionless Checking parent. This specifically checks absence of an added SDK-phase prerequisite.
- Nested admitted pure frames link to the exact stack top and release in LIFO order. Leave clears the token and restores the prior Full or connect permission.
- A pure frame atop a counted connect Waiting phase suppresses outer connect pending queries and rejects an attempted completion mutation. Leaving restores the unchanged wait; actual Execution wait then completes. Another frame nests atop the resulting connect SDK phase.
- All six selected callback enums can admit a foreign actor with no private domain PTD. Pause counts held pure frames, rejects new entries, retains the existing frame's original permission, and reports drain only after exact leave.
- Isolated child source modes deliberately attempt non-LIFO leave, leave from a different OS thread, and leave with tampered fiber metadata. They expect the existing terminal abort policy before parent release. The tampered-fiber case is metadata validation, not a claim about an actual fiber switch.

## Typed native-wrapper fixtures

pure_forward_routes registers actual synthetic SDK-typed function pointers through the selected Publication and original slots. It calls the source native entry wrappers in this nested chain:

openA(null subkey) -> openW(null subkey) -> processA(null application) -> processW(null application) -> folderA(full id0x4005) -> folderW(full id-7).

The folder ids deliberately establish full integer comparison against5, including a value with low bits5. Every fixture checks all passed pointers and scalar argument values, including security attributes, unnormalized inherit values7/-3, creation flags, environment/current-directory/startup/output pointers, and window/token/folder flags. Outputs include HKEY cells, PROCESS_INFORMATION process ids, mutable A/W command buffers, and A/W folder buffers. Distinct 32-bit LSTATUS/BOOL/HRESULT bit patterns and per-level incoming/outgoing LastError markers are checked without Boolean normalization.

The chain runs twice: directly through the null-key open wrapper and as a nested chain inside a counted connect SDK fixture. Exact native lease expectations are six and seven respectively. Both overlap a controller-owned Full callback. The final SDK fixture waits under its own condition variable while the controller completes host setup, attempts pause, and leaves Full. Counted pure/connect entries must still retain the bound Context until the synthetic SDK stack unwinds. The pure scope has original-only permission and no private PTD at each level; parent original scope/count is rechecked after each nested wrapper returns. After join, domain/native counts drain, the managed log is empty, tracked private allocations are unchanged, and normal Original close succeeds.

The thrown-leaf child raises a synthetic std::runtime_error from the folderW SDK fixture. Its terminal error observer checks that the leaf frame/native lease have retired, five parent native leases remain, the exact parent original-only scope is restored, the private PTD remains absent, and the original code owner is retained. It terminates the synthetic process from the existing explicit error-policy boundary instead of inventing a successful return.

Nine isolated negative source modes hold an overlapping Full callback and call close, queryA/W, nonnull-subkey openA/W, nonnull-application processA/W, and exact-folder5 A/W wrappers. They expect the ordinary Overlap rejection. Sentinel nonnull pointees ensure selection/admission must reject before ordinary body reads caller storage. These cases detect an accidental blanket forwarding permission that includes the excluded argument branches.

## Static review

No concrete type, lifetime, permission-restoration, or data-race defect was found by inspection of these test additions and their use of the landed APIs. No execution-based verification is implied.

All typed synthetic function signatures match the SDK-typed OriginalEffects dispatch. std::bit_cast comparisons preserve the complete four-byte return values. The fixture stores probe/state/output storage until after the callback thread joins and Owner closes. Publication retains actual synthetic code lifetime through the admitted stack and is stopped only after drain. The manual observed live-port factory from the prior stage still binds its exact PolicyView cell before Owner publication.

New SDK fixtures never invoke the shared non-atomic require check counter while running concurrently with controller work. They accumulate validation on their one callback thread; the controller reads results only after join. Leaf-entered/release state is protected by its mutex/condition variable. Each recursion level owns its argument/output slice, and all output observations occur after join. The exception observer runs on that same callback thread before terminating its child process. Direct frame tests use one actor at a time except the deliberate terminal wrong-thread contract case.

The original live-port scenarios and scalar helpers remain unchanged by this stage. Existing counter observations still occur after callback join and before CallbackSeed destruction. Source hashes bind this review to the inspected revision; later root edits require a separate amendment.
