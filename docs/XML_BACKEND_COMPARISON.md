# Isolated XML backend comparison

This diagnostic evaluates pugixml as a second implementation of the existing
`messages::XmlBackend` and `dispatch::NodeAccess` interfaces. It does not select
a production backend. The shipping dependency graph, the four accepted vendor
trees, their locks, the public DLL ABI and the top-level CMake configuration
are unchanged.

## Inputs and separation

The reference is the reviewed PR #108 correction at
`a34a0536f0980b38d77d029f11fa3720dcaedc6e`. That proposal was still unmerged when
this comparison was prepared. A separate clean checkout supplies its actual
reader, tests, message processing and achievement storage implementation. The
diagnostic verifies its exact Git commit and rejects tracked input changes.
This is a comparison against the proposed corrected contract, not a statement
that current `main` already contains that correction. Refreshing this control
requires an explicit pin and fixture review after #108 changes or merges.

The owner-uploaded `pugixml-master.zip` has SHA-256
`4cd80a72fde87ce1f23eae4ed048446b2e887fc25437793e525367448be2cf81`.
Its three core files match official pugixml commit
`f0a3faa76e04f35110257ca8713363dee3c890e8` (version macro 1160). The runner pins
each core file and its MIT license separately. It can read a local extraction or
explicitly download those exact diagnostic inputs into ignored `build/`.
It never changes `third_party/` or updates `VENDOR_LOCK.json`. XPath is disabled
only for the candidate diagnostic target. No vendor source is patched.

`tests/diagnostics/xml_backend/CMakeLists.txt` is a standalone build. Its
`comparison_*`, `reference_*` and `candidate_reader` targets have no connection
to production targets. Both readers use the **same actual checked decoder**
from the pinned reference source. `reference_codec.cpp` includes that source
once in a diagnostic translation unit and exposes its local decoder to the
candidate. No production API or duplicate decoder is introduced. Pugixml's
public parse flags disable its entity conversion and attribute whitespace
conversion; the existing first-party `MemoryPrinter` supplies serialization.

## Contract being measured

- Stop at the first NUL inside the supplied view, own the parse buffer, and
  preserve document/node/value views through later parses and map rehashes.
- Preserve element and attribute order, null versus empty text, first eligible
  text rather than concatenated descendant content, and CDATA boundaries.
- Decode the five predefined entities and valid XML character references once
  using checked arithmetic. Preserve unknown and invalid references literally;
  escaped ampersands never cause a second decoding pass. Do not expand DTD
  entities. Raw UTF-8 validation and input-size budgets are not changed here.
- Keep the reference's syntax acceptance, newline/BOM policy and depth boundary
  (498 elements accepted, 499 rejected) visible in comparisons.
- Compare returned status, `has_error`, full diagnostic and tree independently.
  The actual message processor branches specially on error codes 7 and 8;
  approximate numeric mapping alone is insufficient to prove equivalence.
- Preserve complete achievement keys, names, resulting state and INI bytes.

The candidate intentionally has only a small status mapping. It does not claim
to reproduce TinyXML2's grammar, depth policy or diagnostics. Differences are
recorded without classifying each one as a security vulnerability.

## Measurements and decision

The local Linux/GCC Release comparison found **335 acceptance
differences**, **14 successful-tree differences**, **3,065 status differences**
and **11,595 full-diagnostic differences** in the unchanged 12,087-input corpus.
The separate reference corpus test validates every original frozen row and all
20 explicitly reviewed value corrections from #108; fixtures are not rewritten.
The candidate fails the unchanged reader contract at the BOM/leading-whitespace
and LFCR case. Its failure is retained in `candidate-reader.log`; later checks
in that executable are not reported as passed.

Actual message processing has one difference in the 47-case matrix and four
in the nine negative cases. The eight achievement-storage cases match. These
use real project processing/storage/writer code with fake crypto/effects and
in-memory files; they are not game, network or native filesystem acceptance.

The same run measured these local median p50 parse/traverse/destruction times
over five interleaved repeats, 2,000 iterations per case:

| Input | Corrected TinyXML2 | Checked-value pugixml |
| --- | ---: | ---: |
| Small request | 0.671 microseconds | 0.260 microseconds |
| 64 KiB text | 83.716 microseconds | 21.472 microseconds |
| 256 children | 36.946 microseconds | 18.888 microseconds |
| Reference-heavy text | 27.541 microseconds | 25.618 microseconds |

For the small request, tracked peak live heap is **9,614 versus 33,598 bytes**.
This measures ordinary C++ allocations plus pugixml's public allocator callback,
not process RSS, OS allocations or all aligned allocation families. Allocation
probing runs in a separate executable and does not instrument the timing loop.
Timing is informational, host-specific and not a CI speed threshold. The
candidate's speed must be measured again after any additional compatibility
policy is implemented.

The diagnostic also checks common value semantics, decoded-cache and wrapper
ownership, external/internal entity fixtures, depth/width/large-input cases,
100,000 deterministic mutated inputs per reader (seed 85621504), and isolated
one-shot/persistent allocation failures. These are bounded tests, not exhaustive
fuzzing, an I/O audit, or a vulnerability-free claim. Fresh logs and exact harness
file hashes are emitted with every run. Sanitizer runs omit timing.

The current decision is **NO_GO_DIRECT_MIGRATION**: faster parsing is useful,
but the observed message/error and syntax changes remain migration blockers.
The first step remains reviewing and integrating the narrow #108 correction.
A later migration needs explicit acceptance/error/normalization policies,
regressions for the recorded differences, updated measurements and separate
native Windows plus real Consumer validation. An adapter reduces coupling; it
does not supply those policies or establish parser safety.

## Reproduce

From a clean project checkout, prepare a separate reference checkout without
switching the active branch:

```sh
git fetch origin a34a0536f0980b38d77d029f11fa3720dcaedc6e
git worktree add --detach build/xml-reference a34a0536f0980b38d77d029f11fa3720dcaedc6e
python -B tests/diagnostics/xml_backend/run.py \
  --reference-root build/xml-reference \
  --pugixml-root build/xml-comparison-input/pugixml --fetch-pugixml \
  --output build/xml-backend-comparison/release
```

For offline execution, omit `--fetch-pugixml` and point `--pugixml-root` at the
verified owner-upload extraction. Repeat with `--sanitizers` and a different
`--output` directory for GCC/Clang ASan+UBSan. Output directories are confined to
this checkout's ignored `build/`. The separate workflow runs Linux Release,
Linux ASan+UBSan and native Windows/MSVC Release; execution remains pending until
the corresponding exact-head jobs finish.

`summary.json` distinguishes execution completeness, the candidate contract
**FAIL**, corpus/application differences, allocation results and the migration
decision. A successful **diagnostic job** means that the reference tests and
diagnostic assertions passed and measurements completed. It does not turn the
recorded candidate failure into acceptance. A crash, reference regression,
sanitizer error or unexpected harness failure still fails the job. Generated
logs, CSV/TSV, allocator records and compiler configuration are retained as
workflow artifacts, not committed under evidence directories.

Local Release and ASan+UBSan runs both completed: the reference passed 5,042
reader checks and all 12,087 corpus cases; the common value/ownership fixtures,
100,000 mutations per reader and 94 allocation-probe processes (four controls,
90 one-shot/persistent injected failures) passed without a tracked leak or
sanitizer report. Both runs retained the candidate reader failure and the
NO_GO decision. Native MSVC and real Consumer execution were not performed
locally.
