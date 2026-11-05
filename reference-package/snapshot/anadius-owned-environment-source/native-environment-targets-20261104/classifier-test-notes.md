# OriginalDefault classifier regression source

Only `outputs/anadius-owned-environment-source/environment_tests.cpp` was changed. `graph_integration_tests.cpp` was inspected and deliberately left byte-identical to its preserved before copy after the root confirmed its existing worker-delay fixture uses Explicit lifecycle, not OriginalApplication. No compilation, tests, linking, DLL execution, ZIP update or new REA session occurred during this test-author task. These are authored assertions, not passing test results.

## Authored OriginalDefault case

`EnvFixture` accepts an optional `owned_environment::ClassifierProfile`, defaulting to Host. Existing Host tests—including initial1 constructor/copy growth, 4100 simultaneous references, reuse/expired capabilities, lineage, scheduling and all eight isolated retained-prefix cases—retain their previous mode and assertions.

New `original_default_classifier()` creates an OriginalDefault Environment with initial capacity1 and a real fixture SessionMemoryDomain/session owner. It checks:

- The session root remains a registered context-bearing state1 projection with a live query capability. This exercises the root projection API, not a fabricated raw native Slot memory overlay.
- Actual recovered `construct_io_environment_023dd0` produces state0 with the exact empty operations/schedule/executor sentinels, without adding created/copied/moved owned references or changing the source capability. Querying that empty clone rejects without poisoning the registered source; the clone is safely released.
- Two actual OwnedIoMemory receive/send projections clone that same source through actual023dd0. The fixture manually arms and records the typed pending loans (no WSARecv/WSASend call is made here), retaining two exact buffer leases and two additional source strongs in addition to the transferred handler strongs.
- Actual OwnedIoMemory completion calls invoke recovered `027710`/`027a20`; both direct body counters must be exactly1. Scheduling methods throw if selected. At each body the appropriate loan/allocation has already ended, while the moved handler and remaining operation still own their strongs. Final allocations, pending loans, buffer borrows and source strongs return to their captured baseline. Freed generations cannot resolve/replay their loans.
- The classifier/projection/completion portion adds no scheduler work pin: work_count must remain equal to its captured baseline. This bounded completion fixture does not claim to exercise actual021d10/021fd0 async-start work accounting or the physical dispatcher decrement path.
- Message-copy slot still creates an independently registered state1 projection under the same classifier. Its existing response schedule still consumes a real PostedFunctor through actual force-post022e40, and the fixture's actual source worker invokes it once while draining the existing accept/posted work. Only the source and explicit message copy contribute ledger creation/copy/destruction counts.

The test is wired into the existing main. No global lifecycle conditions, original worker affinity, child JobScope authority or existing failure assertions were weakened.

## Explicit coverage boundary

The actual Graph OriginalApplication dependency wiring is covered by the root's independent static source review, not by a new functional Original Graph integration in this task. `prepared_worker_session_delay` uses Explicit lifecycle, selected Stop and external proof callbacks with exact two-close-epoch assertions. Converting it would alter the purpose of that test and conflict with OriginalApplication lifecycle validation. Its source and all existing native70/reuse/delayed/negative scenarios remain unchanged.

The empty-clone query rejection regression explains why JobHandoff must skip capability query for the exact sentinel. It does not claim to execute that child JobScope path; no new child-scope fixture or native environment ABI/RTTI/vtable implementation was introduced.

## Identity and static review

Read-only diff against the stage before copy confirms the Environment test changes consist of the optional fixture classifier, the focused direct completion helper/case, and main wiring/output wording. Graph integration remains unchanged.

- Final `environment_tests.cpp` SHA256: `86fd0192a44cd64493a4d838f61a7f69cad1af298de1bf1453be314c5c6b7252`.
- Before `environment_tests.cpp.before.txt` SHA256: `145dec7a3ce33a856c9bb9a221bf9d8204ee1414fe26875c50cc99dd72897a7d`.
- Unchanged `graph_integration_tests.cpp` and its before copy SHA256: `6d6ca930e82f20dc81c9d0390f53bf117e17be501e171bd835010ae8f98357ad`.

Production files are owned by the other implementers. Root independent static review was requested before freezing these source results.
