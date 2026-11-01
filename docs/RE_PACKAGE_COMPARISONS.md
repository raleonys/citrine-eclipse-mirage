# Detailed package comparisons

These partial inspections are not completed file or module reviews. Exact source
identities and state are in [the file ledger](RE_PACKAGE_FILE_REVIEWS.json).

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
