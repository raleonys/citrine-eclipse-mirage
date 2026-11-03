# Owner-supplied vendor archives

The source consists of four active ZIP archives supplied
by the owner. For Asio, the later uploaded archive
replaced the former input; the owner subsequently selected the supplied
pugixml archive to replace TinyXML2 in PR #149. MinHook and PEGTL
archives are unchanged. Gulrak was removed by the
owner's later instruction. Brownfield vendor copies
or other versions downloaded from the internet
are not substituted.

| Vendor | Version stated in archive | Location |
|---|---|---|
| Asio | 1.38.2; owner snapshot `bdf337ddb1a667c274372a602de5857ad3cb3ed0` | `third_party/asio` |
| MinHook | v1.3.4 in README | `third_party/minhook` |
| PEGTL | 4.0.0 | `third_party/pegtl` |
| pugixml | 1.16; owner snapshot `f0a3faa76e04f35110257ca8713363dee3c890e8` | `third_party/pugixml` |

All 2,753 file/link entries are verified against
archive-member hash lists. Licenses and Asio's
logical links `asio/include → ../include`
and `asio/src → ../src` are preserved.
In the Windows-portable source ZIP, these two
entries are regular files containing the exact
original link-target text bytes; symlink
permissions are not required. Compilation
uses the actual `third_party/asio/include`
directory. Headers/sources, licenses,
original ZIPs and vendor member hashes
remain unchanged. Archive-byte equality
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
it does not execute upstream CMake, fetch
dependencies or use a package manager.
Project CMake targets are
`preloader_l_asio`, `preloader_l_minhook`,
`preloader_l_pegtl`, and
`preloader_l_pugixml`. The previous `preloader_l_tinyxml2` identifier is
retained as a CMake alias of pugixml for build compatibility; it contains no
TinyXML2 source. The development DLL uses its own shared-CRT pugixml build.
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
interface. The legacy depth contract remains: 498 nested elements succeed and
499 fail with error 18. This is a project policy, not a pugixml vendor patch.

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

MinHook and pugixml are linked to existing
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

The PEGTL 4 public-input probe runs with
Linux/GCC. The same public headers also
compile with LLVM-MinGW **20251118,
Clang/libc++ 21.1.6, UCRT x64**.
LLVM-MinGW 20261006 / Clang 23's libc++
changed the private `std::vector::__end_`
field accessed by PEGTL. These are
two different compiler outcomes.
Vendor files were not patched and the
PEGTL version was not changed.

The default
`PRELOADER_L_REQUIRE_VENDOR_COMPATIBILITY=ON`
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
The actual vendor pins remain unchanged.
Results and tool provenance are in
[VENDORING_STATUS.json](../metadata/evidence/VENDORING_STATUS.json).

For diagnosing an old/incompatible compiler
only, a separate build directory may use
`-DPRELOADER_L_REQUIRE_VENDOR_COMPATIBILITY=OFF`.
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
python -B scripts/extract_source.py ../yellowfield-windows-vendor-source.zip ../yellowfield-clean
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

Current runtime integration:
[RUNTIME_BINDING.md](RUNTIME_BINDING.md).
Earlier vendor/parser test counts here
are historical snapshot evidence; newer
Application tests are in
[RUNTIME_CURRENT_STATUS.json](../metadata/RUNTIME_CURRENT_STATUS.json).

Owner authorization to remove Gulrak
and restore Windows I/O:
[WINDOWS_IO_FIX.md](WINDOWS_IO_FIX.md).
Asio Windows consumers use
`_WIN32_WINNT`/`WINVER=0x0601`
as in Brownfield.

Current commit evidence and hash-pinned
SAFE-FREEZE exception applied only to
a build copy:
[HOOK_ENGINE_HARDENING.md](HOOK_ENGINE_HARDENING.md).
The three retained vendor pins are unchanged; pugixml is pinned separately
to its exact owner archive.
