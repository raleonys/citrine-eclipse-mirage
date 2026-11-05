# Independent static review — session growth test sources

Result: STATIC PASS for the final three test-source diffs. No compilation, project tests, linking, DLL execution or packaging was performed. Test-source review does not imply passing executable checks or runtime equivalence. The previously recorded production review `independent-review.md` was left unchanged (SHA-256 `80a215ef92d40b9e129336baf901d96634ac6ddf3b27128f4624f283b6f3d6b2`).

## Reviewed scenarios

`profile_tests.cpp` adds twenty simultaneously held factory-backed owners with initial session allocation16. The test uses the existing accepted-owner factory and greeting composition; it is not twenty controlled socket clients. After each potentially growing greeting it reacquires RegistryView. Saved binding records contain copied scalar values and pointers to stable Slot-owned subobjects, not saved spans into retired routing arrays. The checks retain old token/object/stream/environment identities, inspect current frame/read/write contexts, retire the first owner while nineteen remain, reject its stale token, reuse its slot with a newer generation, and retire twenty current owners. Twenty-one total owner generations are expected to be destroyed once.

The first control's strong reference is released once before reap; its dangling SessionPair is overwritten before reuse. The remaining controls are released once at final retirement. Profile's existing weak-observer cleanup and actual factory disposal are used rather than manually deleting embedded owners. The inline short greetings do not introduce an unretired heap payload. No retained donor or queued job is created by this core growth scenario.

`profile_integration_tests.cpp` changes the existing two-client composition from initial allocation2 to1. The second peer forces growth while the first owner's deferred raw job and exact source identity remain retained. The existing controlled accept/factory/greeting/read/job/send/EOF/reap pipeline and packet/free/teardown assertions remain. The first receive's initial1 assertion occurs in the synchronous first receive setup before native accept rearm; the second receive reacquires the current registry.

`graph_integration_tests.cpp` similarly changes the existing plain/encrypted two-peer pipeline from initial allocation2 to1. This exercises both Profile's arrays (1 to2) and Graph's independent Logic rows (minimum growth allocation8). It does not assert that Graph itself grows to only2 rows. Existing old/new IO continuations, response/event FIFO checks, retained owner assertions and prepared/native isolated scenarios are preserved.

Only obsolete fixed-session-capacity child mode/exit211 and its invocation were removed. The positive twenty-owner core scenario replaces that outdated rejection. Duplicate registration212 and the remaining original retention modes through222, including fixed job-capacity219, remain. The source main text correctly changes twelve isolated cases to eleven. Existing fixture cleanup and packet count expectations were not weakened.

No blocking static defect was found. Failure injection for every allocation prefix, twenty real clients, prepared-child activity concurrent with registry relocation, exact native allocator/ABI behavior and executable outcomes are not established by these additions.

## Final test-source SHA-256

| Source | SHA-256 |
| --- | --- |
| anadius-session-profile-source/profile_tests.cpp | 3f760b2f42bc0af68ccbeb6fda21813307705a18bf66ab7fbe6b97011d75046e |
| anadius-session-profile-source/profile_integration_tests.cpp | 8df185be573ce8a7457900ac8701c10579d5b5d46e163cd120773f301ab3953b |
| anadius-application-graph-source/graph_integration_tests.cpp | 147d3d14e28ee7a70fb07048ce15b69c0642eac1cb346112c67bffc165e3562c |

Author confirmed these bytes final; reviewer independently rehashed them before recording this review.
