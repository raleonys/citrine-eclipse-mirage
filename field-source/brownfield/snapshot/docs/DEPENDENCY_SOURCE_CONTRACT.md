> **HF12 current static CRT supersession, 2026-10-13:** Shipping `/MT`, Debug `/MTd`, pending exact Windows acceptance. Historical `/MD` or `/MDd` statements below are lineage only. See `docs/CRT_STATIC_POLICY.md`.

# HF12 third-party dependency contract (intermediate; not a release)

## Authority and scope

This addendum is active **only in the cumulative HF12 internal source state** descended from the immutable HF12 P2 ZIP. It does not modify the canonical HF10 release, promote HF11 Candidate13, authorize release publication, or supersede `docs/PROJECT_GOVERNANCE_CONTRACT.md` (the separately hash-bound governance mirror). Its machine-readable authority is `metadata/DEPENDENCY_SOURCE_CONTRACT.json`; enforcement is integrated into existing `scripts/check_repository_contract.py` through the existing `scripts/test_vendor_sources.py --contract-only` path. No new top-level QA script or additional FALCION script is permitted by this work.

## Exact pinned dependencies and graph distinction

| Library | Git commit | License | In-source usage | Native status |
|---|---|---|---|---|
| TAO PEGTL (upstream main; 4.0.0 header) | `3b075432e94f6f906425350b43d03cd9cebfa4f1` | BSL-1.0 | Shipping-linked Config2 shared PEGTL grammar, with separate settings/service projections | Windows/MSVC not run |
| gulrak/filesystem (1.5.17 WIP) | `3812f8a6f31ed67d32272ac33fe2b85c6b2c947e` | MIT | Shipping path composition for `surge.cfg`/`surge.png`; Win32 open/error handling retained | Windows UTF-16, UNC, long path not run |
| Standalone Asio (1.38.2) | `8806a6803cde7054c3049d3666d3ec36786568c5` | BSL-1.0 | Vendored **test-only**, not in shipping socket backend | Windows native and real LSX tests not run |

The three upstream trees have no local patch/fork. Each shipped vendored file is locked by exact SHA-256 per-file inventory in the machine contract and was independently matched byte-for-byte to the sealed upstream-archive members. The upstream ZIP comment contains the exact commit identifier, but **archive integrity is not Git object verification**. `UPSTREAM_COMMIT.txt`, exact licenses, CMake pin checks, production/test-only usage and the file inventory must remain consistent. Offline builds only; no opportunistic downloads, silent upgrades, or heuristic tag substitutions.

## Build, ABI, and semantic acceptance

- HF12 targets declare C++20 in CMake. Portable GCC/Clang tests are not Windows/MSVC verification. `/std:c++20`, `/GR-`, `/EHsc`, `/MD` and `/MDd`, CRT linkage/imports, and 548-byte model ABI must be checked on Windows before release.
- PEGTL is grammar tooling, not an Anadius VDF `Grammar` supplier. Both independent typed consumers, duplicate-key behavior, accepted/rejected malformed input, rollback, BOM, escape handling and no-partial-publish semantics need native acceptance. Do not silently ship an unapproved donor duplicate-object replacement policy.
- filesystem is intentionally path-only. `CreateFileA/W`, `ReadFile`, module-relative ownership, return/error mapping, Unicode/UNC/long paths, CWD preservation and exception behavior cannot be discarded without native differential evidence.
- Asio is **not** a shipping I/O replacement yet. Modern direct Asio is the target. Do **not** add a custom socket ownership layer, dispatcher or bespoke wrapper without concrete deficiency evidence and the user's explicit **prior** authorization. Cancellation, native socket release, failed close ownership, worker quiescence, DLL unload, protocol framing and real game callbacks must be separately proven.
- Anadius is reference evidence only, never an import or runtime dependency. Third-party import/CRT/loader-lock rules and no second socket backend apply without waiver.

## Finding / risk / update governance

The current in-tree tracking mirror is `metadata/THIRD_PARTY_KNOWN_VULNERABILITIES.json` (plus `docs/THIRD_PARTY_KNOWN_VULNERABILITIES.md`). PEGTL, filesystem and Asio are included with explicit linked-vs-test-only scope; an empty `issues` list means **no verified finding recorded**, NOT vulnerability-free or an upstream advisory scan. The filesystem WIP flag is a nonsecurity engineering risk unless substantiated otherwise. For each new upstream issue log: affected pinned commit, source/case, reproducibility, severity, confidence, impact, mitigation, owner, and next review trigger. New vendor source bytes/commits require explicit user approval, independent provenance recheck, QA, and affected native/game acceptance.

## Acceptance and publication boundary

Required before any full HF12 status: 8/8 working and 8/8 fresh-extract source QA, exact manifest, license/pin/hash/tamper negatives, both Windows CRT variants and `/GR-` native build, Native/CRT/Golden, filesystem Unicode and status matrix, Asio shutdown/cancellation/failed-close checks, LSX handshake and real CollegeFB27/Madden27 callback acceptance. Until then **RESEARCH_INTERMEDIATE_NOT_RELEASE**. No canonical pointer, shipping release identity, or unapproved ABI/route policy modification.
