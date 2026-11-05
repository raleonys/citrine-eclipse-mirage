# Actual Root message copy caller implementation

Source-only frozen change. No compiler, project tests, linker, DLL execution or ZIP update.

Root owns io_completion.hpp and message_dispatch.hpp/.cpp; the Environment author owns its implementation and the SDK author owns the two test files. Independent review covers this exact six-file freeze.

| File under outputs/ | SHA-256 |
| --- | --- |
| anadius-socket-source/io_completion.hpp | 337ab5ee731caa9c7f19680aa01beb3225876dd11c1c4eca7a1f3dc68cb01c5b |
| anadius-socket-source/message_dispatch.hpp | 409c366a32ece93070af656ccfd636b07ae625eece42551ec7309c7dac364536 |
| anadius-socket-source/message_dispatch.cpp | e7c259b62931308a813e0e2be1821bad2ab477f6dc9adaa3a1f16aacd904af94 |
| anadius-owned-environment-source/environment.cpp | d3b83e35e3cb1237b25398c35f14a29f3eab7131f893e478b5e1d8e1e02ba9cf |
| anadius-socket-source/message_dispatch_tests.cpp | edaaa9c3601fa6ee5f6e973ab959f8d3b9c73c6a46be6fb1c322d0751ce6a5f5 |
| anadius-owned-environment-source/environment_tests.cpp | 654fb74592b875290cd563e06bfea06aa45228ca20a64f95637a77aa8150d3f4 |

The explicit stack IoEnvironmentCopyBody/Slot HOST interfaces let the selected owning provider surround the actual caller. They are not recovered native ABI/vtable slots. The optional table hook defaults false; a null source table follows the existing prefix-before-lookup failure route. A handled hook runs once with its admitted bridge; a declined hook runs the same body once normally.

copy_message_environment_028d30 retains the original order: destination operations/schedule prefix, callback lookup and cache, slot08 invocation, LIVE source executor load/store after callback returns. Both first and second actual dispatch_prepared_response copy segments now call this helper. Other task/control/string/response ordering is unchanged. The const-span overload uses the same body without manufacturing unrelated fixture dependencies.

The owning Environment reserves source/read and destination/write before prefix, publishes payload/state through the existing same Effect, checks callback outcome while the destination executor is still legitimately partial, then validates full destination only after the actual caller tail. Claims and metadata-node destruction extend beyond the tail. Unknown outcomes retain real prefixes and failed effect state; there is no rollback, duplicate retain, early executor write, ambient router or durable reference to the stack body.

Saved REA6.8/Ghidra12.1.4 Evidence ev_c1e35ff0c5c57064a1ddf6f2ba08016bd1eecedf05e9c21ef99d15c95c75e2c7 establishes first prefix180028e1f/180028e2a, slot180028e3a, live executor180028e3d/180028e44; second prefix180028e4c/180028e54, slot180028e64, live executor180028e67/180028e6b. The complete analyzed body is656 bytes in THREE ranges, not a contiguous658-byte body. Static evidence does not establish runtime or EH/ABI equivalence.

Direct raw slot08 remains a partial trusted projection whose external caller owns its tail. Production Parent stays Serialized. Positive actual two-Scope Runtime tracked-family/B-physical-reap/natural-child-old1 and actor IO/full native child tail remain open. No new native function credit or total completion percentage.
