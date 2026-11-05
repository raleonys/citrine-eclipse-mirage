# Independent string, Parent and root consumer review

2026-11-05. **No blocking finding in the eleven files below. Static source inspection only; NOT RUN.** Compared each full diff with the captured before file and traced the selected producer/handoff and destructor callers. No production edit, compiler, project tests, linker, DLL execution, fresh REA query, ZIP or recorder execution was performed by this review. This author owns the separate Deques implementation, not these reviewed files.

The Parent full-map predicate requires a Live, nonbusy typed Map with the exact array address AND element count. It returns no borrowed catalog or extra view pin. `relinquish_operation_view` requires an admitted actor and an unambiguous live range, drops only that actor's deduplicated whole-ticket view, and retires its metadata node outside the gate. SDK borrows and other actors' view counts remain intact. It is a last-use declaration by the real owner, not a new actor permission or raw allocation transfer.

Concurrent Strings never grows or traverses its legacy header vector. Requested large headers resolve through the exact Parent owner and retain the existing unsigned alignment test; small-string paths still validate admission. The legacy ledger, duplicate/free-row handling and live-span publication remain Serialized behavior. Concurrent bindings/live_headers/bind_header_views explicitly reject rather than fabricating an empty ownership/drain result. Relinquish is a Serialized no-op; Concurrent SSO requires no heap release, while heap capacity+1 is checked against a live Parent range without clearing or freeing the string.

Root's empty initial header span is used only during construction. The exact resolver is installed into both frame and finish contexts before the completed application is published; normal Serialized binding observers remain. `prepare_job` relinquishes the moved request only after adopting its offer and matching the exact handoff, within the existing retained-failure try block. In the selected source path, later launch/detach identity checks read closure owner/function metadata; frame cleanup sees the moved local as SSO and rearming uses the separate input buffer. No producer span of that request is used afterward before child eligibility. This is source last-use proof; fresh native timing/ownership comparison belongs to root's separate evidence and is not inferred merely from this placement. Other ownership handoffs are outside this specific source-change review.

All three remaining socket destroy helpers now use the same authoritative header lookup. Null resolver preserves legacy miss/ambiguity behavior. Capacity+1, threshold4096, overhead39, data-8 lookup, unsigned alignment failure and sized-free/reset ordering remain at their prior source points. The three added tests exercise actual message, residual response and owned-response discard cleanup against stale header catalogs. Miss/Ambiguous stop without fallback/free while preserving already scheduled, moved or early envelope/control cleanup prefixes. Existing cases remain registered. These are controlled typed fixtures, not executed Parent lifetime or parallel actor validation.

Actual production selection remains Serialized. Arbitrary shared string/queue mutation, remaining per-message Profile/Graph/provider state and complete same-session progress are not established by this checkpoint. Consistent rebuilding and execution still require the user's explicit instruction.

## Reviewed final bytes

Paths are workspace-relative; all before proofs are in this stage's `before-manifest.json`.

| Author / file | SHA256 |
|---|---|
| SourceReview — `outputs/anadius-session-memory-source/memory.hpp` | `32e37805f2b01e992b4607a25bdff114dcac3f71d43280c3b48ffaed21dcdc19` |
| SourceReview — `outputs/anadius-session-memory-source/memory.cpp` | `84cb386fa4f0093c431376bf14e292e379c19777ec2f9d51577e6a7769987276` |
| SourceReview — `outputs/anadius-session-profile-source/strings.hpp` | `444f6d71099926110e8781309f2fca9398dad5350ff6e6d0bfd34f9c3577053c` |
| SourceReview — `outputs/anadius-session-profile-source/strings.cpp` | `ac810821216394fcfbb741e5639f866fded3dc15ef2d3179d86a15d4eff52bed` |
| SourceReview — `outputs/anadius-socket-source/message_dispatch.cpp` | `a406a1c96205865e25a13be944ba77a1a0404c0455022661e2a2e5fe4c69acaf` |
| SourceReview — `outputs/anadius-socket-source/response_dispatch.cpp` | `20a45675686aaaa6fcc243de1b5ff7350277dac6f21f2dc01fd911c48e6e0214` |
| SourceReview — `outputs/anadius-socket-source/owned_response.cpp` | `59f6e5610bb81faab3df29a31211e45a77bb8cd3e2f0985320dcc34239af019c` |
| Root — `outputs/anadius-session-profile-source/profile.cpp` | `7413d158e406bdcdca2b308f495dbc4a71db265df313566f17e741f4cb892676` |
| Root — `outputs/anadius-socket-source/message_dispatch_tests.cpp` | `5980c32365669191912281ec39e7ccbc74dc9cddff27a53e43313504694f87e2` |
| Root — `outputs/anadius-socket-source/response_dispatch_tests.cpp` | `b35b224166ecc0403fa64f0e61c218db0ed2be2c46bb79e71af7f14cf47ac0a4` |
| Root — `outputs/anadius-socket-source/owned_response_tests.cpp` | `a8d5a1ca491d13081ad693ab9ae5513fb4450f65d7e8548e2cf65621e38a90be` |
