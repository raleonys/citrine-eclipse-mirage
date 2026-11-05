# Independent84530 literal correction review

Result: STATIC PASS for the two frozen diffs. No blocking finding. Read-only source comparison against this stage's actual `before/policy.cpp` and `before/policy_tests.cpp` captures, plus the fresh literal/constructor evidence. No compiler, tests, linker, DLL execution, new REA session, or packaging.

| Current file | SHA256 |
|---|---|
| outputs/anadius-session-policy-source/policy.cpp |32de8c7a0f70371baae8117b009eaed9c25d6882e963eab74cf783f661b95759 |
| outputs/anadius-session-policy-source/policy_tests.cpp |8fcbb082fb0adb4640e341a33006d422bd62581c9a6997ae279a8e7a13948d28 |

Reviewed baseline hashes: policy.cpp `5c42ccbff002069dc4b1bdb111f6ed59efcb634ecc01efaef7890fd55b06307f`; policy_tests.cpp `601d9f0e69ee3a4f1aa353c360ff2389d9445a9f4c9e1005bab272fbc869aa73`. These captures include the preceding recorded source stages; this correction does not rewrite their frozen evidence.

## Production change

The anonymous-namespace constexpr char array contains exactly pqcs followed by its implicit C NUL. That matches the measured prefix `70 71 63 73 00` at0x180084530 in `bytes-180084530.json`, EID `ev_9e3850282dbec7d20b4d26ecef4374ea44cd967544f4cf322f655cc55e91227d`.

Both actual84530 projections use this same mapping: ordinary Policy::raise and the tracked finish adapter's raise028240. Their context validation, category-index mapping, std::system_error type, signed integer code and compiler error-category identities are unchanged. The original terminal admission/poison sequence and the child's already-consumed work counter/stop-wake/error prefix are untouched. No new callback, SDK call, allocation, retry, category registry, lifetime owner, or TLS mechanism is introduced.

The distinct write84720 path remains unchanged with its existing write context. Other typed SourceError wrappers and their uninspected literal mappings are not changed by this narrow84530 correction.

## Test source changes

The retained workerror case now checks an exact `pqcs: ` prefix through a caught const std::system_error reference, while retaining its code77 and std::system_category address checks. Its consumed flag, zero work count, changed stop/wake words, one post and one immediate-error-read assertions remain intact.

The existing generic worker terminal case similarly adds the exact prefix while preserving code and compiler generic-category address assertions. The conditional leaves the separate write case's expectations unchanged. No existing scenario/main invocation or counter/failure assertion was removed. These are source assertions only; no scenario was run.

The fresh native00a270 body supports nonempty-context followed by `: ` and a supplied category-message value. The prefix assertions therefore exercise the recovered context/separator, not a broad claim that the unexecuted compiler exception's entire what() matches native output.

## Remaining open boundary

This correction fixes only a measured literal-input defect. Full028240/native what() reconstruction is still OPEN. Native00a270 invokes the supplied category's slot10; for the native system category,020950 is separately proved. The selected LLVM compiler system-category message implementation has not yet been compared with that native provider, so equivalence is **unproven**, not asserted equal or different by this review.

No change to category identity/lifetime or to compiler exception ABI is justified by the literal fix. Native string allocation, MSVC exception text/copy/throw/unwind ABI and exact compiler-generated message suffix/effects remain outside this STATIC PASS. Neither the whole reconstruction objective nor028240 completeness is declared achieved.
