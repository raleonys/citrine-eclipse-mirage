# IOCP constructor diagnostic test source

Edited only `outputs/anadius-owned-server-source/server_tests.cpp`, `server_integration_tests.cpp` and this note. **Source authored and statically inspected; not compiled or executed.** No build, project tests, linker, reconstructed/original DLL, process launch, ZIP update or REA session was run by this test author. Parent-owned before copies and pending metadata were already captured before the edits. Production server.cpp belongs to the separate implementation agent.

## Source and native evidence boundary

Read the stage's fresh `native-findings.md` and the current source/API. Native024990 uses distinct measured contexts084428=`mutex` and084528=`iocp`, both with system028620. Both raises require a nonzero cached status. Source production now maps those two Construction addresses to their measured operation strings and selects the module native category only for logicalsystem plus one of those exact addresses. SourceError itself still owns the caller-supplied operation rather than replacing it from the address. Unknown/context-free and non-system projections keep their existing HOST providers.

The preexisting socket `iocp_constructor_tests.cpp` NULL-handle/error0 case remains unchanged. These test additions do not invent a throw for that condition, extend a hypothetical public-ctor zero formatting path, or claim native exception ABI/EH equivalence.

## Direct SourceError tests

`typed_source_error_values()` replaces the obsolete compiler-system/guessed-IOCP expectations with two nonzero selected cases: raw INT_MIN at084428 with owned `mutex`, and code5 at084528 with owned `iocp`. It verifies:

- Exact signed raw value, module native category pointer/name `asio.system`, original opaque source name token/logical category and source value.
- Complete `operation + ": " + native.message(code)` text, exact retained numeric context and operation ownership after the input string changes.
- SourceError copy metadata/text, ordinary system_error base copy, runtime_error value slice and throw/catch through system_error while typed metadata remains discoverable.
- A caller-owned arbitrary operation at a selected address is preserved rather than rewritten by SourceError.
- Unknown-address logicalsystem, no-address operation and context-free logicalsystem retain compiler system category and their existing complete/no-context text behavior. Other logical category at a known address still uses compiler system; misc at a known address still uses the concrete misc category/message.

The default native message oracle is computed once per selected case before constructing SourceError. This is a separate actual provider call in the future test, so the test **does not claim to count one total FormatMessageA call**. It does not call code.message() again while checking copies. Obtaining the expected text from the same selected default native provider tolerates installed SDK language/message text rather than hardcoding English Windows descriptions. Copies and fullwhat are compared as owned values.

## Actual construction integration paths

Existing isolated `--constructor-error` retains its actual OwnedServerBackend.initialize_context0241b0→construct_iocp024990 path, but now expects native category, `iocp`,084528, exact complete provider-derived text and retained copies/base slice. Rejected CreateIoCompletionPort now validates its actual SDK arguments and records one attempted creation before returning NULL/error5.

A bounded new `--gate-error` reuses the same existing setup. Its injected first gate initializer validates section/spin0x80000000, records one attempt and returns false/error8 without physically initializing that gate. Actual027be0 captures error8, actual024990 obtains system category, and the Construction raise must produce native `mutex`/084428. The fixture does not call the private Construction method directly.

Both isolated modes require backend Poisoned, WSA startup once, gate initialization attempted once, no actual created/closed port, no gate deletion and no WSA cleanup. Gate failure requires zero port attempts and one source category lookup. Port failure requires one port attempt and two source category lookups (the successful gate's category lookup still occurs). These counters are passive observations of actual source order. The expected whole message is computed once before initialization using the default native category; constructor SDK calls are not intercepted or counted.

Each failing child reaches its existing explicit ExitProcess0 retained-prefix boundary. It does not unwind the unknown/incomplete backend state, fabricate gate/WSA cleanup or claim normal completion. Existing normal IPv4/IPv6 and sticky-quiescence flows retain their assertions. The no-argument integration main includes the new gate child after the existing retained children, and the existing hidden CreateProcess harness requires the selected process exit status. No process was launched during this source-only stage.

## Frozen source hashes

| File | SHA256 |
| --- | --- |
| `before/server_tests.cpp` | `eef73cfe91cdf11af2325db9fdd201fbfbab6119230836880aea76451dc3d705` |
| `before/server_integration_tests.cpp` | `bc8933a2d89e948e1229f7f2bc86a72400b9d9ef0f6a3fd2207d4b623055831f` |
| `before/server.cpp` (separate production owner) | `9516bed4855e3d144f4617ec73e44cf6116d2912b8e757646210ccfbd6fe5300` |
| `before/pending-source-status.json` (55 sources) | `1413abd4e37597f58eff1da220c2149eb5e0be34a88a9c3c2a2a133544cb1147` |
| current `server_tests.cpp` | `d2895d9128410aa015cdbb36e1288b716f540b718a4ca7c331290f1361b5f4f0` |
| current `server_integration_tests.cpp` | `103e830ec37f2d052c94a7410c079146e9fd5f72cb03f1c199b97406a2fcffec` |

Static source/diff/whitespace inspection found no whitespace defect. Parent independently reviewed these exact test hashes and reported no blocker; `root-test-review.md` records the gate/category-count bounds. Both test files are frozen. The required future explicit rebuild/test request remains outstanding; no test PASS or runtime/native compatibility conclusion follows from these authored assertions.
