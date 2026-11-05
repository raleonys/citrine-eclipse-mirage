# Native stop diagnostic category: test source

Only `outputs/anadius-socket-source/native_tests.cpp` and `outputs/anadius-session-policy-source/policy_tests.cpp` were edited by this test author. This is authored source validation, **not compiled or executed**. No compiler, project test, linker, reconstructed/original DLL, REA session or ZIP update was run. Fresh native evidence and production implementation were handled separately by the parent and other agents.

## Socket test additions

`native_system_error_values()` checks the first injected-pair factory lookup is passive, repeated immutable typed import pairs select the same category, a different LocalFree function pointer selects a distinct category, and the default SDK pair has stable identity. The category is named `asio.system`, distinct from the compiler system category. Missing import pointers reject. Raw codes 2, -1 and INT_MIN retain their code and native identity in default conditions and both equivalence overloads; compiler generic/system identities and unequal values remain distinct. These expectations follow the newly confirmed native slots; no compiler Win32 normalization is assumed.

Plain `std::system_error(error_code,"pqcs")` cases require exact owned text from actual020950: CRLF trimming preserves a preceding space, count0 selects `asio.system error`, embedded NUL shortens the successful C string, and LFCR removes at most the final CR. Each constructor performs one FormatMessageA/LocalFree pair. A `std::system_error` copy, `std::runtime_error` value slice, repeated `what()` and extracted error_code preserve text/category without another formatting call. A separate direct category message case captures LastError immediately after return and requires the actual LocalFree error to survive category metadata allocation/publication/retirement.

`independent_system_error_attempts()` uses two ordinary C++ threads, statically valid injected functions and separate static buffers selected by distinct message codes. Both FormatMessageA bodies must enter before either returns, using an atomic rendezvous with a five-second timeout. Each pointer and trimmed buffer must be freed exactly once; callback checks never mutate the main fixture's non-atomic check counter. Callback-visible setup is published before thread construction and changed only after joins; each thread writes a distinct result, exception slot and record. This is a regression source for independent concurrent attempts, not runtime evidence of concurrency correctness.

A third static record throws after supplying its pointer. Its Format count remains one and Free count zero. A later successful call on another record must complete normally, with no retry or cleanup of the failed record. The retained failure pointer addresses static bytes that remain valid through category teardown; the test never dereferences a destroyed stack fixture through a retained attempt. This is a controlled imported C++ exception, not a claim about native SDK EH or allocation-failure behavior.

## Policy test additions

`native_system_category_lifetime()` keeps the SDK fixture alive while constructing and then fully retiring an actual Policy/Profile fixture. A plain native-category system_error, its copied exception, runtime_error value slice and extracted error_code retain exact text/code/category after the weak Policy reference expires. A fresh code.message() then formats successfully through the same static function pair after Policy destruction. No callback captures the destroyed Policy, and the test does not invoke SDK functions after its SDK fixture ends.

New isolated `--systemraise` (expected terminal code 535) invokes the actual Profile worker binding with the installed system category and native084530 context. It catches an ordinary `std::system_error`, requires raw code77, the exact native category pointer and exact `pqcs: native stop ` text, and checks copies/slicing preserve it. One actual020950 Format/Free pair is required after the existing Profile/Policy terminal poison.

Existing isolated `--workerror` (code 531) still drives actual Policy `finish_environment_work` with work1, clear stop/wake, a failed zero-packet post and saved error77. Its expected category now explicitly matches the native `asio.system` category from the injected pair, rather than `std::system_category()`. It retains the existing consumed/counter/stop/wake/post/error-read/poison assertions and additionally requires once-formatted exact native text and copied/sliced values. The ordinary generic084530 raise remains unchanged. All new normal tests and isolated cases are called from the existing no-argument main; the isolated terminal count becomes 29.

Before the source-evidence freeze, fresh native evidence additionally confirmed084720 is `write`, with actual025720 sending all nonzero02a200 errors through system028620 and028240. The parent extended the production fix to system-category `Policy::raise_write`. Existing isolated `--write` (code515) is consequently upgraded using the same injected static SDK functions, raw code55, actual application raise_write binding and exact `write: native write ` text, including the space before native CRLF trimming. Input count and expected mutation are constructed from string lengths. It requires native category identity, one Format/Free pair and the existing terminal poison. No arbitrary generic-error ABI is invented and generic raise514 is unchanged. Round-one notes and their earlier test hash remain preserved in `test-source-notes.round1.md`.

The final expanded case `--systemraisezero` (terminal code536) reuses the actual `ProfileBindings.worker.raise` path with raw code0 and the installed native system category. It requires exact `pqcs: injectedzero text`, one Format/Free pair, unchanged zero-valued error_code, native category identity, normal system_error base catch, copy/slice preservation and terminal poison. SDK count and mutation are computed from the injected CRLF string. This specifically validates the shared helper's selected-library code0 composition: native00a270 formats a category even for code0, while this libc++ system_error constructor skips code0 formatting. The test exercises the actual Policy helper indirectly rather than treating a plain public system_error(code0,...) constructor as native-equivalent. Existing tracked stop postfalse/status0 cases still continue normally without a throw and are not repurposed as a zero-raise case. Round-two notes with hash39e36... remain preserved in `test-source-notes.round2.md`.

The source does not add an actual prepared child failure path, native exception ABI proof, formatter allocation-failure injection, unknown LocalFree cleanup or a claim of execution success. The earlier successful real prepared child last-release source is a separate stage.

## Preserved before files and final hashes

Before editing, the test files were copied normally into this stage's `before` directory. Parent metadata already recorded 55 pending sources; the additional `pending-source-status.before.json` is an exact contemporaneous copy, not a rewritten historical result.

| File | SHA256 |
| --- | --- |
| `before/native_tests.cpp.before.txt` | `419114eeda6048e0beb73b816c837d7e1a97f1a8d84f167b435593e4fe193357` |
| `before/policy_tests.cpp.before.txt` | `8fcbb082fb0adb4640e341a33006d422bd62581c9a6997ae279a8e7a13948d28` |
| `before/pending-source-status.before.json` | `a1069b5154ad87b72e8b06ac019532d03c5f854aeafbce8da54d4484c047e355` |
| current `native_tests.cpp` | `448916ae2019eaf65af3c4b097c8e11bfa8e3771cd1a515e7a38b89a35cd7e09` |
| current `policy_tests.cpp` | `8f68c316ad3b666472914e4eefc205c55bd03bb77d5069df8b85cc7c8ff0b0f1` |

Static source/diff inspection completed without a new compiler invocation. Whitespace diff checks emitted only repository line-ending conversion notices, with no whitespace findings. Independent parent review is pending for these exact candidate hashes. An explicit user rebuild instruction is required before execution validation.
