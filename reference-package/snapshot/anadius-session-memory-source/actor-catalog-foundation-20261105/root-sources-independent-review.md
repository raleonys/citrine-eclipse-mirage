# Independent root source review

No remaining blocking finding in the final five files below, compared with this stage's captured before files and the agreed socket resolver APIs. This is static source validation only: **NOT RUN**. No compiler, test, child process, DLL, linker or packaging operation was executed.

`receive_buffer_tests::authoritative_mapping` runs actual010040/008350 growth with duplicate stale fallback rows while an owner sees the actual stable fixture catalog. It checks the grown payload and later owner revocation without fallback. Ambiguous pure lookup leaves the caller descriptor unchanged. The owner uses the fixture's fixed array, whose allocation entries update in place; this is not a stale span into a resized vector.

`response_containers_tests::authoritative_maps` runs actual026670 with offset3/size8. Its suffix stays at3–7, prefix moves to8–10 in the16-slot result. New allocation/free updates the owner's fixed bindings array while deliberately duplicated fallback rows remain unused. Revoked owner mappings reject despite valid fallback; duplicate owner mappings reject identity and window access. Null map address preserves zero behavior.

Frame header tests exercise actual error-message logging/free: exact data-8 owner lookup overrides an invalid stale base, and Miss/Ambiguous stop after flush without free/consume/rearm. Write-finish tests exercise both queue success and log-message destruction; exact owner base/4135-byte free wins over stale headers, Miss/Ambiguous preserve the pre-free queue/string prefix, and gap32 reaches the existing invalid-parameter terminal before free/reset. Existing legacy cases remain registered and unchanged.

Domain list-ticket preparation now allocates its compiler node outside the shared nonrecursive gate, rechecks poison, and splices the prepared node only when healthy. The iterator remains valid across list splice; actual heap allocation and existing failure-prefix tracking retain their prior order. Review identified that the new second poison exception could allocate while holding that gate. Root corrected both checks to capture a bool under lock and construct the exception after unlocking; the final bytes were reread and hashed. Prepared metadata cleanup also occurs outside the gate on that rejection path. No new whole-body callback lock was introduced.

Limits: socket owners are controlled typed fixtures, not actual Parent actor/payload-pin integration or a full NativeProfile composite resolver. Parallel Parent admission and lower failure/lifetime cases belong to the separate fixture/review. These cases establish meaningful source assertions, not native execution, aggregate binary compatibility or OriginalParallel enablement. Consistent rebuilding remains required.

| Final file | SHA256 |
|---|---|
| `anadius-socket-source/receive_buffer_tests.cpp` | `ad96114dde3e79cc572b24bdc8d803a642125259e0002d2faa25d7ffc073c875` |
| `anadius-socket-source/response_containers_tests.cpp` | `2ec3c704a325ec373df6e0fed73ebffa6eb35b29c4ee266cdbdacb958f9fa200` |
| `anadius-socket-source/frame_read_tests.cpp` | `cf31318984a45e3559474e9b5834f887b183d0526f696f27348a2091d61d99d1` |
| `anadius-socket-source/write_finish_tests.cpp` | `a8d58834188feaf3cc5093bb31d63c9bdf123acb2088fc31f41d3733604df469` |
| `anadius-thread-domain-source/domain.cpp` | `33bd376910b8b413d5f57906cf6bc2de451aff15bcb5bd623821f63e7e521c36` |
