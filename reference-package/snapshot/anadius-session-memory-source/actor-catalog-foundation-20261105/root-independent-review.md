# Independent source review: resolver boundary and actual lower-actor fixture

2026-11-05. Root review of sources authored by the two subagents. All findings here are static source inspection; NO compiler, project tests, linker, original/reconstructed execution or ZIP operation.

## Resolver production boundary

The optional fields are appended to existing aggregates with null defaults. A nonnull typed resolver is authoritative even on Miss or Ambiguous; outputs are committed only on Found. Legacy span traversal, null-map address0, byte-count divisibility and ambiguity failure behavior remain available under null resolver. No borrowed catalog is inspected on the authoritative route. A descriptor itself does not mint authority or keep payload alive: the integrating owner must compare resolver identity and hold exact actor/resource ownership.

The frame025480 and write025a70 destruction branches still read the current capacity/pointer, compute threshold4096 and overhead39, check unsigned gap8..39 through the same invalid-parameter provider, and call sized-free before later mutation/continuation. The shared header helper changes only lookup. It does not swallow owner admission failures, perform native allocation/free, or add a lock around an application effect. Metadata preparation by the owner can allocate outside its gate. Actual production assembly is not switched to concurrent actors here.

## Authored lower fixture (not run)

The new handoff case reuses existing actual NativeThreads, PreparedMemory/Borrow and JobMemory/JobScope, rather than setting PTD/work/readiness counters. Original creates two offers, A remains explicitly admitted and pins only its73-byte range; B owns a different real PTD/offer and grows seventy independent typed maps plus an aligned large map. A failed93-byte allocation callback waits outside the heap ledger mutex while B publishes/holds an89-byte allocation; this distinguishes individual known failure from the old global-count rollback. The absent source new-handler follows the normal bad_alloc branch.

B first invokes A's actual still-live Operation::job_permit from B's different thread, requiring denial before PTD validation and no Parent poison. B's foreign free of A's actively borrowed range must reject without poisoning. B's own views/maps must be physically freed immediately, then its copied permit expires and its actual native wrapper/FLS exits. Original joins/reaps B and a canceled additional offer before releasing A. A's exact view and seal are rechecked afterward; close returnsfalse while it is still admitted. Final A return/native join/reap and actual range free allow ordinary Parent/Borrow/controller retirement. Finite waits terminate on failure with owners retained; a blocked B cannot pass after releasing A. Legacy Serialized cases remain registered.

No blocker found in these inspected final sources. Parent implementation and its combined lifetime/reservation review are separate mandatory work. The direct map case is not actual026670 or Graph/NativeJobs coverage; root's separate socket fixture exercises actual026670. Full same-session/distinct-session UnknownMessage/GetConfig progress still requires per-message Profile/NativeProfile/Graph/providers, stable IO catalog conversion and removal of the existing NativeJobs gate after coherent integration. This checkpoint does not close that native mismatch or establish runtime fidelity.

## Reviewed exact bytes

- `anadius-socket-source/receive_buffer.hpp` SHA256 `607f158bb54fb305cb40e26ac2939e0d0dee5099fd64e78561f689f7c7ad9503`
- `anadius-socket-source/receive_buffer.cpp` SHA256 `aedb4023d0a3e9ed55acd9d592eaca70c08c8bed0d21e6082e3d474ab9435398`
- `anadius-socket-source/write_finish.hpp` SHA256 `8f232392a79da93f1f0cba1093ba0f21824b9d77a1f507452618fb904213cff3`
- `anadius-socket-source/write_finish.cpp` SHA256 `a25e6ffba62e3ed15cbc93a5d0a900c98a438384eac80d22e6bf445af786956d`
- `anadius-socket-source/frame_read.hpp` SHA256 `2cac0dc94fa28520e617b17bcc50957014fb5b8b20be7e1393ae0d6a3a9858e1`
- `anadius-socket-source/frame_read.cpp` SHA256 `435d5884532583c5c7e7cb40a62315e3becce0aea47f21197efd105d8bd1177c`
- `anadius-socket-source/response_containers.hpp` SHA256 `bc28f179d0ba6747162689468f5c30c427cb6274c95ce0b1936835a06d19a69a`
- `anadius-socket-source/response_containers.cpp` SHA256 `17a39127e9e9a8fe53e269859c6dfff807200a5dea881cebf76151cee4bc6e10`
- `anadius-worker-handoff-source/handoff_tests.cpp` SHA256 `23fedea9c99480b33ef8e8c5b776effa9ef168c0d12e109b22c3239bad837f30`
