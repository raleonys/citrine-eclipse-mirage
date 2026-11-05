# Detailed package comparisons

These partial inspections are not completed file or module reviews. Exact source
identities and state are in [the file ledger](RE_PACKAGE_FILE_REVIEWS.json).

Archive 12 intake: all three donor files discussed below changed. The importer
reset their current rows to `pending` and preserved these partial Archive 11
reviews in `prior_reviews`. The following comparison remains historical;
`RPK-0001` is open and requires rechecking against the new donor hashes. See
[the delivery update](RE_ARCHIVE12_UPDATE.md).

## Archive 12 preliminary recheck: RPK-0001 and startup publication

**Reviewed inputs (2026-11-05).** Archive 12 changed the three files behind the historical RPK-0001 comparison. This is a bounded source inspection, not a completed per-file ledger review or a native acceptance test.

| Input | Pinned SHA-256 |
| --- | --- |
| Donor `anadius-execution-source/execution.hpp` | `eb6c3dd0fd279ee9aecafef33c60ba184ab424b3e003143b5271f91ce818ab55` |
| Donor `anadius-execution-source/execution.cpp` | `c9a30ba892a526b9b00068d7e8d15927a60e2ecf7eda8d23a02bbfa2576f6884` |
| Donor `anadius-startup-publication-source/publication_tests.cpp` | `7af4e858d48fc91b64989dde02db2fa4cc67de25995cf4c9681e435c78dccd4b` |
| Product `src/runtime/execution.cpp` | `9f3d96c4b58c813f7fafad1d25d5c5ebaa3f2a8f36c28af6935fbcb20ee4e1bf` |
| Product `tests/unit/execution/concurrency_tests.cpp` | `24ff1f6d64c12cfe0d205656ad8571f092517b78f8a2fd00f7ac336cf2ecfff0` |
| Product `src/runtime/application.cpp` | `d34aa3f8f464b5ee3ce580dbbdbf7d3f9c348c12933989d2f8fccb4b32df926a` |

Product identities were read from accepted `main@183a253d08323cb4eb1ed37a3ee38f00fd596a4b`. Donor identities match the registered Archive 12 delivery. No donor code was executed or imported.

**Clock contract.** The updated donor wraps `now_nanoseconds_036740` in a private recursive source gate, so its eight-reader/800-sample test expects exactly one cold `QueryPerformanceFrequency` call. The active product uses `std::atomic_ref` compare-exchange for cache publication; its deliberately synchronized two-reader test expects two calls and a correct common value. The product Application constructor initializes and validates its frequency before launching its provider and extra-library workers. A difference in query count therefore does not demonstrate a shipping startup bug. The earlier product-side crosswalk is [PR #136](https://github.com/raleonys/citrine-eclipse-mirage/pull/136), which must not be duplicated as a speculative implementation fix.

**New donor publication composition.** Archive 12 retains an optional `Publication` observer, `ReadyStage` values, `finish_server_startup` bound-port publishing, identity/flag checks and negative cases for preseeded or duplicate flags, foreign issuers, native-unlock failure and event-before-provider readiness. The active product does not offer that observer API. Instead it holds session admission closed through worker joins and profile publication and calls `AsioService::resume_accepts()` after its existing ready flags, through its own Asio-backed ownership model. Missing an identically named donor API is **not** evidence of a missing externally visible function. [PR #128](https://github.com/raleonys/citrine-eclipse-mirage/pull/128) already addresses a distinct worker-publication scenario.

**Error and lifetime boundaries.** The donor's `SourceGuard` preserves Win32 `LastError` across its additional lock bookkeeping and releases that gate before sleeping/observer callbacks; product lock/QPC operations differ and have no matching source gate. A native, product-specific reproducer is required before asserting an error-preservation defect. The donor's full event ledger is not itself evidence of real socket admission, completed worker teardown, hook safety or provider lifetime; the active application's true acceptance boundary must be tested separately.

**Disposition.** RPK-0001 remains an observed source-contract difference, **not a confirmed product defect**. Review remaining product callers, externally constructed `Execution` instances, QPC failure/error preservation and any host requirement for exactly-once sampling. Compare concrete event/admission/failure cases against actual application tests before creating an implementation PR. The three changed Archive 12 file-review rows remain pending detailed review. Supplied donor PASS counts were not rerun; native Windows/MSVC, Golden/original DLL and real Consumer/game acceptance are not established by this comparison.

## RPK-0001: concurrent cold frequency initialization differs

Classification: observed contract difference; product defect not established.
Status: open. Product baseline: `d9c860d`; donor: archive 11.

Inspected donor sections:

- `anadius-execution-source/execution.hpp` adds a private recursive source gate,
  prohibits copy/move and excludes concurrent direct-helper access or another
  wrapper sharing the same state.
- `anadius-execution-source/execution.cpp`, `now_nanoseconds_036740`, holds that
  gate across frequency initialization and counter sampling. The cache is a
  plain integer protected by that gate.
- `anadius-startup-publication-source/publication_tests.cpp`, `waiter_and_clock`,
  starts eight threads sampling 100 times each. Its assertion requires exactly
  one frequency query, 800 counter queries and nonnegative samples.

Current product counterpart:

- `include/preloader_l/runtime/execution.hpp` has no matching source gate.
- `src/runtime/execution.cpp` atomically publishes the cache with `atomic_ref`
  and compare-exchange. Two cold callers can both query frequency before one
  wins publication; counter sampling has no corresponding wrapper serialization.
- `tests/unit/execution/concurrency_tests.cpp`, `cold_frequency_cache`, deliberately
  synchronizes both frequency queries before publication and requires **two**
  queries, correct converted samples/cache and no later repeated query.

Directly copying the donor's one-query assertion would conflict with a deliberate
current test contract. Existing product coverage does not prove equivalence to
the donor's serialization or publication observer; the donor's 800-sample test
does not independently exercise the product's cold-cache publication race.

Remaining review: establish whether exactly-once frequency acquisition is a
product requirement; inspect native error preservation and every cache/platform
sharing caller; assess failure, locking and performance effects; reproduce any
claimed defect before a focused PR. This partial comparison alone justifies no
production change. The three donor rows remain `in_review`.
