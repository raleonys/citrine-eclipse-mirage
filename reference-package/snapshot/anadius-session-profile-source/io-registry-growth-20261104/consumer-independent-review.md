# Independent static review — live IO table consumers

Result: STATIC PASS for root's ten-file consumer diff in this stage. Reviewed each current source against its retained before copy. This review is confined to the new live-view consumer integration; earlier unrelated code in these files is not independently recertified. No compilation, project tests, linking, DLL execution or packaging occurred.

PostedEnvironment gains an optional pointer to an owner-held live storage span and current_storage(). DispatcherState gains the corresponding current_operations() capability. The new data members have default null values at the ends of their existing aggregate fields. Existing aggregate construction using only legacy fields retains the fixed-span fallback; public member functions do not remove aggregate initialization. By-value environments, transient copies and const Ports retain the live-view pointer rather than requiring writes into each copied context.

Runtime supplies the exact base OwnedIoMemory storage_view() when constructing the shared IoCompletionEnvironment and dispatcher_view() when constructing the whole-run DispatcherState. The viewed span fields belong to the stable IO Impl, which outlives the selected Graph/context copies. Base storage/packet relocation must update those fields before old allocations retire; that producer obligation is reviewed separately with base/accept implementation.

Both actual storage lookup loops (posted_operation.cpp and handler_dispatch.cpp) now resolve current_storage() at lookup. completion_dispatcher.cpp binding_for resolves current_operations() after opaque waits and epoch effects. It extracts the stable operation pointer and packet values before invoking the opaque completion, and never dereferences its metadata binding reference after that invocation can allocate, free or grow tables.

All selected identity checks in Runtime, Profile, Environment, InlineJobs, NativeJobs and Policy use current views. Policy compares both sides through current_storage(), avoiding a comparison of one refreshed side against a stale saved side. An exhaustive focused search found no remaining production .storage.storage direct accesses in these selected source trees. Read/send/handler/posted context copies inherit the live capability automatically; independent Profile environment-source bindings remain a different table.

Runtime Router still captures operation/OVERLAPPED/generation identity before completion_owner, then reacquires the current packet table afterward and checks the same slot/generation/operation before dispatch. Its post-callback lookup does not retain an old packet-span reference across that callback. Borrowed snapshots in external fixed fixtures are not silently made self-refreshing unless the optional owner view was supplied.

No blocking consumer defect was found. Existing Parent serialization, owner/context lifetime and exact operation finality remain required. This is not an arbitrary concurrency, native ABI or runtime passing claim. Base IO alias repair and accept sidecar lifetime are separate producer work and must both be present; this consumer report alone does not close those obligations.

## Final source SHA-256

| Source under outputs | SHA-256 |
| --- | --- |
| anadius-socket-source/posted_operation.hpp | 7c72379e35ef0f9993c1d643012eda189824d74f47c1e180104bc220ecde33f8 |
| anadius-socket-source/posted_operation.cpp | 6389460fc5978b609f4752581170eb8565cd1a8d959bd036692690760ede312a |
| anadius-socket-source/handler_dispatch.cpp | 512444bb40a57b21c1acd40274579c4d7a8acf70921482d01606658cbe6a15fc |
| anadius-socket-source/completion_dispatcher.hpp | 26c716c5a6f08d8f741b964ccb8e48719707799c655e447c3cbb8ed65f412f48 |
| anadius-socket-source/completion_dispatcher.cpp | 56a9c5386a1e1615b222f26da95d918f3aeef8666fd16fb64fc9bfba7ea1ab95 |
| anadius-server-runtime-source/runtime.cpp | d7bfd7dd438f2ef9fe5fc06512730d8dbff887e893ccddd5a264b3271fa954ae |
| anadius-owned-environment-source/environment.cpp | af5bb3c6c5b03714818d616759968b428afe96e6078e81d8b67b6324056be6dc |
| anadius-owned-jobs-source/jobs.cpp | c9d625565e89fd7605de34374f5548733d0b9a5603912a0366c2c9405c70a7cd |
| anadius-session-policy-source/policy.cpp | ec4699374d4ef0aec381252fb6b435006c35b637e895ac6b7d63837a61524fcd |
| anadius-session-profile-source/profile.cpp | b66b79487d283416b6ad9082fbdb4527449212708133d70f603be1f1b40d5b9d |

Root confirmed these ten files stable before this hash capture. No source edits were made by this reviewer.

