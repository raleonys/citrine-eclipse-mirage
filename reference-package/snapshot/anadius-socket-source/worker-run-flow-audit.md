# Independent worker run and deletion flow audit

Reviewer: worker_run_audit. Scope: complete retained assembly and pseudocode for four normal procedures only. No original image, original executable, user configuration, network, or real worker thread was executed or opened. This is static evidence, not observed runtime behavior.

The retained pseudocode SHA-256 is `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`; retained assembly SHA-256 is `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`. Both were independently hashed during this audit. Target identity is the previously bound anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

## `0x180026320`: worker function scalar deleting destructor

At `026326..026330`, store opaque vtable identity `0x1800844b8` before testing flags. At `026333`, only flag bit zero matters. If set, call `04b0f8` with this identity and exact native allocation size `0x20` (`026338..02633d`). At `026342`, return the original pointer identity cached in RBX; the provider may have destroyed the object, so no field or pointer-to-reference access is allowed after the delete call. Upper flag bits do not alter this behavior. There is no event closing, scheduler destruction, or extra cleanup in this normal body.

Evidence: pseudocode `ev_a5b1ddc59703eac49757a284109778592c0ff620184c9ec7cb170cafa588bd6e`; assembly `ev_c1d7725a414b7f64b770058dc15e8db4aa9f6c4d09b025ad2e0f0eb9cf84911b`.

## `0x180029890`: worker function virtual run wrapper

Read scheduler field `+0x18` (`029894`), create local result storage, and call the actual recovered `0298b0` (`02989d`). The returned count and local result are discarded. Do not replace this call with an abstract successful no-op. The wrapper contains no post-call function-object access, ownership operation, or result inspection.

Evidence: pseudocode `ev_521cfdbca7541399b8af5c4f7bdc5ceb096310c502b3436053fdfcd22e970ec5`; assembly `ev_450a3eec4ce1b268df9350871ff8b463b49cd06620c715153ee431acdf313130`.

## `0x1800298b0`: scheduler run loop

The first operation is a locked **32-bit** fetch-add of zero at `scheduler+0x30` (`0298ca..0298d1`). A high word does not affect the branch. If the previous low word is zero, call actual `02a190` (`0298d5`) first, then write result low32 zero and category identity `0x1800a10f8` (`0298da..0298ef`), then return zero. The copied native status high32 padding is uninitialized stack data. Do not claim it is zero or preserved from the caller; a typed result intentionally models only the meaningful value/category. If stop raises, no successful result write occurs.

On nonzero work count, initialize local state `+0x90` as a 32-bit zero and state qwords `+0x98` and `+0xa0` as zero (`0298f9..029908`). Bytes `+0x94..+0x97` are unspecified padding. Call required existing CRT provider `04a240` on the latter pair (`029910..029918`) **before** zeroing ten cache pointer slots (`02991d..029934`). The ten pointers and count/pair are separate semantic fields of the local thread state; no native stack overlay or complete state-padding initialization is established.

Build a TLS frame with first field the **address identity** `scheduler+0x28`, second field the ten-slot state address, and third field the prior `TlsGetValue` return (`02993c..029962`). The first field is not the value of the IOCP handle. The native CMOV-null expression does not establish supported null scheduler input: the earlier fetch-add already dereferenced `scheduler+0x30`. A host null guard is a bounded precondition restriction.

Read global TLS index `0x1800a38f0` independently for `TlsGetValue` (`029956`), initial `TlsSetValue` (`02996c`), and restoring `TlsSetValue` (`0299ce`). Callback mutation of the index must be visible on subsequent SDK calls. Getter and setter failures do not invoke last-error handling here. The initial setter uses the local frame identity. The final setter reloads the prior-value field from that frame at native stack `+0x30` / frame `+0x10`; a dispatcher that mutates this field changes the restoration value.

Call required dispatcher `027c10(scheduler, 0xffffffff, state, result)` (`029979..029989`). Each nonzero return increments a uint64 counter by one, saturating at `UINT64_MAX` (`0299a0..0299ac`), then calls the same dispatcher again (`0299af..0299bf`). Zero ends the loop without incrementing. No sign, magnitude, result status, or TLS-setter result controls this loop. Result writes on the active path belong to the dispatcher, not to this wrapper.

After the final zero dispatcher return, reload the frame prior-value field and restore TLS using a fresh global index (`0299c9..0299d4`). Then inspect ten cache slots **in ascending order**, reading each slot immediately before testing and freeing it (`0299db..0299fa`). Each nonnull slot is passed to exact unsized provider `04b788`; no slot is reset by this body. Earlier cleanup callbacks can mutate later slots and those changes must be observed. Finally call required CRT cleanup `04a2cc` on the local qword pair (`0299fc..029a04`), and return the saturated count (`029a09`). No exception-safe cleanup or unwind path is established by this normal assembly listing; do not introduce automatic RAII cleanup on throwing providers as a claimed native behavior.

Evidence: pseudocode `ev_e6bbbaa1780b6df4e1edc25150c1c8a939c0d5fb937afed734d4f8c641d88539`; assembly `ev_4b20e2442cb7d601c8177ba4b68acc8c09a8cce81580892199e78a5d0f00c319`.

## `0x18002a190`: scheduler stop and IOCP wake

Exchange **only low32** of stop field `+0x34` with one (`02a194..02a19b`). If prior value was nonzero, return with no wake exchange or SDK calls. Otherwise exchange low32 of wake field `+0x38` with one (`02a1a2`). If prior wake was nonzero, return. Upper32 of the `+0x38` qword must survive. Typed atomic-effect providers may implement these exact operations while keeping layout/ABI limitations explicit; ordinary unprotected writes would not prove concurrency equivalence.

Only after both exchanges returned zero, read the **live** port handle at `scheduler+0x28` (`02a1a9`), then call `PostQueuedCompletionStatus(port,0,0,nullptr)` (`02a1ad..02a1b3`). EDX is zero because it contains the old wake value on this path. Any nonzero BOOL is success and returns without reading last error or category.

On zero BOOL, cache GetLastError (`02a1c2..02a1ca`), call `system_category_028620` **even when cached error is zero** (`02a1ce`), then test the cached error (`02a1d3`). The category callback cannot change the selected cached status. If cached error nonzero, raise `028240` with that error/category and opaque message identity `0x180084530` (`02a1e6..02a1f2`); zero returns. The native error padding high32 is unspecified. Neither flag is rolled back on failure, zero error, or raised exception.

Evidence: pseudocode `ev_e826697a7d9543338d9741906c5ce25c29a36806fd701f2178a335f314f7fee2`; assembly `ev_28baeec203ffe1abf8bab8af7907605b09a6dfb72018fcd7ea4ffbca8955f680`.

## Boundaries and independent review status

Four original normal procedures are counted: `026320`, `029890`, `0298b0`, `02a190`. Required providers `027c10`, `04b0f8`, `04b788`, CRT `04a240` / `04a2cc` / `049be0`, native exception implementation, ownership, ABI overlays, actual concurrency and executable vtable binding remain outside this stage. CRT production is paused and unchanged. Opaque image addresses are data only and must never be called as executable addresses. Independent source review waits for the four frozen new production files; this flow report does not assert that tests have run or that full DLL equivalence is complete.

