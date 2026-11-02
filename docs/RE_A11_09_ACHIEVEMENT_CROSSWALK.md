# Archive 11 A11-09: achievement storage/runtime/writer source crosswalk

**SELECTED_MODULES_INITIAL_CROSSWALK / NO_NEW_DEFECT_CONFIRMED / FULL_ARCHIVE_REVIEW_OPEN**

**Accepted comparison base:** `main@4d2d083786283bca2f8d53e79e7a99aae364464d`. The retained eight-part [Archive 11 intake](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/docs/RE_ARCHIVE11_REVIEW.md) has combined SHA-256 `db394434e38c7edad33607a846ffdc36b04a1e5978d2fbe3d74da24bda5179da`. The long-running [research package](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/docs/RE_PACKAGE_PROJECT.md) owns the detailed file and module ledger; **this review does not change its status to completed**. The historical differently hashed donor discussed in [PR #115](https://github.com/raleonys/citrine-eclipse-mirage/pull/115) must not be substituted.

## Exact source identities

| Archive 11 source | Product counterpart at comparison base |
| --- | --- |
| [achievement storage](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-achievement-storage-source/storage.cpp), SHA-256 `2a2f23e278f35e8920047c2a9444e1fd150220d01bb2d727d1f3378a43684d13` | [achievement_storage.cpp](../src/runtime/achievement_storage.cpp), SHA-256 `f85ca5baa949c74c3b1a9d5380c3cdef465d6bd2bf25dff1d76d3f9e049d845e` |
| [INI writer](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-achievement-writer-source/writer.cpp), SHA-256 `20bd58a3fb48d3db046725fa6c154da475072c9dbd29f21e7c6a34c617319b87` | [achievement_writer.cpp](../src/runtime/achievement_writer.cpp), SHA-256 `d4018b757421f9638861a4ac889f4a2393eb79d689860ba26710b8b2f2c9a605` |
| [grant persistence adapter](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-achievement-runtime-source/runtime.cpp), SHA-256 `5ec1e7c2fc2dced97d781450ad15058bc9cc2c9f5486d7ff3c35fe81bfd9e2b7` | [achievement_runtime.cpp](../src/runtime/achievement_runtime.cpp), SHA-256 `3897f8c066586682d1f022ac561c60c94c26581305b7f693785312dbd441ac6c` |

The hashes identify **source bytes**, not equivalence of DLLs, private CRT behavior, ABI, locale, native OS calls or original game behavior. The three donor files were compared as source, not executed.

## Behavior and existing regression coverage

| Boundary | Source comparison | Current regression and limit |
| --- | --- | --- |
| Achievement XML path and ingestion | Both preserve an existing XML path when `achievements_set` becomes empty, parse the C-string portion, accept arbitrary root/child tag names, ignore missing Id while accepting an explicit empty Id, and process duplicate records in order | [storage_tests.cpp](../tests/unit/achievement_storage/storage_tests.cpp) exercises stale paths, malformed/root-only XML, missing/empty/duplicate Id and Name/Grant policies with controlled XML effects |
| Numeric interpretation and missing values | Both select `%x` for a `0x` prefix after non-high-bit whitespace and otherwise use signed `%d`; scanner receives the original attribute, with no `%u` substitution | The same storage suite tests negative/signed limits, hex, default0, string suffix and C-string NUL; this does not prove the original private CRT's undefined overflow/locale/invalid-memory behavior |
| Steam watcher/epoch gate | Both derive the watcher path in filename/app/directory widening order; iterate all map records in key order for epoch conversion; perform status(path,3); preserve existing regular-file kind2 and the observed error-kind mask | The storage suite covers status kinds, partial epoch failure and actual generated XML/INI files. It does not establish every native filesystem/reparse/failure interleaving |
| INI byte representation | Both serialize map-key order with `steam_id` section names, unescaped explicit-length bytes, CRLF, signed Achieved and Progress, unsigned UnlockTime, and progress fields only when signed total > 1 | [writer_tests.cpp](../tests/unit/achievement_writer/writer_tests.cpp) pins byte-for-byte cases including duplicate sections, embedded NUL, negative and limit values |
| **INI write and failure contract — intentional product difference** | The donor `write_ini_01f9a0` calls `write_binary_03c1d0` and **ignores its boolean result**; current product routes through `file::Factory::replace_atomic` with a checked failure contract | The writer suite checks directory-first ordering and exception propagation. [atomic_persistence_tests.cpp](../tests/native/atomic_persistence_tests.cpp) exercises real Windows replacement/rollback and retained staging; do **not** copy donor's ignored-write-result semantics into the product |
| Grant save and state/event ownership | Both use the same State map and storage paths, UTC timestamp helper and output XML printer; product's `persist_achievement` batches XML and INI atomically rather than issuing independent unchecked writes | [runtime_tests.cpp](../tests/unit/achievement_runtime/runtime_tests.cpp) tests owned XML/INI output, completion/skip, failure rollback, selected event ordering and round-trip parsing. This is not a general cross-process/power-loss transaction or Consumer-game proof |

## Triage decisions and next gate

1. **No standalone bugfix from matching behavior.** XML parsing and INI byte rules already have product source/tests. Opening another PR to reassert them would duplicate accepted work.
2. **Preserve the deliberate checked/atomic persistence policy.** The donor's ignored write return is an observed difference, **not** evidence that the safer product path is wrong. A proposed change requires an exact native reproducer against the current atomic file owner and explicit owner policy.
3. **Open evidence boundaries:** actual malformed XML/CRT locale parity, native OS error races outside existing fixtures, and a named Consumer session exercising live file recovery during shutdown. These are hypotheses; none is a confirmed production bug. An individual test PR requires a new reachable scheduling condition or assertion that existing [atomic persistence](../tests/native/atomic_persistence_tests.cpp) and [shutdown](../tests/native/application_shutdown_failure_tests.cpp) fixtures do not already cover.
4. **Archive status:** this is an initial three-source-family crosswalk only, not all files in those modules, not the remaining ~200 module groups, and not A11-09 completion. Continue file-level tracking in the retained master research workstream.

No runtime or test implementation, vendor source, public ABI, CMake/CI, Golden policy or release artifact is changed by this document. Full exact-head Source QA, Windows/MSVC native Debug/Release, original donor, and Consumer/game acceptance were **not** executed in this source-review session. Owner review controls integration.
