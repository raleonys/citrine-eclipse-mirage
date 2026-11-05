# Independent Original connect lane state audit, 2026-11-03

Read-only source/native review. No source edits, compilation, linking, project tests, target execution, ZIP work, or version.dll access. The initially empty agent-local MCP session was opened only after the parent explicitly authorized an independent target; it was closed after all four complete Evidence records were exported. Original bytes remained unmodified.

## Native identity and evidence

Target: C:/Users/Skysect/Downloads/anadius64.dll, SHA-256 9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c. REA 6.8.0/Ghidra 12.1.4, analysis-profile digest 64922283ade10fadf28df5d1689f74f28ec027c5180b774a9d26635c0e69c554.

Complete structured records: fresh-evidence.json, SHA-256 5432b274b4a1037574b4ab45b4be634e504b86d1e2a3d76532983d892dd30b89, 32,116 bytes, four records, no unknown ledger entries. That absence is not proof of runtime equivalence.

| Operation | Address | Evidence ID |
|---|---|---|
| Instructions | 0x18000fd00 | ev_839077f99c02612bc8d409358a72a00b4078cf9a201c931361fd8e1c4381c349 |
| Pseudocode | 0x18000fd00 | ev_4b237267ebf4f1563f67a6e5c3bea40fbee890a0e1fceb1b3425de5a06fb7204 |
| Instructions | 0x180036880 | ev_9f1de4375f3c44c8e0c59d95b90042240c92853434c9a941fc374236aa1bc138 |
| Pseudocode | 0x180036880 | ev_c9f5209a7f663688e2ca0c2ba7a8583e52b0afd75fb8a3ab48395e4c03aa1efe |

Ghidra bodies: 00fd00 inclusive 0x18000fd00..0x18000fe56, 343 bytes; 036880 inclusive 0x180036880..0x180036ab3 and 0x180036ab5..0x180036abe, 574 bytes. Ghidra terminal-call assumptions exclude fallthrough after 00fe52->05cdc0 and 036aaf/036aba->04a648. These flags are not independent proof of nonreturn. Pseudocode misrepresents the final indirect tail jump as a void call; instruction evidence controls the SDK result/tail-transfer claim.

## Observed native invariants

- 00fd1d..00fd3c perform the two length/family alternatives. Unknown length short-circuits without reading family; a recognized length needs only the family prefix until classification succeeds.
- 00fd42 reads the network-order port once into the DI route value. 80/443 branch directly to 00fd73, with no 036880 call.
- All other recognized ports call 036880(-1) at 00fd64 BEFORE the 3216 test at 00fd6e and bound-port comparison at 00fd73. Thus a pre-wait bound snapshot is not a native prerequisite, including for 3216.
- 3216 always redirects after the wait. Other non-web ports redirect iff they equal the then-current bound port. Web ports redirect iff they equal the bound port.
- 00fd8a..00fdbb log and flush before counting. 00fdc0 loads the attempt counter into EAX, 00fdc6 increments EAX, 00fdc8 stores EAX, and 00fdce compares THAT EAX with 15. No LOCK prefix or atomic read-modify-write exists.
- The fifteenth local result invokes dialog at 00fe48 and exit at 00fe52 before port write or original dispatch. Otherwise the bound port is read again at 00fdd3, converted, and stored at caller+2 at 00fde0. The SDK dispatch is an indirect JMP through 0x1800a3838 at 00fe02.
- 036880 locks at 0368a5, snapshots the third/first/second flag bytes at 036916/03691d/036925, evaluates all three for count -1, unlocks at 03695f, returns at 036a9f only on completion, and sleeps/rechecks otherwise. It contains no admission-pause cancellation condition. Its locks are released before sleep.

## Current source findings

The current counted TLS lanes are coherent within their intentionally restricted scope. Context/SDK-code lifetime remains protected by the outer native Lease; AdmissionLease releases the source reservation before the Context lease. Domain::active, pause, begin, activate_foreign, and destructor account for original_connect_calls_. TLS owner/fiber and admitted/checking metadata prevent reentry and wrong-fiber permission. The scope branch grants only wait or original permission, without calling CallbackThreads or allocating a private PTD. General callback bodies remain one active owner.

Metadata operations preserve LastError using Error guards. invoke_selected restores entry LastError after selection/admission, preserves outgoing SDK LastError, and retains the Context throughout leave. Actual wait/body/SDK effects retain their own error behavior; a new stage method must follow the same metadata preservation rule.

Milestones::server_port_published publishes one immutable payload with release ordering, and server_port requires the Server bit plus valid payload with acquire loads. This is an actual bound-port publication, not merely the preseeded configuration value. assembly.cpp already wires the same Milestones instance and per-hook successful-create publication into the Original Domain. No new all-ready proof is needed for the proposed extension.

The current restriction is not a faithful implementation of overlapping native redirect calls or overlapping cold-start non-web calls: try_enter_connect_original at foreign.cpp:189 requires an already published different bound port, explicitly excludes 3216, and complete_connect_original_wait at :229 requires the same earlier snapshot and unconditionally transitions to original-only. The native wrapper's direct_original branch at native_callbacks.cpp:80 can only wait then dispatch original. The current void stage API cannot choose redirect versus original after a wait. Ordinary uncontended Full callbacks still use the actual native-style body, so this is an overlap restriction rather than a universal missing redirect implementation.

The existing non-atomic ++attempts in connect.cpp is safe only while redirect bodies remain serialized by the ordinary single-active domain. Expanding counted lanes to redirect bodies without changing ALL callback counter accesses would introduce a C++ data race. This is a required companion change, not a current SDK-only-lane regression. Likewise, matching-port/concurrent cold web fallback into ordinary raw config access needs the parent's separate server-port-writer review; this report does not establish its safety.

No independent syntax/type error or unavoidable deterministic failure was found in the current unexecuted foreign_tests.cpp and foreign_integration_tests.cpp by static inspection. This is not compiler/test validation. Existing cases exercise current exclusions deliberately: original_web_lane_selection rejects missing/matching snapshots and 3216; concurrent_original_connect overlaps two untouched and one different HTTPS SDK-only call; staged_wait_connect(false,true) publishes Server before entering the Original concurrent wait; redirect integration cases use the Cooperative staged path. Those tests therefore do not demonstrate Original concurrent redirect bodies or Original waiting before Server publication.

## Minimal coherent transition

1. Extend the counted TLS metadata with explicit phases such as Checking, Waiting, RedirectBody, OriginalOnly, rather than equating a counted reservation with original permission. Keep one counted ownership through all phases. Its leave path must release the count even when wait, publication check, body, or SDK throws.
2. Keep immutable length/family/port classification under the native Context lease. Untouched calls can enter OriginalOnly after the actual connect create receipt. Non-web recognized calls, INCLUDING3216, can reserve Waiting after that receipt without reading or requiring a bound port. Invoke the actual wait exactly once with -1.
3. After a normal wait return, recheck the same live hook/code publication and obtain the actual immutable server-port payload. Choose RedirectBody for 3216 or a match; otherwise choose OriginalOnly. A missing/invalid post-wait publication must throw without SDK/body permission. For80/443 choose immediately from an available actual snapshot, preserving the no-wait branch.
4. Return a typed route or optional redirect-port result from the completion/classification API. Do not reuse the old void completion that hardcodes OriginalOnly. A returned port must be a copy, not a raw mutable configuration reference. Extend permits_connect_body_current only for the exact admitted TLS actor and Body phase; keep general registry/CRT/callback permission false. Keep SDK permission false until successful body completion.
5. Preserve pause semantics: pause denies new reservations and reports false while any counted phase remains. An already admitted wait must still wait for native startup completion; pause cannot fabricate successful completion or retire code/providers. Body completion and post-wait checks may continue after pause while the same live publication remains. Setup completion must remain possible while waits are active.
6. Replace every callback-path counter increment with an aligned std::atomic_ref<uint32_t> relaxed LOAD, local unsigned +1, relaxed STORE; test the cached local value for 15. This avoids C++ undefined behavior while retaining the observed separate MOV/INC/MOV and possible lost updates. fetch_add would introduce stronger atomic RMW behavior absent from the binary. Ensure alignment meets atomic_ref::required_alignment and no concurrent callback retains plain counter accesses. Seed writes before publication and observations after join/drain remain ordinary owner operations.
7. Keep existing managed stream acquisition per operation; it protects ostream internals without inventing one transaction over the entire native log sequence. Retain per-effect permission postchecks before counter and port changes, terminal15 ordering, caller pointer/length identity, four-byte mutation bound, and SDK result/error.

## Static test additions needed with that source change

- Two overlapping Original non-web entries begin while Server has no payload; a controlled actual Execution wait observes no body/SDK/general permission, then real Server/Online/Libraries finish selects different-port original, matching-port redirect, and3216 redirect.
- Original web80/443 match versus differ with no added wait; missing actual payload cannot be replaced by seeded configuration/global readiness.
- Counted Body allows only network log/UI effects, rejects general Policy/registry/CRT and SDK until completion; overlap, fiber changes, same-thread reentry, thrown observer/effect, and revoked publication release or poison ownership consistently.
- Pause/setup completion during waiting or blocked body retain Context/code and cannot cancel the actual wait; after body/wait and SDK return, source and native counts drain and per-thread LastError remains exact.
- Sequential counter wrap and terminal15 must use the cached next value; a concurrency scenario must allow the native separate-load/store lost-update behavior rather than asserting one unique global increment per connection.

Inspected source identities are recorded fully in source-hashes.json. Principal initial hashes: native_callbacks.cpp 67f6f5fb57dc02b977edd9780df73a4a2637409f759e806847b33cd0af11caf7; foreign.cpp 5502502c72d7ebfe309c450cf61ca0cf36ca85cbe969ee04b6bfdbc526ba83ed; foreign_tests.cpp 0355d678ea1f60d3d3c31a1e75214a53c8c0c963a49de68f90e9cf43d2b51b17; foreign_integration_tests.cpp 233a79b92f1ec58d3478013f535e8fa60e01a76db78c517bf494447964463dac. Hashes describe the files audited, not any later parent implementation.
