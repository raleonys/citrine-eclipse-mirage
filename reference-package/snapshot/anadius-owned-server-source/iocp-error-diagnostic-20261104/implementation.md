# Bounded IOCP diagnostic implementation

Source-only implementation, 2026-11-04. Changed ONLY outputs/anadius-owned-server-source/server.cpp. No tests, status/work-plan files, public signatures, constructor algorithms or other source files were edited by this author. No compiler, build, test, link, target/DLL execution or package update.

Current server.cpp SHA256: **db9337ccfff364b63c9dfd269822166e6d8bf04181ddf841f20543f43fa3a42d**. Actual pre-edit stage/before/server.cpp SHA256 was verified against the current file before editing: **9516bed4855e3d144f4617ec73e44cf6116d2912b8e757646210ccfbd6fe5300**.

## Exact selector and context mapping

The private contextual_error_category selector uses the existing module-owned socket native system category ONLY when both conditions hold:

- logical error category is system;
- context address is exactly84428 or84528.

All other cases keep the previous mapping: misc uses misc_error_category, remaining HOST categories use compiler std::system_category. The context-free SourceError constructor is unchanged. The selector does not cast/dereference the context address or infer a category from a numeric name token.

SourceError's existing three-argument constructor now invokes that selector while retaining the caller's operation string, original logical Error and context_address members unchanged. No public overload, dependency injection field, category lifetime guard or generic exception factory was added.

Actual Construction::raise028240(error,address) maps84428 to mutex and84528 to iocp, matching the fresh measured C strings. Unknown addresses retain the existing IOCP constructor fallback label. Direct HOST calls to SourceError with an arbitrary operation string still keep that caller-owned string; address matching does not silently replace operation text in the public constructor.

## Native justification and finite behavior

Fresh024990 evidence in this stage establishes both actual nonzero branches use028620's system category: gate status after027be0 selects mutex, and null completion port plus captured nonzero GetLastError selects iocp. Prior fresh028240/0248b0/00a270 and020950 evidence supplies the context/separator/category-message composition and native system message provider; actual selected libc++ constructor evidence shows the supplied category.message is used once for these nonzero inputs.

Thus those two selected typed errors now retain std::system_error catch compatibility and signed code while intentionally choosing the stable asio.system category and correct native context text. Existing SourceError metadata and copies remain intact; its category reference is backed by the established module-lifetime pool.

No change was made to initialize_gate/construct_iocp024990, error capture order, category lookup, port storage, worker creation, cleanup or throw guards. In particular, native NULL completion port with GetLastError0 still falls through; this implementation does not turn NULL alone into an error or retry an effect.

## Explicit limits and validation status

This is a bounded selected-context correction. Other addresses, context-free SourceError and unrelated logical-category/HOST inputs retain their earlier projection. Direct hypothetical zero-code SourceError construction at these addresses is not claimed native00a270-equivalent; the freshly proved024990 branches only raise nonzero. Policy's separate zero-value helper was not moved or refactored.

Native exception/RTTI/unwind/allocator ABI, exact compiler metadata-copy effects and current OS text remain outside this source implementation. No broad CRT work, global category migration, new unload condition or overall-goal completion claim is introduced.

Root/another author owns the two source test files; independent production review and separately authorized execution remain distinct. This note is author self-review and exact hashes, not a test pass or independent approval.
