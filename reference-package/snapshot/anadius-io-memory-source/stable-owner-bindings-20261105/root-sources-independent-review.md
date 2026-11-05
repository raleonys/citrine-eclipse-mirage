# Independent root consumer, test and recorder review

2026-11-05. **No remaining blocking finding in the eleven frozen sources and separate recorder below. SOURCE ONLY; NOT RUN.** Reviewed their full diffs against the captured before copies, the changed consumer paths and complete recorder text. No compiler, project tests, linking, DLL execution, REA call, packaging or recorder execution was performed by this review.

Concurrent Runtime construction propagates the exact IO owner resolver through by-value Posted/IoCompletion environments and DispatcherState. Admission compares that exact owner identity through OwnedAccept rather than reading replaceable spans. Serialized still uses its current legacy tables and rejects an unexpected owner resolver. Profile, Policy (both supplied and retained Ports), Environment and both Jobs bindings use this same mode-aware authority check. CoherentIo's phase admission no longer requests a forbidden union catalog. The extra Runtime write-context resolver check applies only to Concurrent mode, preserving the historical HOST admission domain.

Review found that the first Runtime Router candidate used the owner resolver even in HOST mode, changing its prior indexed first-match behavior for duplicated public packet metadata. Root corrected that branch: Concurrent resolves the actual typed operation; Serialized retains the prior `dispatcher_bindings()[before.slot]` check. The before/after handle generation and operation/OVERLAPPED checks remain before opaque completion, and no packet is read after that completion can end its lifetime.

OwnedAccept projection, retry and submission use the exact base packet accessor, including the distinct accept overlay on a base Posted reservation. Retirement captures the actual old packet pointer before alias unregistration; it does not traverse a moving table or dereference a potentially stale storage descriptor. The alias owner validates/forgets the field before variant destruction. The adjacent Stop preflight was added after SDK review: `storage_capacity()` observes CURRENT base registry coverage, so a factory that grows IO during binding is not measured only by its initial capacity. Existing receipt limits and later stop algorithms are unchanged.

The two socket cases exercise actual021820/021900 cache hits against stale same-identity legacy storage and actual027c10 key0/key2 authoritative packet selection. Miss/Ambiguous prevent fallback, callbacks and work release; the legacy duplicate cases retain first-match behavior. Existing cases remain registered. These controlled resolver fixtures do not establish actual Parent admission, concurrent Record lifetime or end-to-end Original message progress; the IO producer and real handoff fixture have separate reviews.

Recorder v3 is a separate metadata helper. It verifies the captured absent-before manifest for a newly added header, records no fabricated build hash (`build_sha256:null`, added flags), hashes that baseline manifest into the evidence, checks unchanged unlisted prior pending sources and preserves frozen build/DLL/ZIP hashes before and after metadata writes. Existing added pending headers can later be modified using their actual prior SHA. It never rewrites the historical source snapshot or launches project code. Current v2 still matches the prior evidence's SHA256 `ee58e3403d744f28f94e7e439b0346d05581f6fb8744663791012ba08fab98c7`.

Actual production Parent selection remains Serialized. Stable owner descriptors do not supply lifetime pins or synchronize shared payload mutation; upper NativeJobs/Profile/Graph/provider and OwnedAccept body serialization remain separate integration work. No complete native ABI, whole-DLL equivalence, successful build or executed concurrency result is claimed.

## Reviewed final bytes

Paths below are relative to the workspace. Before proofs are in this stage's `before-manifest.json`; Stop's separately captured before copy was added before its one-line change.

| File | SHA256 |
|---|---|
| `outputs/anadius-server-runtime-source/runtime.cpp` | `8b5886df6188e8781eebc9a66c5104cc7a02eb2c990b7fe74c800343ae5d2080` |
| `outputs/anadius-session-profile-source/profile.cpp` | `dc40777b05ffce8698fafea48734b0c876df995a824deaa1aa4385e5ddfa42ba` |
| `outputs/anadius-session-policy-source/policy.cpp` | `e71bdc060cb62baf04c6354197beca04022c928f0ba257cc6429ce0a4164de0c` |
| `outputs/anadius-owned-environment-source/environment.cpp` | `26c07a1458233c803aeffb2026aec203f84b590c075add0b43741b9a84fd5100` |
| `outputs/anadius-owned-jobs-source/jobs.cpp` | `8973b5c6b1346fce65fcbf417f5b011919abc9f4bfd3cc9673a692552da3bfbf` |
| `outputs/anadius-accept-source/owned.hpp` | `9f71991d8e4222891b769870eeb394f54c7a0b278ce7a2fa87faefeb700f8316` |
| `outputs/anadius-accept-source/owned.cpp` | `dc8c5dde22e13b599c57260adfd5bbbaa0575e778b5e92167be16c9095abcf0e` |
| `outputs/anadius-session-flow-source/coherent_io.cpp` | `2d02a237b375931814dc772f733b38ae78f5dfa080aeaf4a0ecffaa1da25ee98` |
| `outputs/anadius-accept-stop-source/stop.cpp` | `8906506358deff204eecdddfd1fce4b4b3c4ec9a1456e84bf1fab696686297a5` |
| `outputs/anadius-socket-source/posted_operation_tests.cpp` | `ffda109ff756595702e1496a92bccee203589427b0b50fae196216e0baa0405c` |
| `outputs/anadius-socket-source/completion_dispatcher_tests.cpp` | `13db5c76bb8a048b1900e5cd90ecd1b228698eb778d17a64d0ba3b52beaceac7` |
| `work/record-focused-source-stage-v3.py` | `db4232eb1a64b90413278f87c785dc82e8102618db3d05cbdde08d426ccda414` |
