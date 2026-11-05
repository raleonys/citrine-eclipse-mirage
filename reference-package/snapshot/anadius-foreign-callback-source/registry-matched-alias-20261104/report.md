# Matched registry nesting and aliased outputs — 2026-11-04

Fresh native/source comparison found no new production ordering defect in the selected paths. Two existing test sources now cover matched handle changes and aliased query outputs that the previous untracked-key nesting chain did not cover. These are uncompiled, unexecuted test sources; no production rewrite or whole-DLL completion credit is claimed.

## Fresh native evidence

REA6.8.0 server identity and original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` were verified. Ghidra12.1.4 provided eight complete fresh records, retained individually and in `rea-evidence-bundle.json`; session closed after export. Same content-addressed Evidence IDs may recur across fresh calls. No target execution.

| Operation | A | W |
| --- | --- | --- |
| Restore saved pre-SDK capacity through actual size pointer | 0x1800091dd | 0x1800093d4 |
| Reread caller size after resolve/conversion | 0x1800091fd | 0x1800093f4 |
| Write required byte size | 0x180009208 | 0x180009402 |
| Secure string copy | 0x1800092ce -> 05cff0 | 0x180009429 -> 05cdcc |
| Writer return followed by current type-zero check/write | 0x1800097ab / 97b7 / 97bc | 0x180009adb / 9ae7 / 9aec |

Fabrication helper010170 allocates four bytes at0x18001019e, writes the output handle at0x1800101a6, tracks the allocated identity, logs and finally clears status at0x180010283. Retained earlier native open/close/wrapper records supplement this stage for their original-before-match and post-SDK membership lookup order; they are not relabeled as new queries here.

Provider-reported function boundaries and terminal/no-return limitations remain part of each full Evidence record. Pseudocode is not original C++ or a CRT/EH ABI proof.

## Alias oracle

On the selected little-endian Windows target, let lpData, lpType and lpcbData denote the same DWORD, initial capacity4 and replacement text empty. The writer restores4, stores required size1(A) or2(W), and copies NUL over the low1/2 bytes. The complete DWORD becomes0. The subsequent current type check then writes REG_SZ=1 into that same DWORD, including the caller's data bytes. Therefore final bytes are `01 00 00 00`, despite the empty string source.

Without a data copy, the shared nonzero required size remains1(A)/2(W). With incoming capacity0 and data present, no copy occurs, result is234 and required size remains1/2. Current bytewise ScalarCell accesses and post-writer type reload already preserve this order. Caching type or deferring size writeback would change the native result.

`anadius-registry-source/registry_tests.cpp` adds these A/W cases using raw byte storage and the existing source core/effects boundary. It checks untouched trailing bytes too. This is a direct source-level synthetic scenario, not an observed Windows registry result.

## Matched SDK descendants

`anadius-foreign-callback-source/foreign_integration_tests.cpp` adds a separate bounded fixture using actual native_context::Owner, concrete Policy, Publication and Domain scopes with synthetic typed originals. It positively covers matched OpenW, which was absent from the prior typed nesting chain:

- A recursive OpenW reuses the exact PHKEY cell. The child replaces the parent's tentative old handle; final tracking contains only the child's final handle.
- A tracked query invokes nested real close from its SDK call. After return the outer lookup observes removed membership and keeps the original high-bit status and caller output fields.
- An untracked query invokes nested matched OpenW. The outer lookup observes new language membership; the writer restores original capacity16 over a nested size1 and writes the six-byte en_US value.

The Language path avoids inventing a client wait completion. The fixture checks per-depth Context leases/PTD identity and closes its owned synthetic handle routes normally. It does not cover arbitrary shared-set concurrency, failed fabricated allocation, every output alias combination or actual Windows reentry. No extra readiness or unload protocol was added.

## Review and status

One subagent independently audited the core and implemented matched integration source; root added byte-alias unit source; another subagent statically reviewed both against fresh Evidence. Exact pre-edit sources, implementation notes and final source hashes are retained. No compile, project test run, link, DLL execution or ZIP update occurred. Production code is unchanged in this stage.

Ten canonical source/header files now differ from the last real full build: the earlier nine plus registry_tests.cpp. This count measures pending files, not DLL completion. Existing delivered DLL/ZIP and frozen build/link/package proofs remain unchanged. The newly authorized continuous REA/Ghidra/subagent goal preserves the explicit rebuild requirement.
