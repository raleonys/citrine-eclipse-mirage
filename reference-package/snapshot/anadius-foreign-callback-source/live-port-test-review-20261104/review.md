# Live-port test source update and static review, 2026-11-04

Only foreign_tests.cpp and foreign_integration_tests.cpp were changed for this stage. Production files were read for API and behavior alignment. No compiler, project tests, linker, target/DLL execution, ZIP work, or version.dll access occurred. Test assertions below are authored source scenarios, not observed test passes.

## Exact source identities

- foreign_tests.cpp: SHA-256 73ede05da923e0585c8031a226d8eb2493ef99168daaf6aad9b14d82942b6eb7.
- foreign_integration_tests.cpp: SHA-256 2bde0958ffd8c4efee931a317fe58f54046f995d90299164c30926b40e3f3848.

source-hashes.json preserves full absolute paths, SHA-256, UTC modification time, and byte length for both changed test sources and all inspected production API/helper files. The shell remained in PowerShell; no REA/Node/Java/LLVM process was needed for this source-only stage.

## Unit scenarios changed

Original construction now passes no Cooperative redirect snapshot provider. A negative constructor case explicitly checks rejection of mixed HookPublicationProof plus snapshot. Core Original admission binds the exact configuration uint16_t cell once from the controller before setup and checks missing binding, foreign-thread binding, same-address repeated binding, different-address rebind, and mismatched equal-copy admission. Missing/mismatched binding checks include untouched Original candidates as well as web candidates. Failed checks leave no counted reservation.

Direct calls use try_enter_connect_original(exact_cell, requested, wait). Untouched admission gains only SDK permission. Cold live-zero HTTP selects SDK immediately. Matching HTTP selects Body without adding a native wait. Mutating the scalar after selecting Body no longer invalidates completion. Different HTTPS remains SDK-only with no private PTD.

Non-web tests keep actual Execution.wait_steps_036880(-1) calls before completion. A port changed from80 to4000 during the admitted wait selects a match after return; a port changed from4000 to80 selects passthrough. Special3216 redirects after its wait even with the live scalar zero. Missing-snapshot expectations were replaced by actual hook receipt revocation cases: failed wait/body checkpoints retain no wait/body/SDK/general permission until exact leave, after which a new lane can enter.

Scalar mutations in this core test occur on its one worker thread while the controller waits in join and never accesses the cell. Plain accesses therefore occur at quiescent points relative to transient scalar atomic_ref operations, with no concurrent scalar writer. The hook grant boolean is also worker-owned during the admitted sequence.

## Native bridge integration scenarios changed

All Original-profile Domain constructors in the two owned test files supply an empty Cooperative snapshot callback. make_factory performs its new passive exact-view bind. The manual observed native_context::make_factory path binds Domain to PolicyView.backend.configuration.server_port inside its PolicyFactory before Owner publication; it does not also call the foreign make_factory wrapper. Existing Cooperative snapshot routes are retained on their separate constructor paths.

original_cold_web_connect invokes the actual native connect_entry for80 and443 while Server/Online/Libraries flags are all zero, milestone bits are zero, and the exact live scalar is zero. Assertions cover immediate SDK dispatch, unchanged sockaddr, no startup sleeps/log/private PTD/domain allocation, exact pointer/length and incoming/outgoing LastError. This scenario specifically exercises the source entry route that the former snapshot-dependent profile sent through the ordinary fallback.

original_live_port_rereads starts with a matching live HTTP scalar80 and no startup completions. Its managed LivePortLogBuffer changes the exact cell to4660 when the redirect suffix literal is inserted; the helper's next log-port argument must therefore read4660. sync changes that cell to6001 and sets LastError0x6b00; the helper's final caller-port write must independently read6001. The successful final hook receipt check changes the scalar again to9000; this cannot undo the decided route or rewrite the already stored caller port. Assertions require log4660, caller6001, live9000, one SDK dispatch, unchanged family/tail/pointer/length, no added wait, no private PTD, and SDK incoming flush marker0x6b00/outgoing0x7a55. Both stream and receipt checkpoint writes use store_server_port_0a3c90.

The two stream callbacks also record that only Body permission is active, with general callback, wait, and SDK permission denied. The boolean redirect-pending query has no scalar return; the bridge uses the real Context.network.server_port reference for helper effects. These new assertions would detect a copied-port helper, equality recheck during completion, missing post-effect reread, or fabricated Server prerequisite.

Existing concurrent redirect/passthrough, real non-web waits, quiescent private counter observation, flush LastError, terminal15, and source-close scenarios remain registered. Original terminal dispatch and callback-thread UI setup are preserved.

## Static review result

No concrete new type/lifetime/data-race defect was found in the written scenarios or their production API usage by inspection. Bounded searches found no obsolete connect_original_redirect_port name or old Original try_enter signature in the owned files, and no Original constructor still supplies a scalar snapshot provider.

The new live-reread scenario runs one callback thread while the controller joins. Its checkpoint flags and receipt count are read on the controller only after join. Its managed stream serializes buffer access; callback methods do not reenter the stream. Its stored Domain and port reference remain alive until after Owner close and stream destruction. The proof captures referents that outlive all callbacks and the Domain. Scalar changes during callback execution use the exact shared helper; raw final reads occur only after join.

Concurrent existing SDK probes use their own mutex and distinct result slots rather than incrementing the shared non-atomic require counter. The raw attempts pointer is read only after every callback joins and before Owner reset, so CallbackSeed remains alive and no counter atomic_ref overlaps the read. Core tests call require on one worker at a time while their controller is joined/waiting, avoiding a new checks-counter race.

## Native grounding and scope

The same shipped-artifact instruction evidence remains relevant: target anadius64.dll SHA-2569e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c; Ghidra12.1.4 instructions ev_839077f99c02612bc8d409358a72a00b4078cf9a201c931361fd8e1c4381c349 from the earlier fresh independent session. It observes live comparison at0x18000fd73, log-port read at0x18000fda9, final replacement read at0x18000fdd3, and no second route comparison after the body. The non-web wait at0x18000fd64 precedes3216 selection and bound comparison. Counter load/increment/store/cached-EAX threshold remain at00fdc0/00fdc6/00fdc8/00fdce. Complete records and their terminal-call/decompiler limitations are preserved in the earlier lane-state-native-review-20261103/fresh-evidence.json; no new native session was opened here.

This stage supersedes the former test assumption that Original web routing requires a Server milestone or frozen scalar snapshot. It does not claim runtime equivalence or a successful build. Root owns final production review and any later explicit rebuild authorization.
