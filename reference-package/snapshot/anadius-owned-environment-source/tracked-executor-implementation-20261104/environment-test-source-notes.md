# Tracked executor Environment regression source

Edited only `outputs/anadius-owned-environment-source/environment_tests.cpp`. Preserved the current source first at `before/environment_tests.cpp.before.txt` (SHA256 `86fd0192a44cd64493a4d838f61a7f69cad1af298de1bf1453be314c5c6b7252`). No production files, other test files, original binaries or historical build proofs were edited. No compilation, test execution, linking, DLL execution, packaging or REA session occurred during this source-author task. Assertions described below are authored coverage, not observed passing results.

## Controlled finish capability

EnvFixture now supplies the new exact `Ports.environment_work_stop` interface, required by OriginalDefault binding. The controlled cap matches its real Parent/scheduler, validates its original actor and live Parent Operation, calls the borrowed checkpoint, performs one atomic decrement, marks consumed immediately, and calls actual recovered `stop_worker_02a190` on oldcount1. It delegates the stop flag/zero-packet effects to the existing controlled fixture implementations. It has explicit before/after-consumption exception hooks solely in this test effect provider, not a production metadata mutation API.

This cap deliberately refuses `finish_job`: the fixture uses a sole-owned Parent and cannot manufacture a valid prepared child JobPermit. Thus it tests Environment's integration with the capability contract and actual02a190 body, not Runtime/Policy child admission or the complete production capability. Root owns separate concrete Policy tests for its normal/error0/retained-error paths. Those results must be assessed separately; this file does not claim them executed or passed.

## Positive public-family regression

New `tracked_executor_families()` uses OriginalDefault and initial ledger capacity1. It finds exported bindings by actual executor/schedule identity, avoiding fixed array indices and private flag/row setters.

- A real session root has initial type `0x1800a25c0`. Its actual exported factory58 creates the distinct tracked executor/schedule/type `0x1800a2690`, with exactly one scheduler work retain.
- Actual023dd0 on a tracked source selects the nonempty type/factory path and retains a new tracked projection. Message copy also retains exactly once. Actual023cb0 transfers the existing capability/pin; cleanup of its moved-from sentinel does not finish a pin. Normal destruction consumes one cap call for each retained reference.
- Tracked prepare40, response20 and response88 preserve the tracked family and each retain once. Initial response20/88/prepare keep the initial family with no work retain. Actual023dd0 also clones a forced tracked response so factory58's bit0 preservation is checked through observable scheduling.
- Matching worker TLS makes flags0/prepare-cleared bit0 select actual022e40 inline consumption, in both families. Response20 sets force-post; response88 and factory58 preserve that bit. Four response functors plus one typed OwnedHandlerTask use the actual posted envelope/028ca0 path even with matching TLS. The typed handler overload is exercised directly, preserving the caller's mutable `OwnedHandlerTask*&` slot rather than adapting a copied slot.
- The test distinguishes six live family pins from five posted-operation pins. With 70 additional preallocated, stable-address tracked factory/copy destinations, initial1 metadata grows repeatedly while old source/tracked capabilities remain intact. New capabilities are unique, and no finish occurs during relocation.
- All ordinary tracked results account for exactly78 consumed finish effects (70 held results plus8 other created/copied tracked projections). After releasing them, exactly five posted pins and the original accept pin remain. A pre-existing final tracked root is retained for the cleanup case. Initial projections retire metadata without consuming work.
- Actual explicit02a190 is called before the fixture worker run, allowing it to return with that one deliberately retained family root. The real source worker consumes all five posted callbacks and drains their work plus accept work. Returned-phase cleanup destroys the already-existing root; it does **not** create a new source/factory result after admission closes. Oldcount1 immediately enters actual02a190; the already-set stop flag prevents another wake/post. Total consumed family finishes then equal79 and work returns to0. Source metadata/session ownership is finally drained through the existing close path.

The destinations/capability buffers for the70-held case are allocated before registration and never resized or relocated while live. Root lifetime, family work pins, posted work pins, IO envelopes and capability bookkeeping are asserted independently.

## Retained unknown-outcome prefixes

Two isolated source cases were added to the existing controlled process harness:

- `--tracked-before` / exit310: capability throws before decrement; Environment remains poisoned with its two metadata references, no successful destruction, one entered effect, zero consumed effects and unchanged work.
- `--tracked-after` / exit311: capability throws after decrement/consumed=true; metadata remains retained/poisoned, one work unit remains consumed, and no rollback is fabricated.

Both cases attempt another release only to assert admission rejects before reentering the effect; counters must not change. The existing eight isolated Host retained-prefix cases are unchanged. Main now advertises ten cases. Existing Host4100 projection growth and OriginalDefault empty/direct IO tests remain wired and keep their prior semantics; the fixture's new controlled cap is unused for Host-family metadata release.

## Explicit uncovered boundaries

- Actual prepared-child Environment/Runtime/Policy finish route remains validation coverage **OPEN**. Source implementation and static review may establish its design, but no fake JobPermit, native thread framework or claimed child-last-release test was added here.
- Post-false/error0/category ordering and throwing production Policy stop effects are root-owned separate regressions, not this controlled cap's coverage.
- No admitted input exposes native nonzero bit1 or null context in this owned context domain. Tests therefore prove prepare bit0 clear, response20 force-post and preservation on88/58; they do not claim bit1-nonzero clearing or null-context behavior tested. No debug setter was added to manufacture those states.
- Native DWORD overflow/underflow, native EH/unwind, all RTTI classes, raw ABI equality and concurrent lifetime safety are not claimed.

## Static review and identity

Read-only source comparison, public API inspection and a whitespace diff check were performed. Work-count arithmetic, stable destination lifetime, source/moved-from cleanup, controlled effect ownership and the pre-existing-root Returned setup were inspected without invoking a compiler or test runner. Independent root review was requested.

Final `environment_tests.cpp` SHA256: `0777c3614d7f8cdb96bfe3c02546a737f29557ac1b809bc7061dfea59f84c734`.
