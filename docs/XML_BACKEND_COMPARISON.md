# Pugixml compatibility diagnostic

PR #149 now selects pugixml for the production XML reader and development DLL.
The owner requested completing the migration in this open PR before merging.
The compatibility diagnostic compiles the actual production reader sources;
there is no separate candidate implementation to drift from shipping code.
The public reader interface, DLL ABI, standard CRT and three other vendor pins
remain unchanged. Native application and Golden results must belong to this
production migration commit; earlier test-only passes do not establish them.

## Inputs and independent link graphs

The control is unmerged PR #108 commit
`a34a0536f0980b38d77d029f11fa3720dcaedc6e`, checked out separately with no tracked
source modifications. Its actual reader, immutable 12,087-input corpus and
original test assertions remain the reference. The baseline includes #108's
20 explicitly reviewed malformed-value corrections; it does not preserve the
older numeric-overflow or unknown-entity corruption. Neither historical fixture
is regenerated. A pin review is required when the reference changes or merges.

The owner-uploaded `pugixml-master.zip` has SHA-256
`4cd80a72fde87ce1f23eae4ed048446b2e887fc25437793e525367448be2cf81`.
Its core matches official pugixml commit
`f0a3faa76e04f35110257ca8713363dee3c890e8` (version macro 1160). The runner
verifies its three source/header files and MIT license separately. Full installed
member verification pins all 191 files in `third_party/pugixml`. The runner
requires that installed tree and performs no download. Neither vendor source is
patched. XPath is disabled for production and diagnostic pugixml targets.

The standalone `tests/diagnostics/xml_backend/CMakeLists.txt` compiles exactly
`src/xml/reader.cpp`, `reader_policy.cpp` and `reader_values.cpp` from this checkout.
The reference uses the pinned control's first-party reader with a namespace-only
binding change to avoid an ODR collision. Both preserve the checked value-decoder
logic from PR #108. The candidate does not link the reference reader or TinyXML2. CMake File API traversal verifies this for
every candidate-only executable/configuration and requires a real pugixml
dependency; `candidate-linkage.json` records the evaluated transitive graph.
The reference is linked only into separate control and differential targets.

## Compatibility policy

`src/xml/reader_policy.cpp` owns the project's syntax and error decisions. It uses an
iterative scanner with a small inline stack, limits entered child lists to the
reference depth boundary, and detects malformed text, attributes, duplicate
attributes, mismatched elements, declarations, comments, CDATA and unknown
nodes. It preserves the first-NUL and borrowed-view boundaries, byte-based name
rules, leading-whitespace/BOM handling, error priority, LF-based diagnostic
line counting and bounded full diagnostic text.

After validation, ordinary input is parsed by pugixml. Legacy accepted spellings
outside its grammar are constructed with the same scanner and pugixml's public
DOM API. This is explicit first-party grammar support, not a TinyXML2 fallback
or a vendor patch. It preserves such cases as extended whitespace, unknown
nodes and closing-tag attributes. Unknown/PI nodes retain their non-text
position; comments remain skippable by the first-text accessor.

Public pugixml options leave entity conversion, EOL conversion and attribute
whitespace conversion disabled. Document-owned cached values normalize CR,
CRLF and LFCR before checked reference decoding. Invalid/unknown references
remain literal, valid references decode once, and CDATA receives newline
normalization without entity decoding. DTD entities stay unexpanded. Views
survive cache/wrapper rehash and subsequent parses; failed value allocation
can retry on the same document.

This layer preserves the tested legacy dialect, including permissive malformed
forms. It is not a new validating XML policy. Raw UTF-8 and input-budget policies
are unchanged. Additional first-party grammar code has a maintenance cost;
future changes must retain the differential checks.

## Required checks and current evidence

The runner now **fails** on a candidate reader/corpus assertion, any recorded
acceptance/tree/status/full-diagnostic difference, message or storage mismatch,
differential-mutation mismatch, candidate allocation leak, sanitizer error or
oracle linkage. Candidate failures are no longer permitted as successful
diagnostic measurements.

The required Release and ASan+UBSan profiles cover the following matrices.
Exact-head results are retained as workflow artifacts and in the PR description;
this list is test scope, not a substitute for checking those results:

- The unchanged 5,042-check reader test and 12,087-row corrected corpus test on
  both backends. The corpus also compares every full diagnostic and tree:
  **12,087 exact matches; zero differences**.
- 27 explicit grammar/normalization/depth/diagnostic boundary cases, including
  10,000-byte names/declarations and the 999-byte error-text bound. The existing
  entity, wide/deep/large-input and decoded-view ownership fixtures remain.
- Actual message processing: **47 ordinary and nine negative flows match**,
  including response bytes, retry/decryption state and handler effects.
  All eight actual achievement Storage/writer state/INI cases match.
- 100,000 deterministic original mutations (seed 85621504) plus 500,000 expanded
  holdout mutations/random byte strings (seed 202892583), each evaluated by both
  readers: **1,200,000 parses and zero differences per configuration**. These
  are bounded differential tests, not coverage-guided or exhaustive fuzzing.
- Per-allocation one-shot/persistent failures on plain/reference-heavy,
  legacy, many-attribute, deep and malformed inputs. Attribute/text cache tests
  lift an injected failure and retry the same document. The local configuration
  uses 614 isolated allocation processes: 18 controls and 596 injected failures.

The expanded malformed-input allocation probe exposes a **failed reference
measurement**: pinned TinyXML2 retains one allocation when the final error-text
allocation fails. Its nonzero process status and counters remain in
`reference_oom_failures` and the complete probe log. The runner permits this
specific reference-only observation only when it is a bad-allocation outcome
with a positive outstanding count; other crashes, tracking overflow and
sanitizer faults still fail. It is not reproduced in the candidate or reported
as a reference PASS. LSan remains enabled; the allocation registry clears its
artificial pointer roots before process exit. Candidate probes must stay clean.

MSVC Debug adds an independent standard-library-only control, with no parser
call. If failing an empty string's allocation demonstrably terminates through
its noexcept constructor, the diagnostic records that runtime behavior before
classifying matching injected `std::bad_alloc` terminations. Debug iterator
checking is preserved. Such exits are accepted only for that demonstrated
Debug configuration, a selected injected allocation, the active bad-allocation
exception and a non-overflowing tracker. They remain
`UNRECOVERABLE_DEBUG_OOM_NOT_A_RECOVERY_PASS`, with allocation recovery marked
unproven. Counters at termination are not normal cleanup/leak measurements.
Unknown terminations still fail; a separate deliberate-termination control
verifies they cannot produce an eligible Debug allocation record. Release
candidate failures retain their strict leak/recovery requirements. This does
not classify every Debug fault as recoverable or disable any functional test.

The Windows matrix additionally runs the unchanged complete router/AES,
settings/license and real-file achievement-storage fixtures with both backends
in MSVC Debug and Release. These are source-linked integration fixtures, not
real Consumer/game or production-DLL acceptance. Exact-head hosted execution
must be checked before describing those profiles as passed.

## Performance and remaining scope

Adding compatibility reduces the initial minimally mapped candidate's speed
advantage. An illustrative local Release measurement (median p50 over five
interleaved repeats, 2,000 parses/traversals/destructions per case) is:

| Input | Corrected TinyXML2 | Compatible pugixml |
| --- | ---: | ---: |
| Small request | 0.681 microseconds | 0.501 microseconds |
| Request with declaration | 0.771 microseconds | 0.551 microseconds |
| 64 KiB text | 83.756 microseconds | 28.513 microseconds |
| 256 children | 37.005 microseconds | 29.224 microseconds |
| Reference-heavy text | 27.832 microseconds | 26.520 microseconds |
| Legacy accepted form | 0.531 microseconds | 0.541 microseconds |

Small-request tracked peak heap remains **9,614 versus 33,598 bytes**. Tracking
covers ordinary C++ allocations and pugixml's public allocator callback, not
RSS/OS allocations or every aligned allocation family. Allocator instrumentation
runs in a separate executable. Timing is informational and host-specific, with
no CI speed threshold; sanitizer runs omit timing.

The current conclusion is compatibility on the tested contract, with a useful
speed gain on ordinary fixtures and a higher small-document memory cost.
Production now selects this reader, with separate static-CRT and development
shared-CRT pugixml builds. Full native application/Golden acceptance must run
with this selection. Real Consumer validation remains the owner's separate
pre-release task. Passing source-linked comparison fixtures does not establish
real game execution or universal XML equivalence.

## Reproduce

```sh
git fetch origin a34a0536f0980b38d77d029f11fa3720dcaedc6e
git worktree add --detach build/xml-reference a34a0536f0980b38d77d029f11fa3720dcaedc6e
python -B tests/diagnostics/xml_backend/run.py \
  --reference-root build/xml-reference \
  --output build/xml-backend-comparison/release
```

The installed owner-archive pugixml tree is the default input. The control
checkout must already exist; the runner itself is offline. Repeat with `--sanitizers` and a separate output directory for
GCC/Clang ASan+UBSan. On Windows, use `--configuration Debug` or `Release`.
Inputs are normalized to CMake paths before source expansion. Output paths stay
below this checkout's ignored `build/`. `summary.json` records completeness,
exact harness/input identities, evaluated candidate linkage, differential
results, reference allocation failures, timings and the qualified decision.
CI retains these files and logs as artifacts, not new evidence-tree commits.
