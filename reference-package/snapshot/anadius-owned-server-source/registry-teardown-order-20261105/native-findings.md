# Fixed Original registry teardown order — 2026-11-05

**Confirmed selected-path gap:** native deletion of the socket-service gate precedes scheduler-gate deletion, which precedes IOCP CloseHandle. Current Original destroy_context closes IOCP first and raises on a false CloseHandle result; the measured destructor ignores that result. Empty-service shutdown also enters/leaves its service gate, and scheduler shutdown atomically sets its +3c field to1 before its zero-work exit. These are actual normal/cleanup effects, not merely absent function names.

No production/test/status edit, compilation, tests, linking, target execution or packaging was performed. The preceding four-file shared-Winsock implementation stayed frozen.

## Fresh identity and scope

REA6.8.0 configured server `C:/tools/rea-6.8.0/node_modules/rea-agents/scripts/rea.mjs`, available Ghidra12.1.4, immutable original SHA256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Own isolated session queried only two16-byte tables and four actual slot bodies (pseudo+instructions): **10 records**, exported **64211 bytes**, zero unknown records, then normal close. Original SHA256 after close is unchanged. Full raw replies, EIDs, parameters and file digests are in `native-evidence-index.json`. This stage did not repeat 029ea0,024990 or other constructor bodies merely because they occur in the chain.

## Measured table bindings

| Final service table | +0 deleting slot | +8 shutdown slot | Fresh byte EID |
|---|---|---|---|
|0x180084510 scheduler|0x180026420|0x180029ba0|ev_0016dd4e69801f8e20e1b0b471a036d7950cac5121fc74bf375d7d4fff6357b2|
|0x1800845b0 socket service|0x180026350|0x180029a30|ev_dae43245713bba15fb4406f5dd84f0cd801e72a67f38569bab85b25bb8a93077|

The prior fresh Winsock stage proves 029ea0's all-services +8 pass, then all-services deleting +0 pass, then registry-gate deletion. The selected list order is scheduler first installed by0241b0, then socket service prepended by0239c0 during socket construction; therefore the final list is socket service→scheduler. Retained0239c0 exact instructions `023ad2/023ad6/023ada` set new.next=old_head then head=new; full pseudo/assembly extracts and EIDs are retained here, with source hashes in `retained-extraction-index.json`. Its existing source counterpart `service_registry.cpp` implements that same prepend. No arbitrary custom-service ordering is inferred.

## Exact shutdown and deletion effects

**Socket-service shutdown029a30:** `029a51` selects service+50 gate; `029a5d` enters; `029a68` reads head+78. Empty head skips socket/cancellation/strong-owner work and tail-leaves the gate at `029b8c`. For a nonempty list, the measured function cancels/closes/resets each node and releases ownership; it does not unlink those nodes. The current selected backend already retires sockets and requires service.head==nullptr, so the empty shutdown's gate round-trip is the relevant remaining effect. EIDs `ev_ace9a280c1de976c15e289a597bb25a079df6582c60d221eefd8e2b36e5a2c9f`, `ev_d02363cc52babb4f3bc43bff06e852cfcbfa7cf9943c24f1b5e81f6f86b87023`.

**Scheduler shutdown029ba0:** `029bb6` XCHGs dword+3c to1, distinct from stop+34 and wake+38. It may set a timer if+48 exists, stop/join/free a hidden worker if+a8 exists, and drain work while signed low32 work_count+30>0. The selected context construction uses own_thread0, the production Runtime dispatcher state uses services=null/timer_enabled=0/timer=0, and existing quiescence requires zero work and empty queues. Under those existing conditions this shutdown has only its +3c state effect: no extra wait, post, timer, cleanup callback, hidden-worker allocation or drain proof is needed. It must not be described as an unconditional no-op. EIDs `ev_1cafca60978a40e47d8f8b8ddeec7827e88ca4d94854e3d96b366c5023fca6e6`, `ev_c1b0333dcc8bff19032eb9120c84d513460d0d49895e9652dad51b94763b4d9f`.

**Socket-service deleting026350:** `02635f` selects service+50; `026363` calls DeleteCriticalSection. Then it stores base service table084438 and, for flag1, frees the native0x80 allocation. In the current embedded host projection, native allocation/free/layout is not claimed; actual service gate deletion remains applicable. EIDs `ev_1566b648e49cb6af5c588b7e0bd50d4f8bda934d7939ca13a8d6f01273277209`, `ev_8a00ef6fb58364cfd49181b324051a6cf40336e7794ed490eab700c4886d8217`.

**Scheduler deleting026420:** after optional hidden-worker/residual-queue handling, `0264ea` deletes scheduler+60 gate. It then optionally closes timer+50 (`0264f9`), deletes timer-thread+48 (`02650c/02651a`), and finally `026528` closes nonnull IOCP+28. CloseHandle's return is not tested and no GetLastError is called. Base table store and optional0xb0 free follow. With the selected null optional members/empty queue, its applicable SDK sequence is scheduler-gate deletion→IOCP close. EIDs `ev_fabb4e9bbe2d8f89656b896856c87495e78da995b4119afe8984bc0c45f86f4b`, `ev_248cee07465ac69545b23e8695075a40bc7f4bdcd047bb98cf5e9eff544c6f33`.

Thus the existing actual selected Original cleanup can be compared as:

| Phase | Native selected effects | Current source |
|---|---|---|
|after sockets / WSA release|service gate enter/leave; scheduler+3c=1|both omitted|
|deletion pass|delete service gate; delete scheduler gate; close IOCP (ignore BOOL)|close IOCP (false→GetLastError/throw); delete service gate; delete scheduler gate|
|last|delete registry gate|already last in new WSA branch|

## Minimal next source action

After the pending WSA baseline is recorded, update only the Original branch of owned-server retirement to project this fixed two-service shutdown/deletion order under the existing zero-work/empty-socket constraints. Keep Host's current order/error contract. Use the actual service gate; preserve low32 wake when projecting high32+3c shutdown state in `word_38` under exclusive retirement. Do not alias-cast a uint64 member into a native dword atomic or invent a general registry/provider framework. Retain existing native shared release and final registry-gate deletion. Original IOCP close should consume the retained handle with its ignored BOOL, without querying GetLastError or adding a source exception. An injected SDK C++ exception remains an unknown prefix, not authorization for retry/rollback.

Meaningful future source cases: exact three-gate identities/order with injected SDK recording; ignored failed Original port close without error-query/throw; high32 shutdown state set while low32 wake is retained; existing Host order/error assertions unchanged. These are proposed checks, not executed tests. No full hidden-worker/timer/custom-service teardown implementation or native object/allocator/EH ABI equivalence is claimed here.

## Current source authority

- `anadius-owned-server-source/server.cpp`: `bcbf7ab75389dfc833adb72eba6f120613a0e9ace205e236fb00fa1bdf21804e` (existing explicit retire checks and manual close order).
- `anadius-socket-source/service_registry.cpp`: `663e6f4799e9173722e0af62b05d7a4e96d069f4f0dc55b6777cc46e037cbc02` (head prepend).
- `anadius-socket-source/scheduler_registry.cpp`: `dd435dd06297b628a8a101effc1c1d4263a281299d04f8517ee16368de4c64ac`.
- `anadius-socket-source/iocp_constructor.hpp`: `c82705d8734e98d5df368faeb20d202a08c04d70092f6dd4e858d617e9112725` (typed word_38 is distinct from high_34, native layout not overlaid).

Native support helpers for registration/constructor/worker/close already exist in their scoped source files; no selected general shutdown/deleting implementation for these four slots was found. The conclusion above uses actual applicable effects and order, not that absence alone. The generic native branches remain separate reconstruction work.
