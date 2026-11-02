# Windows build-path component pinning

## Supported boundary

The checkout, its ancestors, build inputs, local operator and credentials are
trusted. Untrusted accounts must not be able to modify that tree. This is an
operator precondition; the build does not rewrite or certify the checkout ACLs.
Use a local Windows filesystem with normal directory-handle sharing behavior.

The protected entries are the checkout directory, `build/`, and each directory
component through the selected output leaf. A lexical confinement check still
rejects an output outside `build/`. Before acquiring `.build.lock`, the profile
opens these entries with `FILE_FLAG_OPEN_REPARSE_POINT`, reads their attributes
through the opened handle, and rejects reparse points and non-directories.
Missing child directories are created while their parent is pinned.

Read/write sharing permits ordinary build output. Omitted `FILE_SHARE_DELETE`
prevents selected entries from being renamed or removed while the handles are
held. The profile keeps those handles until after its exclusive build lock is
released. Acquisition failure releases the partial chain; successful cleanup
closes child handles before parents. A lock-disposal failure still runs pin
cleanup and environment restoration. Explicit CLR null removes newly introduced
environment variables across supported PowerShell/.NET versions.

## Native regression and negative controls

`build_path_reparse_contracts` executes the existing Windows PowerShell fixture
under the full Native Debug/Release CTest inventory. It owns a temporary checkout
and sibling outside directory, not the actual working checkout.

- Retain existing junction, ordinary-file and lexical escape checks.
- A separate process replaces a path after the parent validates it. The old
  precheck/lock/write sequence must demonstrably follow that junction into the
  **owned outside fixture**; this is an intentional RED control. Its two files
  are removed before testing the guarded sequence.
- The real pin/lock/write order must reject the substituted junction. The outside
  sentinel, its write timestamp and the directory's entries remain unchanged.
  Removing the junction itself must preserve the target sentinel.
- A second-process rename control succeeds before pinning, is denied for every
  selected component while pinned, and succeeds again after release. The lock
  file is not open during the rename-denial phase, so it cannot mask absent pins.
- An actual ACL denial on an owned temporary leaf must produce Windows error 5,
  restore that fixture's ACL, and release the already-opened parent chain. An
  ordinary-file failure also proves immediate partial-chain rollback.
- Execute the actual profile's parsed `finally` body with an injected lock
  disposal failure: child/parent pins still close, the original error propagates
  and the temporary environment marker is removed.

The test explicitly removes junction entries before recursive fixture cleanup.
The native test's 60-second budget and existing assertions are preserved. Local
PowerShell syntax/C# compilation or managed cleanup/actor controls on Linux do
not establish Windows sharing, ACL or reparse behavior.

## Limits and issue disposition

This is selected-component protection against entry replacement, not a general
filesystem sandbox or handle-relative implementation of all CMake/compiler I/O.
Generated descendants are not individually pinned. Administrator interference,
malicious build inputs, hostile same-credential code, and in-place reparse
metadata modification are outside the supported isolation guarantee. Ordinary
toolchain temporary-file behavior is separate from redirected build-output paths.
Runtime DLL code, public ABI, vendors and publication/promotion policy are unchanged.

Issue #16 acceptance must be evaluated against this stated boundary. Keep it
open until the owner reviews the exact-commit Windows test/real build results,
source-manifest and artifact checksums. A green portable run alone does not close
D326-F0005. The PR description records actual CI results and remaining limits.
