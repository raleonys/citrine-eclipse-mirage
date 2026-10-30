# Owner-provided Asio snapshot update

The owner supplied a new asio-master.zip and explicitly requested the update.
The header still reports 1.38.2 (ASIO_VERSION 103802); this is a source snapshot
update, not a claim that a new numbered Asio release was supplied.

| Pin | Previous | New |
|---|---|---|
| Archive SHA256 |1070a32d6c26ebf441d3de8b2fc89c5fc580f2fe0ca4aee8dfb5e818a5096af9|d28dbc79b8c98b860ba6c06bd89de76bc352931560f9d169819e46280778975b|
| ZIP commit comment |026dce530a6c39a8aa6d98e5177482310c4a01e5|bdf337ddb1a667c274372a602de5857ad3cb3ed0|
| Archive bytes |4192700|4239595|
| Installed file/alias members |1607|1629|

The new supplied archive passes ZIP CRC and the offline bounded-member verifier.
Every installed payload is exact; the two registered aliases retain their exact
relative-target descriptors. All files, licenses, examples and tests supplied
by the owner are retained. No vendor source patch, download or upstream CMake
import is used. The previous archive remains outside the product unchanged.
A ZIP comment is provenance bound to the pinned archive, not independent proof
of a Git object or an independently verified upstream release/signature.

## Reviewed delta and integration

The archive adds 22 files, changes 38 and removes none. It introduces multi-datagram
helpers/APIs, changes IOCP completed-operation dispatch and resource-failure
handling, and applies ASIO_EMPTY_BASES to executor types. Platform feature/fiber
detection and several upstream tests/docs also change. The new APIs are not
enabled as application features. These vendor internals require fresh runtime
validation; equal public version numbers do not imply equal implementation.

The pinned detail/memory.hpp allocator helper is unchanged; retain the scalar
and aligned-CRT injection tests and the isolated MSVC-helper replay. Existing
shutdown/drain, timer, bounded-session, worker FIFO and network failure tests
exercise the active transport. No application source, protocol/SDK/listener,
public ABI, metadata resource or avatar policy changes are authorized here.
MinHook/PEGTL/TinyXML2 archive locks and payloads remain unchanged.

The existing tamper regression now derives its expected member count from the
lock rather than the previous hard-coded 1607. Its same-count/two-damaged-alias
assertions and all fail-closed archive checks remain enabled. Source/member
manifest refreshes record this specifically authorized replacement only.

## Review and validation

[Current record](../metadata/ASIO_UPDATE_REVIEW.json),
[archive delta](evidence/asio-update/ARCHIVE_DELTA.json), and
[validation](evidence/asio-update/VALIDATION.json) distinguish exact vendor bytes,
local builds and real Windows/MSVC CI. Game acceptance is separate.

The owner merged the prerequisite CI/review setup PR. This dependency update
now targets main in its own PR; it is not merged automatically.

Local LLVM-MinGW Release build and all 206 tests pass under Wine. The 12
selected Debug runtime/transport tests also pass. Source QA passes all 17
groups, including 19 vendor regressions without skips. Compiler dependencies
prove that the application transport uses the new installed Asio headers.
All three C exports/ordinals, all 3346 development export names, the static
CryptQueryObject import and both avatar payloads are preserved. Native
Windows/MSVC Golden CI and game acceptance of this snapshot remain separate.
Vendor-origin whitespace in generated reference documentation is retained
byte-exact; it is not silently formatted during installation.
