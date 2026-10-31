# D326-F0005: pinned build directories

## Trust boundary

The repository checkout, its parent directories, the local build operator and
their credentials are trusted. Untrusted accounts must not have write access
to the checkout or its ancestor directories. The checkout must reside on a
local Windows filesystem with normal directory-handle sharing behavior.
Administrator-level interference, malicious build inputs and processes with
the operator's own write credentials remain outside the supported isolation
guarantee.

A lexical prefix check and a one-time junction scan are insufficient when a
directory can be replaced before build output is written. For Windows build
profiles, `Enter-PinnedBuildPath` opens the repository directory and each
component through the selected build output with read/write sharing but
without delete sharing. Missing child directories are created while the parent
handle remains open. Reparse points and non-directories are rejected by
inspecting the opened handle. All handles remain open until the build profile
releases its exclusive lock.

## Limitations

This pins selected directory components only, not every descendant generated
by CMake or other build tools. It does not enforce filesystem ACLs, defend
against hostile processes inside unpinned descendants, or provide a complete
handle-relative Windows path traversal. It is not a general filesystem sandbox.
Runtime DLL code, vendors, exports, and release promotion are unchanged.

The Windows-only negative test exercises a pre-existing junction, replacement
after the earlier lexical check, a live pinned-directory rename attempt,
and a local lock that must not escape the build root. Those tests do not cover
every concurrent interleaving or native build variant.

Issue #16 must remain open until the exact commit passes Windows-native tests
and the owner reviews the remaining adversarial path-swap, denied-access,
and output-confinement evidence. Portable source checks cannot certify
Windows security behavior.
