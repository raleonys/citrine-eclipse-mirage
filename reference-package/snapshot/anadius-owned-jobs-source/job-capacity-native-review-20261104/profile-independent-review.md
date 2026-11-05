# Independent static review — Profile job registry growth

Result: STATIC PASS for the final Profile production diff after one blocking review finding was corrected. This review covers `anadius-session-profile-source/profile.cpp` and `profile.hpp` only. The reviewer authored the separate NativeJobs/PreparedMemory changes and does not label those independently reviewed here. No compilation, project tests, linking, DLL execution or packaging occurred.

Read the fresh native findings and seven-record saved evidence bundle supplied by root. The025480 per-frame0x30 allocation (`0x1800254ed`/`0x1800254f2`), launch (`0x180025556`), detach (`0x18002557f`) and read restart (`0x180025607`), plus021770 closure free (`0x180021802`), support removing the examined source fixed-job admission rejection. They do not establish unlimited OS threads or exact native allocation/CRT/EH behavior.

## Stable records and coherent publication

Job records now use deque storage. Existing JobToken indices/generations, heap closure identities, strong-owner pointers, prepared handoff ownership and Job references retained across start/detach/prepare calls remain stable when Empty rows are appended. The separate contiguous closure-binding vector remains the sole moved routing table. Its replacement is allocated before current records are copied; source admission and shape checks follow allocations and append. The helper publishes `message.closures` immediately after swapping vectors, with no opaque call between the two operations, before the old metadata storage is freed.

The append rollback guard removes only unpublished Empty rows if a precommit check fails. Existing pending rows, closures and strong pins remain untouched. Payload allocation now happens before choosing an Empty row, then admission is rechecked and a current Empty index is located. Consequently an allocator callback that publishes another complete job during payload allocation cannot cause the outer allocation to overwrite that same row.

Normal begin-thread still acquires the exact existing session/control strong reference, constructs the same token, and retains all unknown prefixes through the existing source effect/poison rules. Detach, closure free, generation exhaustion and handle-identity checks are unchanged. No false native completion or source rollback was added.

## Blocking finding and correction

The initial diff allowed a deque-block allocator/new-handler callback during `jobs.resize(grown)` to reenter Frame allocation. Newly appended Empty Job indices could then be selected while `closures` still had its old size, producing an out-of-bounds publication before the outer shape check.

The final code introduces a bounded `growing_jobs` reservation before either growth allocation. Frame allocation rejects that state before allocating its payload. The rejection propagates through the existing effect poison path; swallowing it in a new-handler cannot make the outer `check()` succeed. The reservation remains set through rollback and old metadata destruction, and resets on all exits. It does not add a new unload/CRT framework or deny unrelated normal callbacks outside the incomplete-growth interval. This resolves the identified partial-publication defect in the reviewed source.

## Iterators and remaining scope

`after_completion` now records the original job count and visits rows by index. A `complete_job` callback may append deque rows without invalidating the loop's cursor or making it follow an unbounded set of newly created jobs. Other job scans either use indices or contain no opaque call while their iterators remain live. Stable references survive the existing custom backend/allocator boundaries.

No unresolved blocking defect was found in the final two-file diff. InlineJobs remains a separately bounded alternate provider; the actual selected production route uses NativeJobs. This review does not establish arbitrary concurrent access, original ABI/SSO/allocator effects, exhaustive allocation-failure execution or runtime passing results. Test sources are reviewed separately.

## Final independently rehashed source bytes

| Source | SHA-256 |
| --- | --- |
| anadius-session-profile-source/profile.cpp | 3dbc5f43dd4df0f25be150c361a034183ea00321edf8c7ca99bdc0503a695909 |
| anadius-session-profile-source/profile.hpp | 31ffac0c570f34e20cf6be3c14af199d6f4b63a3d37c88a7602e9da80079057f |

The Profile author confirmed these bytes stable; this is source-only static review.
