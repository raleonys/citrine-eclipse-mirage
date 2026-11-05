# Native system stop diagnostic implementation

Source-only, 2026-11-04. This author changed only socket-source/native.hpp/.cpp and session-policy-source/policy.cpp. Actual before bytes and the pending-source/work-plan snapshots were copied into this stage's before/ BEFORE those edits. No compilation, tests, linking, DLL execution, package update or new REA session. This note is implementation/self-review, not independent validation.

| Final source | SHA256 |
|---|---|
| outputs/anadius-socket-source/native.hpp |8d14ded529a1f8766775612d4c910d72c711f23f49da112b32eddd80387e847c |
| outputs/anadius-socket-source/native.cpp |688c0323472f7ca0601016dc1109d9706bc1276e60ae142b81687929b9aaa1b8 |
| outputs/anadius-session-policy-source/policy.cpp |7a7603dc92eb5d8617d1efa6599aa0cb7f8f6542514b9d28352b81bf56d26771 |

Before hashes: native.hpp054f0a03884610c94ec0683f67735ce41fb2025130cbbd757f6c818da31d8e5c; native.cpp718bfac524e2b8d445e522b433abba165dee811e1301cc50a0ecc9fdc9a41a39; policy.cpp32de8c7a0f70371baae8117b009eaed9c25d6882e963eab74cf783f661b95759.

## Stable module category, plain standard exceptions

Public getter `const std::error_category& system_error_category_028620(SystemMessageImports={})` returns a typed logical category named asio.system. It does not overlay/call an original vtable address. A module-static pool retains each category until ordinary CRT module teardown. Its ONLY key is the immutable typed FormatMessageA/LocalFree function-pointer pair, so repeated/default calls share one category identity; a different injected pair identifies another selected provider.

This makes ordinary std::system_error base copies/slices and extracted std::error_code references valid during the category/module lifetime without a derived lifetime guard, per-error category cache, TLS/current-constructor variable or poisoned-Policy leak. Existing injected function-pointer code lifetime remains the caller's responsibility; retaining a pointer is not permission to unload its code then call it. This pool does not certify calls during/after module teardown and adds no new unload/drain prerequisite.

Lookup first searches under the pool metadata mutex. If absent, both the category and physical list node are allocated outside the mutex; final recheck chooses an already-published matching category or splices the prepared node without allocation. Duplicate unpublished candidates are destroyed after releasing the gate. Entire getter metadata preparation/retirement preserves incoming LastError and invokes neither message SDK import.

## Independent attempt rows and retained failures

NativeSystemCategory::message creates one local list Attempt node outside its metadata gate, then publishes it by no-throw splice. Each call has its own SystemMessageState and stable node, so parallel or SDK-reentrant message calls do not share mutable buffer state or encounter a global busy rejection. No gate spans FormatMessage, string allocation, LocalFree or new-handler code.

The existing reviewed system_message020950 performs the actual formatting exactly once. On success, the node is unlinked into a local retirement list and deallocated outside the gate. Both metadata allocation/publication and post-SDK retirement preserve LastError, leaving FormatMessage/LocalFree effects intact.

On helper/SDK/allocation failure the node stays passively owned by the category. A later independent message call creates a new attempt; it neither retries nor frees the old buffer/prefix. No SDK-buffer RAII, automatic phase reset or guessed native EH cleanup is introduced. At ordinary CRT pool teardown the plain Attempt metadata is destroyed; it does not invoke LocalFree on unknown retained pointers. This does not claim exceptional native cleanup/resource parity.

## Actual nonzero84530 Policy path

Policy::Impl caches the getter's stable category reference at passive construction using Options.system_message. Original Policy::raise and the tracked finish adapter select this native category only when their validated category index is1 (the actual system category), retaining pqcs context. Generic/misc explicit HOST inputs retain their old compiler category; write84720 remains unchanged. Existing format_override remains logging-only. No callback/current constructor is stored in the category.

The thrown object remains ordinary std::system_error. Its signed code value and catch type are retained; code().category() intentionally migrates for these system stop errors to the stable asio.system category. This is a documented HOST category-identity correction backed by native evidence, not hidden compatibility with std::system_category.

The actual selected compiler-provider audit proves nonzero std::system_error composition calls the supplied category.message once and stores prefix + `: ` + returned message; inherited what() returns that stored text. The supplied message is now actual020950 with selected imports, avoiding the disproved compiler defaults (language0/repeated whitespace trim/Unknown error/counted copy). No second preformatted diagnostic or custom what override is used.

Original terminal admission/poison order and tracked child Parent/JobPermit admission, completed work decrement, consumed flag and stop/wake/error prefix are unchanged. The category is a self-contained SDK/string value leaf; it does not call ordinary Policy admission after intentional poison or borrow a dead Policy checkpoint after an exception escapes.

## Category comparison domain

Fresh native signature evidence established default_error_condition(int) returns {value,this}; equivalent(error_code,int) compares identity key/value; equivalent(int,error_condition) calls the category's default condition then compares identity/value. The selected native A1110 object's key+8 equals its own address. The implementation inherits std::error_category BASE default methods, preserving that selected identity/value domain. It does NOT delegate to compiler system_category's Win32-to-generic normalization and does not claim native vtable-position ABI.

Root/alias retains the fresh comparison records separately. Tests should call both public equivalent signatures and verify the native category remains distinct from std::generic_category for comparisons.

## Boundaries and review status

Actual02a190 raises only for nonzero captured post error. Compiler make_error_str skips category.message for code0, unlike the observed general native00a270 constructor. This stage addresses the selected nonzero84530 path, not every arbitrary028240/code0 caller.

Normal diagnostic value/name/comparison domain and owned category lifetime are source goals here. Native exception layout, allocator/SSO capacities, C++ throw/catch/unwind ABI, exceptional buffer reclamation and module-teardown concurrency remain separate limitations. Additional HOST metadata allocations are explicit and do not establish native allocation equivalence. No completion percentage or whole-goal completion is inferred.

Root reviews these production files; another author owns the two source test files. No tests were executed. Independent source review and any future explicit rebuild are separate from this implementation note.
