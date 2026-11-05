# Caller-specific native evidence links

This is SOURCE INTEGRATION of already established static behavior. No new REA session/record, original hash observation, target execution, build or test is claimed. All native records below refer to the immutable anadius64.dll digest9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.

## Acceptor020e20: known operation/category domain

Retained files:

- outputs/anadius-socket-source/acceptor-evidence.json SHA256c164a9f54e056486786f3a61f599f359d648e210980c7c91743d55b701b93cd8.
- acceptor-evidence.pseudo.c.txt SHA2562a5a99ca92e2b2e64a133fdff0f0f5e3e2b698becad14d29bdd09630d9ac634f.
- acceptor-evidence.asm.txt SHA25686b720b8dc0848cf7261b6eaa4403921fc23bd8f00d60156055bc8d5ac4a8d31.
-020e20 pseudo EID ev_3c0f133878db1fec30ab6ef19ef7b2595fed8a0ba99ea8e0ec75a708ab5925af; complete retained assembly EID ev_ff338f1411812fb5f0cbb9ad6393b0698c2c0a40146ad6aab6f4aaeffaf2bfea.

Concrete branch evidence:020e7a calls0285b0 for already-open1.020efe/020f70 call028620 for failed socket/port attachment; later option/bind/listen and invalid-socket paths also call028620. The common raises load84728/open at02111f and call028240 at02112a;84730/set_option at021149→021154;84604/bind at021173→02117e;8473c/listen at02119d→0211a8. No generic-category nonzero output is selected in those native branches. Existing source acceptor.cpp SHA256c9a5c8a034daf5381d718f1985e8e3f58a530856982fff194fceed77299aea3a preserves capture/guards/order.

The exact four literal bytes were independently verified by the older read-only PE mapping proof work/acceptor-context-strings.json, SHA25624038d03d8ad7eb090084acb01e04efe5c5d3d4480c8c58a3e6dedfb77d1f704. It records open/bind/set_option/listen and exact file offsets. This is **PE byte proof**, not a new REA string record. Retained Ghidra string inventory EID ev_c97664560efc0e5cf3207e174b0defd0b96f84c49fdf20bd0644ce960d07d716 independently observed set_option/listen only; its missing exact open/bind entries were not treated as absence.

## Socket-service0228e0: second gate, not scheduler024990

- outputs/anadius-socket-source/service-constructor-evidence.json SHA2560c68e158f423fe2ba7e1ff01fbee0768deb1b6e01723ea868cacd787dec37e7a.
- service-constructor-evidence.asm.txt SHA256b522e1d20babc6e6c69553e772dde12eea27e67a7947a18ee8ef03c1bc536060.
-0228e0 pseudo EID ev_22c4bffe54fe0b986545326a4020802e4e98041a480e8cafc64b603ecd05d1ef; assembly ev_cccd315f694dc43aeae87a61fc8912482989a03586a119141e1b2a2d8f2bdd7b.
-027be0 pseudo ev_948db36a6b6838994310cbdd3ff325ea9d6c875b61a63f92c9c0d1ce03fdbe85; assembly ev_ebd0f0654f2c3bac442e8d0023199cc155873bebe20cb19ded69b206149bb469.

022945 calls027be0 on service section+50, caches EAX in EBX at02294a, then02294c calls028620. It stores status/category, tests nonzero at02295a/02295c and branches to022984 loading84428;022990 calls028240. Current service_constructor.cpp SHA256e4125fb99d7cbf61c29fd82ac5b7027f5face09a92e77890c66da96a8ec74413 implements the same nonzero status/system/mutex branch. OwnedServer initializes024990 first and0228e0 second; failing gate call2 selects this service path.

## Fresh supplementary work already recorded elsewhere

- outputs/anadius-owned-server-source/iocp-error-diagnostic-20261104/: four fresh records prove84428=mutex,84528=iocp and024990 nonzero system-category construction. Those scheduler address-based source fixes are preserved, not reimplemented here. The fresh84428 bytes also corroborate the service's same literal address.
- outputs/anadius-session-policy-source/worker-error-028240-20261104/: fresh028240→0248b0→00a270 proves supplied nonempty operation +`: `+supplied category.message. Its compiler-provider-review contains the actual selected LLVM-MinGW23.1.4 artifact evidence disproving default compiler system-message equivalence.
- outputs/anadius-session-policy-source/system-message-020950-20261104/: fresh028620 object/table binding and020950 exact SDK formatting.
- outputs/anadius-session-policy-source/native-system-category-20261104/: seven fresh records establish default-condition/equivalent identity/value domain; native-evidence-index.json SHA256ef4e925d09aa71068ca2d880ed5c847352d7b2aaf8e74f19365cffa81d8a7222.
- outputs/anadius-session-policy-source/native-stop-diagnostic-20261104/: reviewed module-owned imports-keyed native category. Current integration reuses its API; it adds no formatter or exception-lifetime mechanism.

No new body absence or algorithm gap is inferred from legacy helper names. Complete original ABI, unknown exception cleanup and other caller/category families remain separate investigation questions.
