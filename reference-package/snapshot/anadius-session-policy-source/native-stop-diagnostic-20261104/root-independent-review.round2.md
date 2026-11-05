# Root independent native system-diagnostic source review

2026-11-04. Root did not author these five changed files. Reviewed the final source, full captured-before diffs, fresh native category/write findings and actual compiler-provider disassembly findings. STATIC REVIEW ONLY; no compilation, tests, linking or execution.

The selected nonzero stop and synchronous greeting-write error paths now supply the same native020950 message provider to plain std::system_error. The module-owned category name is asio.system; original84530 and tracked84530 retain pqcs, selected84720 retains write. Error values, base std::system_error catching/copying, actor admission, source poisoning, work consumption and stop/wake prefixes retain their surrounding order. System category identity intentionally migrates away from compiler std::system_category: the latter's language0/repeated-space-trim/count/fallback mismatch was independently proved. Generic and misc branches do not migrate.

Reviewed lifetime without an exception subclass or ambient-current hook: registry key is the immutable typed SDK function pair; one category and prepared list node publish by recheck/splice. Candidate allocations and duplicate retirement are outside the registry gate. Module retention keeps base-value slices and extracted error_code references valid while this module/imported code remains live. Each message has its own stable Attempt. The category gate covers list operations, never SDK/string effects; independent calls can enter concurrently. Success unlinks and retires metadata outside the gate, preserving SDK LastError; throws leave passive state without guessed LocalFree, retry or category-wide poison. All function callbacks are static/import addresses, with no captured Policy ownership.

Fresh native slots00d120/00e0c0/00e0e0 return own-identity conditions and compare identity/value; selected native self key+8 is its object address. Inherited compiler error_category BASE methods match this logical domain; compiler system-category Win32 normalization is not delegated. No native vtable or class-layout overlay is claimed.

Reviewed substantive native tests: passive same/different import-pair identity; native name and raw defaultcondition/equivalence including negative codes; exact plain system_error pqcs values for trailing space, fallback, C-NUL shortening and LFCR; copy/runtime_error slicing/error_code identity without another formatting call; SDK LastError after metadata retirement. Two std::threads enter distinct static-record SDK callbacks through a bounded rendezvous, with no shared fixture global/check counter touched inside callbacks. Independent failure retention does not poison/replay later successful attempts. These are unexecuted source assertions.

Reviewed actual Policy terminal tests: new original systemraise535, existing tracked workerror531 and existing write515 all invoke injected actual020950 once and require complete native text, category and preserved code/copy behavior. Generic raise514 remains separate. Lifetime case fully closes/destroys a Policy while copied what/error_code and category remain usable under a live SDK fixture. Default test main reaches all new cases; isolated count28 is consistent.

No blocking source finding in the final candidate. This review does not prove execution, concurrency scheduling, native MSVC EH/allocator/SSO/CRT teardown parity, every028240 caller or code0 construction. Runtime raw failed-post/latch and child tracked-row end-to-end remain distinct open coverage. Fresh write evidence justifies the expanded system-only write binding; earlier stop-only notes/review are retained as round1 files and are not substituted for this final scope.

## Reviewed final file hashes

- `anadius-socket-source/native.hpp` SHA256 `8d14ded529a1f8766775612d4c910d72c711f23f49da112b32eddd80387e847c`
- `anadius-socket-source/native.cpp` SHA256 `688c0323472f7ca0601016dc1109d9706bc1276e60ae142b81687929b9aaa1b8`
- `anadius-session-policy-source/policy.cpp` SHA256 `9f4bc6e929af175bc208e32f2eeef64e05ffa83724a4ca2bba842a718d7bc5ff`
- `anadius-socket-source/native_tests.cpp` SHA256 `448916ae2019eaf65af3c4b097c8e11bfa8e3771cd1a515e7a38b89a35cd7e09`
- `anadius-session-policy-source/policy_tests.cpp` SHA256 `39e36fd47e4ed68a28f4f134fef4c90489b9663cbf2c72b8f8944374b5273389`
