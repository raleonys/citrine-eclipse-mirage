# Review of the new reverse-engineering input

The owner authorized separate correction PRs for the supplied ten-part
`dll-reverse-engineering(10).zip` input. The comparison baseline is accepted
commit `498a241b6264621a91c35ce6bc2de7c60b0b3a38`.

The joined archive is 245,879,002 bytes, SHA256
`afca62680802cc77d6dfd72cfbdb00c4e27fa9afeb49c069e62cdfc65eb90356`.
All 6,713 ZIP member CRCs and all 876 entries in its source snapshot verified.
These are byte-integrity checks, not independent confirmation of its build or
native equivalence claims. The input itself still marks its DLL as blocked.

## Source comparison

The source/build-tool filter excludes generated build directories and includes
C/C++ headers and sources, CMake, Python, PowerShell, DEF and RC files. Relative
to the previously supplied input, 961 members are unchanged, 85 are added and
30 are changed; none are removed. The 115 delta members comprise 67 ordinary
C/C++ files, 39 test/fixture files and nine build/tool files. This count is not
a count of missing product functions.

The original assembly and pseudocode dumps are byte-identical to the previous
input. Their SHA256 values are, respectively:

- `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`
- `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`

## Proposed corrections and test adoptions

| Area | Finding and bounded adoption |
| --- | --- |
| Online cookie | Destroy the local path copy after committing the factory result, before status/profile callbacks. Check normal, factory-throw and status-throw paths with the actual allocation lifetime. |
| Synchronous greeting | Release the temporary greeting after full transmission, before initiating the first read. Check large chunked output and send failure. Preserve the active asynchronous transport. |
| Foreign provider slots | Read the readable foreign object/table as bytes, reject null tables/slots, and use the observed one-object destroy call. Test the production adapter with an actual provider fixture and unaligned byte-backed layouts. This does not validate arbitrary addresses or prove a complete foreign class ABI. |
| Cold clock cache | Publish the caller-owned frequency atomically. A two-thread cold-cache test reproduces a data race under ThreadSanitizer before the fix and passes afterwards. The application already initializes its frequency before workers; this is not evidence of an observed game-startup failure. |
| Configuration constructor | Existing defaults already match the selected contract, subject to explicit owner policies. Add exact defaults, independent state and allocation-failure unwind tests; do not rewrite correct construction code. |

The corrections are independently reviewable. Passing portable tests or Wine
tests does not establish native MSVC, Golden or game acceptance for changed
runtime code. Those acceptance results must refer to the resulting commit and
binary, rather than an older accepted build.

## Applicability of the other additions

| Input families | Decision |
| --- | --- |
| Online object/query/version | Adopt the small lifetime/slot fixes above. Reject the input's `Binding::free` behavior that clears ownership after a failed lower release. Preserve the accepted retained-lease/retry correction from PR #25. |
| Configuration defaults | Adopt tests against the real product constructor. Retain SURGE identity, automatic listener selection and the owner-fixed SDK policy. |
| Startup publication | Adopt the independently reproduced cache correction. Do not replace existing readiness and worker ownership with the input's private-domain publication graph. |
| Shared logging | The product already serializes Debug-only diagnostics through `Environment`. The input's recovered shared-stream leases are a different stream/provider model. |
| Callback identity/provider and original checkpoint/provider | These bind selected reconstructed CRT/PTD and original-slot providers. They are not evidence that equivalent SDK functions are absent from the product. Private PTD checks do not establish registry HKEY provenance. |
| Socket move, session owner/dispose/flow | Keep useful lifetime and ordering scenarios. Raw MSVC string/deque/refcount/executor layouts cannot replace compiler-CRT containers and Asio socket ownership. Session cipher copying in the product is already disabled. |
| Session memory, I/O memory, owned server and accept | These compose a new private-domain ownership graph. Their controller/fiber admission, typed packet projections and fail-stop cleanup do not share the active Asio ABI. They remain comparison material, not a replacement transport. |
| DLL link gate and reconstruction tooling | These describe the input's blocked reconstruction. Preserve the product's three-entry production ABI, vendor locks and Golden pipeline. |

An external per-member applicability ledger covers all 115 delta members; raw
logs and generated ledgers remain outside the source deliverable. This review
does not close every original native ABI, provider-liveness or donor-retirement
obligation, and does not assert that no further bugs exist. In particular,
private-domain graph tests cannot be counted as tests of the active product
unless their scenario is adapted to, and executed against, the actual product
implementation.
