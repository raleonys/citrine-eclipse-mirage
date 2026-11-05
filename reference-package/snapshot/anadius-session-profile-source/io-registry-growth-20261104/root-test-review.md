# Independent static review of root's two IO-stage test edits

Reviewed final source against this directory's saved before copies and the relevant fixture, loan and native-shaped cleanup implementations. No test/production code was edited by this review. No compilation, tests, link, REA query, original/reconstructed-DLL execution or packaging was performed. No blocking defect was found by static inspection. This is source review, not runtime PASS.

## io_memory_tests.cpp

Fixture environments now retain OwnedIoMemory's stable storage_view pointer, so by-value copies resolve current_storage() after growth. The previous initial2 exhaustion failure is replaced with a real third allocation and exact2-to4 table-length/current-view checks, while retaining old descriptor/projection/generation identity checks. Snapshots are refreshed before reuse; old span objects are not dereferenced after relocation. The newly allocated third record is freed before the existing forged/altered-descriptor checks. Their allocation counter is captured after that allocation, making the remaining assertion of two live records and no extra raw allocation correct. Existing generation reuse and ordinary failure checks remain.

The new pending_loans_survive_growth() starts at initial1, retains70 live records, and therefore crosses repeated metadata growth including the former64 ceiling. It uses one Receive and one Send projection through the actual PinnedAsyncReceiveApi/PinnedAsyncSendApi ownership wrappers, with fixture lower APIs returning pending997. The other68 records are Posted projections without kernel submissions. It checks old typed projection and OVERLAPPED addresses, handle generations, packet-pointer repair, copied packet metadata, live-view publication, and exact total/pending/buffer-borrow counts. It does not launch70 real kernel IO operations.

The strong-count assertion5 is supported: one original session strong, two source-handler strong pins from receive_owner/send_owner, and two independent SDK-loan pins from arm. Each corresponding weak pin also remains on the source projection. Freeing the68 Posted records does not consume either loan. Completing Receive/Send through the pre-growth copied environment releases the two SDK loans, the source-handler pins and source weak pins via the established observed-completion and027710/027a20 cleanup paths. The final strong count1 is therefore correct, and both callback counters must equal1. Their raw225/209-byte allocations enter the two actual source worker-cache slots; drain_terminal_cache frees both retained Record allocations through the current storage owner. Fixture finish then requires zero IO records, disposes the session and verifies parent/provider drain. The test is registered in main.

Coverage limits: this source checks two pending typed loans across70 held allocations, not external Accept alias registration, allocation failure injection, swallowed new-handler reentry, arbitrary category-provider callbacks or ABI equivalence. No such behavior is inferred from the fixture's intended assertions.

## graph_integration_tests.cpp

The only initial-capacity change is concrete_profile() Runtime capacity32-to1. This ordinary two-peer Graph scenario still runs twice, with default and explicitly selected Jobs. The added require is inside the existing socket202 first-receive overlap block after verifying both live sessions, old owner identity and complete session registries. It asserts current OwnedIoMemory table length >1 and that copied Ports.environment.storage resolves the exact current data pointer and length. Thus success would explicitly demonstrate growth and live-view resolution, rather than merely assuming growth from the smaller initial option.

The SDK receive function returns from state.native_mode before this ordinary branch. Prepared native70-message scenarios, their separate growth/retained-exit modes, negative prefixes and their expected child exit codes are unchanged from the before copy. Existing two-peer counts, packet processing, cache/provider drain and final Graph/Runtime/Profile close assertions remain. No new executable result is claimed.

## Final reviewed SHA-256

| File | SHA-256 |
| --- | --- |
| outputs/anadius-io-memory-source/io_memory_tests.cpp | 7ec876e70eac83d60838b56810436f857dfeca02376291c1cdad959b2aa10fbf |
| outputs/anadius-application-graph-source/graph_integration_tests.cpp | 6d6ca930e82f20dc81c9d0390f53bf117e17be501e171bd835010ae8f98357ad |
