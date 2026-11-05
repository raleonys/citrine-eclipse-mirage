# Final static stage review

The two production findings in amendment-production-stage-review.md are resolved in the source identities below. No further concrete phase, lifetime, type, or concurrency defect was found in the reviewed production change or the inspected new test scenarios. This is a static source review using the same fresh Ghidra Evidence; no compiler, tests, reconstructed DLL, original DLL execution, ZIP work, or version.dll access occurred. The reviewer changed only audit artifacts.

## Final inspected production identities

| File | SHA-256 |
|---|---|
| anadius-foreign-callback-source/foreign.cpp | 5ae305476a8498a8d01b9ba0bb4ea86965f297d34bb92af0a8945664ca2f0077 |
| anadius-foreign-callback-source/foreign.hpp | e7afd4962ba0cd8adbbb3948dfeeae18c67ad2694988a9e76924b552d8d6b6d5 |
| anadius-native-source/native_callbacks.cpp | 7eb43063c67266775b94ff2c9f1509433e3114f326240f2e24cec309bd97569a |
| anadius-native-source/native_callbacks.hpp | 62924d694f1f7deac38e99c0d9f3ddd56a0b61df74439caea1b1b9adfe852ab7 |
| anadius-network-hook-source/connect.cpp | 73659fea38e311c87dd91e3311321f421483cf0f0ea9061d7e266f5469867f9d |

Full absolute paths, UTC modification timestamps, lengths, and test source identities are preserved in final-stage-source-hashes.json. Tests at this inspection: foreign_tests.cpp 4605bb17faa643f9064587fb305f30ea84a248999ea4a1f083d14061a79bc9f6; foreign_integration_tests.cpp 8d77c9411504263896b72429dbc242ffa821e91ba6d27a39d674b00f86de7e4f. These hashes identify review scope; they do not claim execution.

## Confirmed transitions

Non-web entries, including3216, reserve a counted Waiting phase after the successful hook receipt without invoking the bound-port provider. The actual native wait runs before completion binds the first actual port and selects Redirect or Original. Web entries with an actual payload choose Redirect/Original without a new wait. Unknown length/family entries choose Original without touching shared native body state.

The native bridge obtains the selected copied port, invokes the actual redirect helper while only body effects are permitted, completes the redirect checkpoint, and dispatches the original only after permission becomes Original. The cached requested port matches native DI's pre-wait value. Returning false for unsupported optional routes leaves no counted/TLS reservation behind.

Both failed completion checkpoints retain Checking, so catching their exception does not restore wait, body, SDK, or general callback authority. Exact leave accepts and drains that admitted Checking phase. Observer exceptions additionally poison the Domain. Permission is tied to the admitted TLS actor/fiber, not the general callback owner. Reentry remains rejected.

Source count and native Context Lease cover all phases, including failed checkpoint unwinding; leave precedes native lease release. Domain pause sees counted phases, denies new entries, and leaves existing actual waits/bodies able to finish. Setup completion does not wait on them. No Domain mutex is held across wait, provider, body, or SDK execution. Metadata preserves LastError, while real effect/SDK errors survive source/native release.

The shared redirect helper uses scoped relaxed atomic_ref load/local unsigned increment/store and compares the cached local result with15, matching observed native00fdc0 MOV EAX,[counter],00fdc6 INC EAX,00fdc8 MOV[counter],EAX,00fdce CMP EAX,0xf. All three callback routes use this helper, avoiding mixed atomic/non-atomic callback counter writers. Lock-free/alignment assertions are explicit. The atomic_ref lifetime ends before dialog/exit, and terminal15 still precedes caller port writes and SDK dispatch.

## Inspected test correctness

- Core phase tests count port-provider invocations and assert no premature non-web read. Failed checkpoints assert permissionless state and then leave before another admission.
- Cold Original non-web and3216 integration cases begin without a Server event, execute the actual controlled wait, then publish same-Execution Server/Online/Libraries events before releasing it. The held general callback and counted waiter retain distinct permission ownership.
- Concurrent redirect integration uses two real helper paths and a third HTTPS passthrough. SDK observation is serialized by its own mutex; array result/valid/tail elements have distinct thread ownership, and main reads them after joins. The concurrent SDK probe does not call the shared unsigned require counter.
- The observed private attempt pointer remains alive: all callbacks join before its ordinary raw read; close_after_cleanup closes binding/providers but retains Owner::callbacks_; owner.reset occurs afterward. No atomic_ref remains alive at that read. Allowing one or two retained increments is appropriate for the native separate load/store behavior.
- RedirectLogBuffer sets a LastError marker during sync, and the SDK observes that per-thread marker after body checkpoints. All concurrent streambuf mutations occur under the managed stream lease, and raw buffer inspection occurs after joins.
- The Original terminal source mode is registered in child and main dispatch. Its callback thread installs the thread_local UI fixture pointer, and its terminal observer checks body permission, one dialog/log prefix, unchanged caller port, and zero SDK calls. Normal return clears that thread-local pointer.

No external Original-profile test caller found in the earlier bounded audit requires the removed uncontended full/PTD path. Existing independent native-context, resources-factory Cooperative, and original-provider fixtures remain separate.

## Preserved review boundary

Cold80/443 without an actual Server-port payload still falls back to the ordinary Full/raw-configuration route. This final amendment does not reclassify that previously identified separate writer/synchronization boundary as resolved. The production restricted phase change adds no new all-ready or unload protocol.

Native evidence authority remains shipped-artifact/static:00fd00 instructions ev_839077f99c02612bc8d409358a72a00b4078cf9a201c931361fd8e1c4381c349;00fd00 pseudocode ev_4b237267ebf4f1563f67a6e5c3bea40fbee890a0e1fceb1b3425de5a06fb7204;036880 instructions ev_9f1de4375f3c44c8e0c59d95b90042240c92853434c9a941fc374236aa1bc138;036880 pseudocode ev_c9f5209a7f663688e2ca0c2ba7a8583e52b0afd75fb8a3ab48395e4c03aa1efe. Original target SHA-256 and Ghidra body/terminal-call limitations remain as recorded in review.md and the complete fresh-evidence.json.
