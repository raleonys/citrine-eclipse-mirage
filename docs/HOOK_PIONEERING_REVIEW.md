# Hooks of Pioneering: current registry ownership applicability

**Disposition (2026-11-02): PARTIALLY APPLICABLE / DESIGN AND EVIDENCE BLOCKED.**
This is a source-only crosswalk, not an implemented remediation, donor-parity
claim, native acceptance record, or permission to alter the production hook
topology. Reviewed against accepted `main@bb640ee0acb37a847a49bcb93a5ac0cd9b503a13`,
including the merged [allocation rollback fix in PR #98](https://github.com/raleonys/citrine-eclipse-mirage/pull/98).

## Source model and later accepted coverage

- [Registry routes](../src/platform/registry.cpp) retain three raw
  `std::set<Handle>` collections: client, language and fabricated. An opened
  native key and a locally fabricated key are both represented by a numeric
  `Handle`; the collections have no authenticated origin, generation, or
  retired-token state.
- [Native callbacks](../src/platform/hook_native_callbacks.cpp) intercept
  `RegOpenKeyExA/W`, `RegQueryValueExA/W` and `RegCloseKey`, forwarding raw
  HKEY values. The callback binding holds a lease and a registry state lock,
  but this is not proof of provenance for handles created or used through
  other Win32 APIs.
- [Current runtime effects](../src/runtime/application.cpp) use
  `::operator new(4)` and `::operator delete` for fabricated keys. The
  historical injected `HeapFree(FALSE)` result is **not** a current
  production-backend failure reproduction.
- [Registry unit tests](../tests/unit/registry/registry_tests.cpp) cover
  matched/unmatched opens, fabricated values, queries and ordinary closes.
  Those portable tests do not establish real Win32 HKEY origin, reuse/alias
  freedom, or end-to-end Consumer traversal of every Registry API.
- Accepted PR #98 makes fabricated-key creation provisional through both
  owning-set and route-set insertion. On an insertion or logging exception,
  it restores the caller output/status, removes the new ownership record
  and releases the allocation with the current non-throwing delete backend.
  [Allocation failure tests](../tests/unit/registry/allocation_failure_tests.cpp)
  exercise real container allocation failures, A/W client/language routes,
  logging failures and successful creation/close. This closes the bounded
  creation exception-safety gap without proving raw HKEY origin or retirement.
- The approved [hook lifecycle contract](HOOK_ENGINE_HARDENING.md) already
  covers MinHook activation, callback lease drain and retryable engine
  teardown. Those improvements must not be overwritten by an older
  Registry-only prototype.

## Native callback lifecycle coverage

The [native Registry fixture](../tests/unit/registry/native_lifecycle_tests.cpp)
links the production core, calls its SDK-shaped callback entries and forwards
the original-call slots to actual Windows Registry APIs. It creates isolated
volatile HKCU trees and uses the current operator-new/delete fabricated-key
backend. The fixture covers A/W native and failed-open fabrication paths,
overlay and unrelated-value queries, language routes, untracked SDK opens,
same-key close while a query pauses for startup, and callback binding leases.

The overlap scenario preserves existing behavior: a query selects its overlay
before the startup wait, another callback closes the key, and the admitted query
can finish afterward. The test checks forward progress, original `LastError`,
tracking cleanup and native values on the subsequent unrelated key. Completion
after close alone is not evidence of an ownership defect.

A bounded real-SDK close/reopen probe prints whether Windows actually reused
the numeric HKEY. `NOT_OBSERVED` is inconclusive; `OBSERVED` with clean routing
establishes only the exercised sequential transition. Neither result proves
universal stale-handle provenance, native/fabricated disjointness or alias freedom.

The focused [Windows Debug/Release workflow](../.github/workflows/msvc-registry-lifecycle.yml)
runs this fixture with the existing Registry and allocation-failure contracts;
the full native suite also includes it. Configuring these jobs is not a native
PASS: inspect the exact PR commit's execution. These tests call the production
entries directly without MinHook installation and do not observe a real
Consumer/game call graph. The design and evidence gaps below remain open.

Accepted #224/#255 also exercise the installed Application hooks through W and
A SDK routes and a controlled provider reentry. Those additions are described in
[the Registry crosswalk](RE_A11_09_REGISTRY_HKEY_BOUNDARY.md); the direct-core
fixture below is not the only accepted Registry test. Neither fixture supplies
trusted provenance for every alternate issuer or the real Consumer.

## Bounded D03 alternative Registry API evidence

The source-linked native [Registry lifecycle fixture](../tests/unit/registry/native_lifecycle_tests.cpp)
now also exercises direct SDK `RegOpenKeyA/W` and `RegCreateKeyExA/W`
issuance of real HKEY values, with callback entrypoints handling subsequent
`RegQueryValueExA/W` and `RegCloseKey`. The tests verify original SDK
value bytes and `LastError`, unchanged fabricated-key allocation counts, and
exactly one native close. Created keys and seeded values remain inside an
isolated volatile HKCU fixture and are cleaned up by the test.

This is **bounded D03 native SDK fixture coverage**, not a complete API or
Consumer import inventory. The fixture does not install MinHook; an alternate
public Registry API may internally call a hooked lower-level entrypoint in a
real host. Neither the tests nor static source reading prove Consumer
reachability, that an unhooked call bypasses all detours, or authenticated
native/fabricated provenance. [Issue #109](https://github.com/raleonys/citrine-eclipse-mirage/issues/109)
retains those unresolved D01–D04/D07 evidence gates.

## Research-to-source disposition

| Historical Hooks of Pioneering idea | Current-source assessment | Decision |
| --- | --- | --- |
| Opaque fabricated-handle provenance and generation identity (R12/R13 research) | Raw numeric membership remains the routing authority; an allocated pointer and a native HKEY are not provably disjoint by this code. | **Applicable design risk; no proven real OS alias or exploit.** |
| Atomic claim/retire across query and close | The fabricated close path erases membership before calling the release effect. There is no explicit pending-retire ledger; the existing callback lock is not an origin ledger. | **Partially applicable; design and failure semantics require proof.** |
| Historical fail-once `HeapFree` ownership recovery | The current concrete backend uses C++ allocation/deallocation, not the historical HeapFree shim. | **Do not transplant the old failure model or claim an observed current HeapFree defect.** |
| Source-level allocation/insert exception boundary | Accepted PR #98 rolls back the allocation and new owning-set entry if either set insertion or intervening logging throws; caller output/status and original exceptions are preserved with the concrete delete backend. | **Bounded creation defect fixed and regression-tested; provenance and retirement remain open.** |
| Coverage of alternate Win32 Registry routes | Only five Registry entrypoints are in the scoped eleven-hook topology. | **Coverage and Consumer provenance remain unverified; do not add hooks speculatively.** |
| Reuse of R12/R13 reference tests | Earlier tests and simulated receipts do not execute this source tree with a real Win32 Consumer. | **Research inputs only; no automatic acceptance inheritance.** |

## Remaining proof for provenance and retirement changes

1. Pin real Windows/MSVC source and host identities, observe the actual
   Consumer Registry call graph and identify which APIs can introduce,
   duplicate, query and close keys that reach these five callbacks.
2. Establish trustworthy native-versus-fabricated origin or prove
   disjointness, including numeric reuse/ABA, pre-defined keys, stale
   callbacks and concurrent lifecycle transitions. A bare set or a
   guessed high-bit namespace is not sufficient.
3. Preserve the accepted allocation/insert rollback from PR #98 while
   specifying the remaining native open, query, close and teardown
   ownership transitions. Establish exactly-once retirement and any needed
   retry for genuinely fallible cleanup effects; the current `operator delete`
   backend does not throw. Test models must not impersonate an OS failure.
4. Exercise source-linked negative and success controls for A/W original
   calls, exact pointer and `LastError` behavior, nested registry waits,
   callback locks, partial failures and cleanup. Keep the selected eleven
   hooks and the production `DllMain@1`, `SURGE@2`, `DllShutdown@3`
   ordinals unchanged absent separate owner authorization.
5. Run exact-head Source QA and portable checks, then actual Windows/MSVC
   Native Debug/Release and Production/Golden gates. Real Consumer/game
   execution and original-donor equivalence remain separate evidence and
   are not established by CI alone.

## Change boundary

This review records an actionable open research boundary, not a shipping
fix. Do not paste the historical opaque-ledger, HeapFree, or custom namespace
implementation into production. The [governance](../GOVERNANCE.md) and
[agent workflow](../AGENTS.md) remain authoritative, and the
[existing donor-gap record](DONOR_RECOVERY.md) also keeps Registry
handle provenance open. No source, public ABI, vendor, hook count,
configuration or compiled behavior changes are authorized by this document.
