# System message020950 regression source

Only `outputs/anadius-socket-source/native_tests.cpp` and `outputs/anadius-session-policy-source/policy_tests.cpp` were edited by this test author. Their before copies were captured normally before editing under this stage's `before/`; this is separate from the production author's reconstructed before-record provenance. No original binary, production source, historical validation proof or delivered ZIP was changed. No compilation, tests, linking, target/DLL execution or packaging was performed. All coverage below is authored source, not passing execution results.

## Direct native-helper cases

The existing socket-native test executable now invokes the actual stateful `system_message_020950` with injected FormatMessageA/LocalFree callbacks. Explicit standard `<algorithm>`/`<limits>` includes were added.

Sixteen normal cases assert:

- Exact FormatMessageA flags0x1300, sourceNULL, full incoming DWORD code bits (including negative/high-bit values), language0x400, initiallyNULL allocated-buffer pointer, size0 and argumentsNULL. Default import construction points at the actual system entries and invokes neither; injected state/import construction is also passive.
- At most one LF removal followed by at most one CR removal. CRLF, LFCR, repeated LF and repeated CR have distinct expected results. A lone LF/CR or sole CRLF takes the exact17-character `asio.system error` fallback.
- Count0 fallback with both a nonnull supplied pointer and NULL. LocalFree must receive the saved pointer exactly once on both paths, including NULL; its normal nonnull return value is ignored.
- Returned count controls the trim positions, not final copy length. A short count over a longer C string, a short count that inserts NUL before a later tail, embedded/leading NUL, ANSI bytes0x80/0xff plus CRLF, and a5000-byte message distinguish these behaviors. No Unicode conversion or locale-dependent OS output is used.
- The injected LocalFree checks the **mutated C buffer bytes before release**, then overwrites every supplied byte. The returned owned text must remain correct, proving its copy/creation precedes free and does not retain the SDK buffer.
- On normal return all persistent SystemMessageState fields clear, after the helper's final checkpoint.

A narrow checkpoint case deliberately clobbers LastError from its metadata callback while FormatMessage/LocalFree set distinct values. Its named const std::function is constructed before the helper call, and LastError is captured immediately afterwards so a temporary callback destructor cannot change the assertion. The helper must leave LocalFree's SDK LastError visible. This tests the source metadata guard, not a claim that original020950 calls GetLastError or preserves incoming error state.

Two controlled **injected C++ effect** cases throw after FormatMessage writes its pointer, or upon entering LocalFree. They assert the exact retained pointer/entered/release-entered phase, absence of automatic cleanup, and rejection of state reuse without repeating either effect. These do not simulate allocation failure or claim that actual Win32 SDK functions throw, and infer no native MSVC unwind behavior.

An additional fourth-checkpoint failure occurs after LocalFree returned normally. The state must still retain entered/release-entered and its passive saved pointer until that final checkpoint succeeds. Retry rejects without another checkpoint, FormatMessage or LocalFree; the test does not dereference the saved pointer after release. This covers the reset-before-final-check defect found and corrected during root review.

## Actual Policy default system category

`native_system_category_messages()` reuses the existing real PolicyFixture/ErrorContexts and clears `format_override`. Injected system-message imports are supplied through `Options.system_message`; fixture construction must invoke neither SDK callback.

Seven lines flow through actual recovered read/write finish025480/025a70 and the real default system-category object. They cover trimmed CRLF, embedded NUL, LFCR, NULL/count0 fallback, all-trim fallback, count-vs-Cstrlen and a5000-byte result. System code2 is logged (only actual misc code2 is EOF-suppressed). The SDK callbacks check that the source prefix is already visible while the message has not yet been appended, and poisoning at LocalFree must not corrupt the final stream text.

Each line must flush once, leave no live string headers or additional Parent allocation tickets, and release Policy logging state. A separate explicit override case keeps both valid injected imports but verifies neither is invoked when the override supplies system text. The passive validation function now rejects a missing FormatMessage or LocalFree import. Existing misc/default-override tests and all27 isolated Policy retained-prefix cases are unchanged.

## Static validation and limits

Compared the two files to their preserved before copies, inspected final production API/state ordering, checked callback signatures/initialized buffers/trim ranges/synchronous callback lifetimes, and ran read-only whitespace diff checks. No compiler or test runner was used. Independent root source review was requested.

No real FormatMessageA, LocalFree, localized message, network API, loader, DLL or game execution is involved in these authored cases. Native string ABI/allocator/EH equivalence, malformed nonzero-count/null-buffer SDK output, allocation exceptions and concurrent callback behavior remain outside coverage. The persistent-state unknown-effect tests are source-owned containment checks, not native exception reconstruction.

Final SHA256:

- `native_tests.cpp`: `419114eeda6048e0beb73b816c837d7e1a97f1a8d84f167b435593e4fe193357`
- `policy_tests.cpp`: `601d9f0e69ee3a4f1aa353c360ff2389d9445a9f4c9e1005bab272fbc869aa73`

Before-copy SHA256:

- `native_tests.cpp.before.txt`: `bf9588da4834d6015619440bb7f8fa3f398a0f28ccd21e7bc20f66dc8b52ce96`
- `policy_tests.cpp.before.txt`: `eebf32b0b958224f01d90e510cffca1d597c4b45f6076ed480a260c582649670`
