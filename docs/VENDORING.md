# Owner-supplied vendor archives

The source consists of four active ZIP archives supplied
by the owner. The owner refreshed Asio and PEGTL from the supplied upstream snapshots on
2026-11-09; pugixml replaced TinyXML2 in PR #149. The owner subsequently selected
vanilla MithrilHook 1.0.4 as the hook engine, replacing the separate MinHook tree. Gulrak was removed by the
owner's later instruction. Brownfield vendor copies
or other versions downloaded from the internet
are not substituted.

| Vendor | Version stated in archive | Location |
|---|---|---|
| Asio | 1.39.0; owner snapshot `190a8d8daf1cc62909b4e0c72ff491ba264ca98e` | `third_party/asio` |
| MithrilHook | 1.0.4; ZIP comment `10203713702b7d9349eb2212bf1bf2e4314a5068` | `third_party/mithrilhook` |
| PEGTL | 4.0.2 in header; owner 4.x branch snapshot `b60f6110abd37e502d202d13a6c565410c552c33` | `third_party/pegtl` |
| pugixml | 1.16; owner snapshot `f0a3faa76e04f35110257ca8713363dee3c890e8` | `third_party/pugixml` |

## Archive identities and recorded upstream snapshots

The snapshot identifiers above and the archive identities below are recorded in
[`metadata/VENDOR_LOCK.json`](../metadata/VENDOR_LOCK.json). An archive SHA-256
identifies the exact supplied ZIP bytes; it is distinct from an upstream Git
commit ID and the per-member hashes used to check the installed tree.

| Vendor | Supplied archive | Archive SHA-256 |
|---|---|---|
| Asio | `asio-master.zip` | `5f384a220751aff27e517e559b9257292f00e7a2c31c2e83282d3e522d591ade` |
| MithrilHook | `mithrilhook-1.0.4.zip` | `e2c1248ed063beb23cb6b5dc63bd6cef8cf9dfc5adbf8fd333729cfac97b76b3` |
| PEGTL | `PEGTL-4.x.zip` | `3eb2096ceeb7bbf722775d7cfd98ddf0f3ca008c5e2f44835b11dd014c1400a0` |
| pugixml | `pugixml-master.zip` | `4cd80a72fde87ce1f23eae4ed048446b2e887fc25437793e525367448be2cf81` |

The Asio input was supplied as `asio-master (1).zip`, and the selected
PEGTL input as `PEGTL-4.x.zip`. The archive directory uses the canonical names
in the lock without altering ZIP bytes. Both archive SHA-256 values and every
extracted payload were verified. PEGTL is selected from the uploaded **4.x
branch snapshot**, not a stable release archive or a release tag. Its header
declares **4.0.2**; that version text does not change the selection authority.
The earlier uploaded `PEGTL-main (1).zip` remains recorded as an unselected input.

Snapshot IDs are recorded from comments in the SHA-256-pinned supplied ZIPs.
For the selected PEGTL upload, all 803 member paths, Git blob hashes and
executable/link modes also match the independent upstream Git tree at the
recorded commit. Other archive-comment records retain their stated verification
limits; no moving branch head is claimed to be an immutable upstream release.
Original archives remain external to the source package. Previous selections
remain recorded in SOURCE_INPUTS.json and
Git history; earlier execution evidence is not new-snapshot acceptance.

Before CRC verification or payload reads, the pinned vendor ZIP inspector
preflights every member's root/path, case-fold identity, supported type and
header-reported uncompressed size. Existing limits remain 32 MiB per member
and 100 MiB in total; nonempty directory entries are rejected. Exact archive
SHA-256/size and commit-comment binding precede this scan; CRC, per-member
hashes and locked symlink targets are still verified after it. These byte
budgets do not establish complete resource isolation for arbitrary ZIP input.

## Installed tree verification

Vendor member manifests and ZIP entries share the source-package path policy:
[`vendor_sync.safe_rel()`](../scripts/vendor_sync.py) delegates to
[`project_files.safe_member()`](../scripts/project_files.py) before staging or
materialization. It rejects Windows device names, trailing spaces/dots, forbidden
punctuation and controls while retaining ordinary pinned paths. Link targets
remain subject to the separate archive-locked alias checks.

All 2,733 file/link entries are verified against
archive-member hash lists. Licenses and Asio's
logical links `asio/include → ../include`
and `asio/src → ../src` are preserved.
In the Windows-portable source ZIP, these two
entries are regular files containing the exact
original link-target text bytes; symlink
permissions are not required. Compilation
uses the actual `third_party/asio/include`
directory. The selected upstream payloads, licenses and link-target bytes are
installed exactly; member hashes pin the refreshed source selection. Archive-byte equality
is not an additional claim of a verified
upstream Git commit. Source packages contain
the complete extracted vendor trees, not
the original input ZIP files.

```sh
python -B scripts/vendor_sync.py --root . --verify-installed
python -B scripts/vendor_sync.py --root . --archive-dir "/path/to/3RD PARTY" --verify-archives
```

An installation with `--install` does not
overwrite a populated target. CMake verifies
vendored sources offline at configure time;
the exact MithrilHook CMake project is included offline. No dependency is fetched
and no package manager is used.
Project CMake targets are
`laureondo_asio`, `mithrilhook`,
`laureondo_pegtl`, and
`laureondo_pugixml`. The development DLL uses its own shared-CRT
`laureondo_source_pugixml` and `laureondo_source_mithrilhook` builds. No legacy XML vendor target aliases are provided.
XPath is disabled with the public `PUGIXML_NO_XPATH` definition.
Build-time member-hash verification also runs.
On mismatch, the checker reports missing,
extra and modified paths and expected/actual
SHA-256 values. `1607/1607` merely indicates
matching counts, not identical bytes.
Do not resolve a mismatch by refreshing
hashes or disabling the checker.

On Windows, the standalone Asio target
declares available threading using
`ASIO_HAS_THREADS=1`. This prevents
LLVM-MinGW's missing `_MT` definition
from incorrectly selecting `null_thread`.
It is not a vendor-source patch or a
disabled threading test.

## XML compatibility and retired input

Pugixml is compiled directly from the complete, unmodified 191-file owner
archive. Its MIT license and member hashes are included. Project grammar,
checked reference decoding, diagnostic text and depth handling live in
`src/xml/reader_policy.cpp` and `reader_values.cpp`, behind the unchanged reader
interface. The owner-requested A12 depth-100 profile accepts 98 paired nested
elements and rejects the 99th entered child list with error 18. A self-closed
leaf at level 99 remains valid; malformed-attribute and EOF error priority are
preserved. This replaces the previous depth-500 profile (498/499 boundary). It
is first-party policy, not a pugixml requirement or vendor patch. See
[A12_DEVIATIONS.md](A12_DEVIATIONS.md) for rationale and remaining differences.

The previous TinyXML2 lock/member list remains historical provenance in
`retired_vendor_inputs`; its active tree and build input are removed. Git
history and the diagnostic's separately pinned PR #108 checkout retain the
original bytes and tests. TinyXML2 is reachable only in that external control,
never in the application or development DLL. The source graph rejects a
retired TinyXML2 path in production. See [XML_BACKEND_COMPARISON.md](XML_BACKEND_COMPARISON.md)
for differential checks, deliberate malformed-value corrections and evidence
limits. Earlier vendoring reports describe the vendor selection at their own
checkpoint; they are not fresh pugixml native acceptance.

## Build-time versus active runtime use

MithrilHook and pugixml are linked to existing
DLL hook-engine/XML targets. PEGTL now runs
the existing Config2/VDF grammar documented
in RE within the actual Application parser:
[VDF_PARSER.md](VDF_PARSER.md).
The separate numeric fixture remains a
vendor-API check. Asio is linked to the
actual Application transport.
Windows storage/I/O uses standard
filesystem/fstream; the Gulrak target
was removed. Concrete paths for the
four active libraries are documented in
[RUNTIME_BINDING.md](RUNTIME_BINDING.md).
These integrations are not substitutes
for actual Windows/game acceptance.

## PEGTL/compiler compatibility

The selected owner-supplied PEGTL 4.x snapshot declares `cxx_std_17`.
Project C++ targets and the required public-input probe use C++20; C sources
retain C17. The VDF byte-sequence rules retain their character-pack API for
CRLF, line comments and block-comment termination. Their byte sequences, action
ordering, error mapping and grammar policy are unchanged. Vendor headers are
unmodified. The unselected main snapshot requires C++23 and is not substituted.

The toolchain-status reader requires exactly one active record for
`LAUREONDO_PEGTL_PUBLIC_INPUT_WORKS`, with type `INTERNAL`.
The exact value `1` returns diagnostic exit 0 and directs the caller to run
`vendor_pegtl_smoke`; it does not replace that execution. Empty or `0` returns
`UNAVAILABLE` with CTest skip code 77. Missing or repeated records, wrong types
and malformed values return `INVALID` with exit 1, so corrupted evidence cannot
become a normal skip. Repeated logical keys remain ambiguous even when their
types differ. Comment lines and unrelated prefixed keys are not probe records.

The PEGTL public-input probe runs with Linux/GCC. The following compiler
observations describe the earlier snapshot; fresh results for the selected
head belong to its PR validation. The earlier public headers also
compile with LLVM-MinGW **20251118,
Clang/libc++ 21.1.6, UCRT x64**.
LLVM-MinGW 20261006 / Clang 23's libc++
changed the private `std::vector::__end_`
field accessed by PEGTL. These are
two different compiler outcomes.
Vendor files were not patched and the
PEGTL version was not changed.

The default
`LAUREONDO_REQUIRE_VENDOR_COMPATIBILITY=ON`
gate stops CMake if the PEGTL public API
cannot compile, even with
`BUILD_TESTING=OFF`. The previously
skipped test was replaced by an actual
PEGTL execution test in the compatible
Windows cross-build. MSVC results still
require separate verification; MSVC was
not executed in the Linux session.

```sh
export LLVM_MINGW_ROOT=/path/to/llvm-mingw-20251118-ucrt-ubuntu-22.04-x86_64
cmake --preset llvm-mingw-x64-vendors
cmake --build --preset vendors --parallel 3
```

Compiler downloads are not vendor-source
inputs and are not bundled in the product
ZIP. The verified tool archive SHA-256:
`53a0c22caa46b501e1c089e1c31b24a7c0e0d5a86f8ad12b131aafd4cee01ef4`.
These toolchain results describe the earlier vendor selection; current pins
are in VENDOR_LOCK.json. Historical results and tool provenance are in
[VENDORING_STATUS.json](../metadata/evidence/VENDORING_STATUS.json).

For diagnosing an old/incompatible compiler
only, a separate build directory may use
`-DLAUREONDO_REQUIRE_VENDOR_COMPATIBILITY=OFF`.
This mode is solely for vendor diagnostics.
The PEGTL test remains **SKIPPED/UNAVAILABLE**;
the real VDF parser target still fails
to compile under that compiler. This does
not count as compatibility PASS.
Compiler diagnostics appear in
`CMakeFiles/CMakeConfigureLog.yaml`
on new CMake versions or
`CMakeFiles/CMakeError.log` on old ones.
The original Clang 23 error is retained
in `metadata/evidence/VENDOR_PEGTL_COMPILER_DIAGNOSTIC.log`.

`vendor_io_contracts` exercises actual
loopback TCP accept/connect/read/write,
EOF and cancellation completions with
Asio, and Unicode directories/files,
binary content, rename, enumeration,
deletion and missing-file errors with
standard filesystem. The connection
uses only an ephemeral loopback port
selected by the kernel; temporary files
are removed from the test's own directory.
These tests do not establish the
Application's Production transport/storage
policy or final bootstrap.

## Extracting ZIPs on Windows

The current source ZIP has a flat root
and contains only regular file entries.
It can be extracted using 7-Zip, WinRAR
or the standard Windows ZIP extractor.
Extract into a fresh directory rather
than merging into an earlier damaged
extraction. Optional verified extraction:

```sh
python -B scripts/extract_source.py ../laureondo-source.zip ../laureondo-clean
```

The extractor verifies CRC, hash, member
and type and refuses to overwrite an
existing destination. No Developer Mode,
administrator permission or
`restore_vendor_links.py` is needed
for the new package. Only the exact
target text for the two paths in
VENDOR_LOCK is accepted. Empty/wrong
text, mismatched symlinks, directory
copies and other symlinks are rejected.
Historical ZIPs with real symlinks
retain their earlier extraction
requirements. `restore_vendor_links.py`
remains optional for converting the
correct text back into real links;
it is not a build prerequisite.

Restoration validates the complete locked alias set before publishing any
conversion. Each link is created in owned temporary staging and atomically
replaces its descriptor after creation succeeds. If a later conversion fails,
earlier valid links remain published and pending descriptors retain their
exact bytes. An ordinary retry processes only the remaining descriptors;
source-manifest identity is preserved across this partial state.

Current runtime integration:
[RUNTIME_BINDING.md](RUNTIME_BINDING.md).
Earlier vendor/parser test counts here
are historical snapshot evidence; newer
Application bindings are in
[RUNTIME_CURRENT_STATUS.json](../metadata/RUNTIME_CURRENT_STATUS.json). New
test execution belongs to the exact selected head's workflow results.

Owner authorization to remove Gulrak
and restore Windows I/O:
[WINDOWS_IO_FIX.md](WINDOWS_IO_FIX.md).
Asio Windows consumers use
`_WIN32_WINNT`/`WINVER=0x0601`
as in Brownfield.

Current engine identity, ownership limits and application-specific tests:
[MITHRILHOOK_INTEGRATION.md](MITHRILHOOK_INTEGRATION.md).
No application source patch is applied to the 104-file MithrilHook tree.
Retained MinHook/BSD, HDE and SAFE-FREEZE attribution is part of that immutable
GPL-3.0-only combined distribution; see its THIRD_PARTY_NOTICES.txt.
