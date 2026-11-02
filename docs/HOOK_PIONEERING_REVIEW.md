# Hooks of Pioneering: current registry ownership applicability

**Disposition (2026-11-02): PARTIALLY APPLICABLE / DESIGN AND EVIDENCE BLOCKED.**
This is a source-only crosswalk, not an implemented remediation, donor-parity
claim, native acceptance record, or permission to alter the production hook
topology. Reviewed against accepted `main@7e94131e5a5692bf349a77993180d64a7ace2408`.

## Current authoritative implementation

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
  They do not establish real Win32 HKEY origin, reuse/alias freedom, or
  end-to-end Consumer traversal of every Registry API.
- The approved [hook lifecycle contract](HOOK_ENGINE_HARDENING.md) already
  covers MinHook activation, callback lease drain and retryable engine
  teardown. Those improvements must not be overwritten by an older
  Registry-only prototype.

## Research-to-source disposition

| Historical Hooks of Pioneering idea | Current-source assessment | Decision |
| --- | --- | --- |
| Opaque fabricated-handle provenance and generation identity (R12/R13 research) | Raw numeric membership remains the routing authority; an allocated pointer and a native HKEY are not provably disjoint by this code. | **Applicable design risk; no proven real OS alias or exploit.** |
| Atomic claim/retire across query and close | The fabricated close path erases membership before calling the release effect. There is no explicit pending-retire ledger; the existing callback lock is not an origin ledger. | **Partially applicable; design and failure semantics require proof.** |
| Historical fail-once `HeapFree` ownership recovery | The current concrete backend uses C++ allocation/deallocation, not the historical HeapFree shim. | **Do not transplant the old failure model or claim an observed current HeapFree defect.** |
| Source-level allocation/insert exception boundary | `fabricate_010170` allocates before inserting the identity into the fabricated collection. A failed collection insertion can leave ownership unrecorded; native callback exceptions are terminal under the current policy. | **New bounded review candidate; not a native fault receipt.** |
| Coverage of alternate Win32 Registry routes | Only five Registry entrypoints are in the scoped eleven-hook topology. | **Coverage and Consumer provenance remain unverified; do not add hooks speculatively.** |
| Reuse of R12/R13 reference tests | Earlier tests and simulated receipts do not execute this source tree with a real Win32 Consumer. | **Research inputs only; no automatic acceptance inheritance.** |

## Required proof before a production remediation PR

1. Pin real Windows/MSVC source and host identities, observe the actual
   Consumer Registry call graph and identify which APIs can introduce,
   duplicate, query and close keys that reach these five callbacks.
2. Establish trustworthy native-versus-fabricated origin or prove
   disjointness, including numeric reuse/ABA, pre-defined keys, stale
   callbacks and concurrent lifecycle transitions. A bare set or a
   guessed high-bit namespace is not sufficient.
3. Design one coherent ownership protocol for create, successful and
   unsuccessful native open, fabricated allocation, query, close and
   teardown. Specify allocation/insert rollback, exactly-once retirement,
   cleanup retry and the semantics of the actual `operator delete`
   backend; test models must not impersonate an OS failure.
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
