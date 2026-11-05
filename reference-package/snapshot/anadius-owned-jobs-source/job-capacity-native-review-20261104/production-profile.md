# Profile job registry growth — source implementation

Scope: only `outputs/anadius-session-profile-source/profile.cpp` and `profile.hpp`. Original binaries and earlier frozen stage results are unchanged. No compilation, tests, linking, reconstructed-DLL execution, REA call or packaging was performed by this implementation task.

## Native basis

The seven fresh native records already saved in this directory show the selected normal per-frame path, rather than a 64-row native registry. At `0x1800254ed` the requested closure size is 0x30; `0x1800254f2` calls04b0bc, `0x18002554b` selects021770, `0x180025556` calls05e8e0, `0x18002557f` detaches, and `0x180025607` restarts0280d0. The examined025480 body has no 64-job admission check. The thread wrapper prepares a per-thread block at `0x18005e91d` and calls CreateThread at `0x18005e94c`. Thread entry invokes the stored function at `0x1800217aa`, cleans exit waits at `0x1800217ac`, and frees the closure at `0x180021802`. See `native-findings.md` and the adjacent complete pseudocode/instruction replies for evidence identifiers and limits. This supports removing the reconstruction's fixed64 admission ceiling; it does not imply unlimited memory or OS threads.

## Production change

`jobs` now uses stable-address deque storage. On exhaustion, `grow_jobs()` doubles the row inventory subject to both containers' actual `max_size()`, prepares a replacement closure-binding vector, checks current source admission and shape, appends empty Job rows, checks again, copies the current closure bindings, and publishes the replacement `message.closures` span before destroying the old metadata. Existing Job references, tokens, generations, strong pins, and handoff objects keep their addresses and identity. Appended unpublished rows are rolled back on failure; no live job is replayed, guessed complete, or freed as recovery.

A bounded `growing_jobs` reservation covers both growth allocations and old metadata cleanup. Frame allocation rejects entry while it is active through the existing poisoning effect boundary. This prevents an allocator/new-handler callback from selecting a newly appended Empty row while the published closures table still has the old length. RAII clears the reservation on every exit; a callback swallowing that rejection still leaves phase poison for the outer admission check. Existing nonempty token paths cannot select the unpublished Empty rows. The reservation adds no concurrency or unload policy.

Frame allocation first allocates its local 0x30 payload, then checks admission and selects the current Empty row. It reselects after growth and commits without another opaque allocation. Thus no Empty row reference is retained across an allocator callback. Existing native-owned failure-prefix policy remains: errors poison through the established effect handler rather than releasing existing live strong pins or jobs.

`after_completion()` traverses the initial row count by index because a callback may grow the deque: its elements remain stable, but its iterators would be invalidated. Newly appended jobs wait for the next traversal. The metadata-growth path preserves Win32 LastError across allocation, rollback and retired-vector destruction, using the existing `PreserveError`. `jobs_capacity` is documented as initial storage alongside the already dynamic session and header counts. The separate initial-allocation validation policy is unchanged.

## Final source hashes

| File | Before SHA-256 | After SHA-256 |
| --- | --- | --- |
| profile.cpp | c45a3535583b0e8827b594c92a192ee209713e74746b49512f31b3d3f6e71a6e | 3dbc5f43dd4df0f25be150c361a034183ea00321edf8c7ca99bdc0503a695909 |
| profile.hpp | ad4b9f2e74b1ba01f34e7a32f9b9a167373625b72a381d9fce452c054b5dc07c | 31ffac0c570f34e20cf6be3c14af199d6f4b63a3d37c88a7602e9da80079057f |

NativeJobs row growth and PreparedMemory offer growth are separate coordinated changes owned by other tasks. This Profile change alone does not establish a complete selected-chain correction. Static review is not executable validation or full C++/native EH/CRT/ABI equivalence.
