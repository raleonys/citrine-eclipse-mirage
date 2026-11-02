# A11-06: staged shutdown ownership and retry review

**SOURCE_REVIEW_ONLY / NO_DEFECT_CONFIRMED / CROSS_STAGE_NATIVE_PROOF_OPEN**

This review follows [Archive 11 A11-06](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/docs/RE_ARCHIVE11_REVIEW.md) in the long-lived Reverse Engineering research branch. It maps **current product source**, not full donor parity. Archive identity is independently unresolved in [PR #115](https://github.com/raleonys/citrine-eclipse-mirage/pull/115).

## Source-ordered lifetime ledger

A failed cleanup stage can stop later stages on that attempt. Individual test PASS results do not combine into a native interstage fault proof.

| Stage | Current code and owner | Failure / retry implication |
| --- | --- | --- |
| 0. Public shutdown | [DLL entry worker](../src/platform/dll_entry.cpp) owns a single Application and distinct queued shutdown requests. | If Application::stop throws, the worker reports incomplete cleanup and **does not reset its owner**. A later explicit request can retry. |
| 1. Admission and workers | [Application::Impl::stop](../src/runtime/application.cpp) seals accepting, aborts incomplete startup flags and joins provider/extra workers. | A failed startup is not successful publication, and new handlers must not be admitted during stop. |
| 2. Network service | Application runs `service->stop(); service.reset();` through [AsioService::Impl::stop](../src/net/asio_service.cpp). | The service pointer is reset only on success. Pending accepts, clients, I/O or deferred native closes after a bounded drain cause a retained-owner retry error. |
| 3. File recovery | `files.recover_atomic()` follows service retirement and precedes hook cleanup. | An exception prevents later hook/extra/provider stages on that attempt; the DLL worker retains Application. |
| 4. Hooks | Application calls `hooks->quiesce(100ms)` then `hooks.reset()`. | Failed quiescence throws **before** releasing hooks and callback/image dependencies. |
| 5. Extra DLLs | `ExtraLibraries::close()` iterates module leases. | Successful FreeLibrary slots become zero; failed slots remain pending. Repeated stop must not double-release successful modules. |
| 6. Provider | Application calls `Provider::close()` and [online::cleanup_0366f0](../src/runtime/online.cpp). | Destroyed object is cleared before module FreeLibrary; a failed library release retains the module handle. Arbitrary foreign-origin callbacks are not proven quiescent. |
| 7. Completion | `started=false` and public Application-owner reset follow successful prior steps. | A partial stop is not DLL shutdown success. |

## Already covered, and overlap to exclude

[Public shutdown failure tests](../tests/native/application_shutdown_failure_tests.cpp) exercise queued retries and retained hook/Application ownership.
[Extra-library failure tests](../tests/native/extra_library_failure_tests.cpp) exercise allocation and module-release failures, with stage-local retained leases.
Do not duplicate open [#104](https://github.com/raleonys/citrine-eclipse-mirage/pull/104) (read/write ownership),
[#107](https://github.com/raleonys/citrine-eclipse-mirage/pull/107) (late socket completion),
[#102](https://github.com/raleonys/citrine-eclipse-mirage/pull/102) (paused startup),
[#101](https://github.com/raleonys/citrine-eclipse-mirage/pull/101) (provider getter),
[#105](https://github.com/raleonys/citrine-eclipse-mirage/pull/105) (factory throw), or
[#106](https://github.com/raleonys/citrine-eclipse-mirage/pull/106) (post-publication greeting throw).
No running/pending test or acceptance conclusion from these PRs transfers here.

## Open test design: A11-06-X1

Before another code PR, inventory available test-only injection seams and
determine whether a **source-included Windows native** fixture can induce two
different cleanup failures in consecutive stages over successive explicit
shutdown calls, such as atomic-file rollback followed by hook or extra-DLL
release. Check that later stages do not run after an earlier failure, already
completed owners are never freed twice, the exact live owner is retained after
each fault, and new sessions are rejected after stopping begins. Require final
idempotent success, actual MSVC Debug/Release, and an assertion-sensitive
negative control.

This is **not** an established shipping bug or completed native test. Do not
import donor factories/CRT, add public exports or alter shutdown budgets merely
to obtain a passing fixture. A product correction requires separate reproduced
failure evidence and explicit review. No runtime, vendor, game or release
acceptance is claimed in this source-review document.
