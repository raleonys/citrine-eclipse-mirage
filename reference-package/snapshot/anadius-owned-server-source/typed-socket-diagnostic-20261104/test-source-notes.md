# Typed socket/service diagnostic test source

Edited only `server_tests.cpp`, `server_integration_tests.cpp` and this note. **Authored source validation, not compiled or executed.** No project build/test/link, DLL execution, child-process launch, packaging or REA query was performed by this author. Production server.hpp/cpp are owned by the implementation agent. Prior IOCP-stage documents/proofs and central status files were not edited.

## Explicit category constructor

The new public overload is `SourceError(Error,string_view,const error_category&,uintptr_t=0)`. Direct tests bind the existing module-owned native category, code10013 and an arbitrary retained address, with caller-local error/operation arguments. The local operation is changed and then destroyed; the exception must still own `typed open`, complete native what(), source code/name token and address. A typed copy, system_error base copy and runtime_error value slice must preserve the text/category after the original exception is reset. The category itself is module-owned rather than a caller-local object; the test does not manufacture a dangling borrowed category reference.

Expected text uses one separately computed default native message oracle. These tests do not intercept default SDK imports or count constructor FormatMessageA calls. Copies are checked through what(), not additional code.message() calls. Existing tests for old HOST operation/address, unknown/context-free system, misc and other categories remain unchanged, as do the prior exact84428/84528 cases. No hypothetical zero-code explicit-constructor formatting rule is asserted.

## Bounded integration extensions

One small Fault enum selects six additional retained-prefix children. The same actual OwnedServerBackend/NativeServiceGateApi/NativeIocpConstructorApi setup is reused. No generic mock framework or additional runtime authority is introduced. Normal modes leave the fault suffix empty; only an injected failure starts trace collection.

| Mode | Injected boundary/error | Actual native operation | Expected fault suffix |
| --- | --- | --- | --- |
| `--service-gate-error` | second gate false/error8 | `mutex`, address0 | no socket suffix |
| `--open-error` | create_socket invalid/error10013 | `open`, address0 | FCE |
| `--attach-error` | attach port NULL/error5 | `open`, address0 | FGCX |
| `--reuse-error` | reuse option failure/error10022 | `set_option`, address0 | FCE |
| `--bind-error` | bind failure/error10048 | `bind`, address0 | FCE |
| `--listen-error` | listen failure/error10045 | `listen`, address0 | FCE |

F is the selected synthetic SDK failure, C the recovered source category-provider call, E its WinSock-style error getter, G the separate raw Win32 error getter and X the natural temporary-socket close. Each getter snapshots the error before recording its trace. Category trace metadata preserves ambient LastError. These observations distinguish the original category-before-error WinSock path from the error-before-category/close attachment path; they do not count every metadata Win32 API invocation.

The service-gate mode allows the real first scheduler gate and real own IOCP creation, then fails only the second initializer. Actual0228e0 produces its string-boundary nonzero logicalsystem `mutex` error, with context_address0. It requires two gate attempts, one real port, two category-provider calls, WSA startup, no socket construction and no invented port/gate/WSA cleanup. The prior first-scheduler gate/84428 and failed-port/84528 children are preserved separately.

Each acceptor mode initializes the actual complete backend context and then invokes `construct_acceptor_020e20` with an initialized IPv4 endpoint, reuse=true and INT_MAX backlog. The existing synthetic socket SDK boundary injects the selected failure, but actual recovered acceptor and backend paths capture the status and raise the typed exception. Each case requires exact raw code, original source name token/logicalsystem, module native category, known native operation, address0, full owned text and typed/base/sliced copies. The expected text is one independent default-native SDK oracle per case, so it tolerates installed message language without claiming one total format call.

All acceptor cases require backend Poisoned, real context prefix (two gates, one created port, WSA startup), one additional category-provider call, no accept submission and no backend port/gate/WSA cleanup. Only attachment failure closes its temporary socket200 through actual025310; invalid creation makes no physical close, and reuse/bind/listen failures retain existing socket ownership. Bind/listen cases retain the exact sixteen-byte bound input; earlier failures have not reached bind. Nothing manually clears source ownership or fabricates teardown.

Every selected failure reaches the existing explicit ExitProcess0 retained-prefix boundary. This is not successful normal resource retirement or native EH proof. Existing normal IPv4/IPv6 and sticky-quiescence modes retain their complete successful-flow assertions. The default integration main launches all six modes using its existing hidden child harness; no such process was launched during this stage.

## Before and candidate hashes

The parent captured all four source/header files and pending metadata before edits. Its snapshot contains58 pending sources. No before file was overwritten.

| File | SHA256 |
| --- | --- |
| `before/server_tests.cpp` | `d2895d9128410aa015cdbb36e1288b716f540b718a4ca7c331290f1361b5f4f0` |
| `before/server_integration_tests.cpp` | `103e830ec37f2d052c94a7410c079146e9fd5f72cb03f1c199b97406a2fcffec` |
| `before/server.cpp` (other owner) | `db9337ccfff364b63c9dfd269822166e6d8bf04181ddf841f20543f43fa3a42d` |
| `before/server.hpp` (other owner) | `d34244ef4eb162705a9aa449a24db140f7b179121765da42f00f2bfce1293af7` |
| `before/pending-source-status.json` | `18daed02b7076870a4b21eed07e8eae5d647613980143c3be6ae6db6fdb4227a` |
| current `server_tests.cpp` | `0905773c98d744cd47dd3b47a740d14038b82e5e2de0b97399cba5c421e9dcfb` |
| current `server_integration_tests.cpp` | `73679f010f505b6da45412f52121c84a7484fd6cb2dd5878635fcbcfbc590268` |

Static source/diff/whitespace inspection found no whitespace defect. Parent independent review is pending at these exact candidate hashes. Existing NULL-port/error0 socket-constructor coverage remains unchanged, and no new category rule for unknown, zero or misc actual string effects is claimed. An explicit user rebuild/test instruction remains required before runtime validation.
