# Next selected capacity gaps — read-only source triage

The session-registry stage remains unchanged. This note records current selected production restrictions for future fresh-native comparison; it is not a new original-binary observation. No code/test edits, REA session, build, project tests, link, DLL execution or packaging were performed by this triage agent.

## 1. Fixed I/O operation registry: highest next value

`anadius-server-runtime-source/resources.hpp` and `runtime.hpp` default to64 operation slots. The selected server installs OwnedIoMemory with that capacity and Runtime constructs matched OwnedAcceptMemory/CoherentIoMemory. `anadius-io-memory-source/io_memory.cpp:125–131` searches the fixed `records` array and throws std::bad_alloc when all entries are occupied, before calling the actual parent allocator. This is an artificial registry exhaustion, not an observed allocator failure.

Ordinary concurrently connected peers each retain a pending receive operation; active send/posted responses and accept/cache records consume additional slots. Thus a few dozen live peers with pending I/O can exhaust the64 entries even after session rows grow. The exact first failing peer depends on active read/write/cache records; it should not be reported as invariably peer65.

Fresh native selectors: async receive021d10, async send021fd0, generic operation allocation0269b0 and cached handler allocation021900, plus native free04b788. Confirm original per-object allocation/cache behavior and absence of a64-record check before implementing.

Growth risk: records already use unique_ptr and can keep operation/projection/OVERLAPPED addresses stable. However generations, storage bindings and dispatcher packet bindings are parallel fixed vectors. AsyncReceiveOperation.packet and AsyncSendOperation.packet point directly into the packet table; existing operation pointers must be repaired if that table moves. Borrowed storage/packet spans occur in Runtime/accept/dispatcher composition. OwnedAcceptMemory also has its own matching projection tables and must be audited together. Increasing a constructor default or resizing records alone is insufficient.

## 2. Layered frame-job/physical-thread/offer limit64

`anadius-application-graph-source/graph.hpp` defaults jobs_capacity64 and the actual resources factory selects NativeJobs with that value. `anadius-session-profile-source/profile.cpp:355–362` has a fixed Job vector and closure-binding vector; a65th simultaneously live frame job throws `Fixed frame-job capacity exhausted` before its native thread launch.

There are two additional selected layers: `anadius-owned-jobs-source/jobs.cpp:170` throws when its fixed physical rows have no Empty entry; `anadius-worker-handoff-source/handoff.hpp:26` sets maximum_job_borrows64 and `handoff.cpp:105` rejects a65th retained offer. Logical completion does not immediately free a physical row while a detached child/FLS exit remains live. A finite burst across many accepted peers or delayed child exits can reach this cap. InlineJobs has a similar indexed fixed table but is not the actual selected NativeJobs production backend.

Fresh native selectors: frame completion025480 allocates the48-byte closure and launches05e8e0 at entry021770, with detach04a458. Compare native per-frame allocation/launch against the selected three-table restriction.

Growth risk: NativeJobs passes the actual address of its Row as the thread argument, so vector<Row> reallocation would invalidate running or physically exiting children. It requires stable row objects, not just republished spans. Profile JobToken slot/generation and Job references must remain stable; its `message.closures` span must follow any closure-table replacement. PreparedMemory keeps shared JobState objects, but its outstanding-offer limit and constructor validation must be changed coherently. This is a larger, coupled change than the I/O metadata restriction.

## 3. Fixed environment capability ledger4096: defer behind the above

`anadius-owned-environment-source/environment.hpp:16` defaults4096 logical references. `environment.cpp:137–138` and164–165 throw when the fixed Control::rows ledger is full while creating or copying valid environments. Session environment roots, pending I/O copies and response executors all consume these rows. Native source paths clone/move environment objects per operation rather than using this source capability table, but that comparison still needs fresh023dd0/023cb0 evidence.

This cap is selected and real, though less likely to be the first ordinary failure while the64-operation/job caps remain. Its table is private: exported configuration vectors describe shared executor/vtable tokens, not spans of Control::rows, and registered projections retain capability values plus actual environment addresses. No Row address is passed as an OS thread argument. The main risk is allocating while Control::gate is held and preserving exact query/move/copy capability semantics; introducing allocator callbacks under that lock would be inappropriate. A separate prepare/postcheck/commit growth path could likely remain local to the environment provider.

Recommendation: fresh-audit the actual I/O allocator/borrowed binding graph first. Keep physical job rows and offer growth as a distinct later change. No new unload, readiness or lifetime-attestation framework is implied by these candidates.
