# Root six-file static integration review

No blocking finding for the bounded selected Root caller contract. No compiler/tests/link/DLL/ZIP was run.

| File under outputs/ | Reviewed SHA-256 |
| --- | --- |
| anadius-socket-source/io_completion.hpp | 337ab5ee731caa9c7f19680aa01beb3225876dd11c1c4eca7a1f3dc68cb01c5b |
| anadius-socket-source/message_dispatch.hpp | 409c366a32ece93070af656ccfd636b07ae625eece42551ec7309c7dac364536 |
| anadius-socket-source/message_dispatch.cpp | e7c259b62931308a813e0e2be1821bad2ab477f6dc9adaa3a1f16aacd904af94 |
| anadius-owned-environment-source/environment.cpp | d3b83e35e3cb1237b25398c35f14a29f3eab7131f893e478b5e1d8e1e02ba9cf |
| anadius-socket-source/message_dispatch_tests.cpp | edaaa9c3601fa6ee5f6e973ab959f8d3b9c73c6a46be6fb1c322d0751ce6a5f5 |
| anadius-owned-environment-source/environment_tests.cpp | 654fb74592b875290cd563e06bfea06aa45228ca20a64f95637a77aa8150d3f4 |

Reviewed the actual helper and BOTH selected message copy consumers, public HOST hook/body/slot declarations, owner implementation, component assertions, author reports and saved full native body. The helper defers lookup until after header writes and executor until after slot return. A false optional hook executes the ordinary body exactly once; owning true return prevents fallback. The owning same-effect bridge validates exact source/output/cached function and one entry/return, with partial-output checking before tail and full checking after tail. No stack bridge escapes into retained failure state. Surrounding response/task/residual/string sequencing is unchanged.

The message assertions observe refusal before prefix, fallback/handled routing, callback-time executor placeholder and live source mutation, after-prefix lookup failures and throwing-slot prefixes. The selected controlled Environment assertion uses its actual bindings, observes the same active effect before prefix/inside callback/after executor tail, and preserves capability/reference accounting. Existing raw callback and lifecycle source assertions remain. These are authored component checks, not an executed actual Scope/Runtime proof.

Independent review root-copy-independent-review.md agrees on this exact freeze. Direct legacy raw callback caller-tail obligations, actual two-Scope tracked-family Runtime progress/physical reap/old1, actor scheduling authority, full native child tail, native ABI/EH and runtime equivalence remain open. Original DLL and frozen build/delivery proofs are checked by recorder, not relabeled current validation.
