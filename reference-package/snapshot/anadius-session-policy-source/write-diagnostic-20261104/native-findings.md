# Synchronous greeting-write diagnostic

Result: actual nonzero write failures from025720 carry the native028620 system category into028240, with the exact literal **write** at084720. Policy.raise_write currently keeps the right context but uses compiler system_category formatting. The already-integrated native system-category provider is therefore the concrete remaining binding target for this selected path.

Fresh read-only REA6.8.0/Ghidra12.1.4, 2026-11-04. binary_session verified aligned configured server and supported provider before own isolated open. Original `C:/Users/Skysect/Downloads/anadius64.dll` SHA256 at open and after close: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Five full Evidence records,45859 bytes, exported; own close succeeded. No source edits, target/DLL execution, build/test/link, packaging, network, version.dll access, annotation or original writes.

Only two functions were inspected:025720 and its actual send02a200. Prior028240/0248b0/00a270, system-message020950, native category defaults and selected compiler-provider records supplement this audit; none was queried again.

## Literal and actual application caller

The complete32-byte read at084720 starts `77 72 69 74 65 00`, the C string write. It is followed by separate open/set_option literals; they are not part of this context.

Complete025720 instructions cover837 body bytes across provider ranges025720–025a4e,025a50–025a60 and025a62–025a66. The initial incoming accept error is tested at02574d–025751. With zero accept error it constructs an accepted session through023000, obtains the actual greeting through02d540, and synchronously sends it in bounded chunks.

At025826–025831 it initializes a local error code0 with the generic/success categoryA10F8. That initialization alone does not establish the category of a later failed send. The pair address is passed as output to02a200 at025896–0258bb. The caller checks the current error before another chunk (025856–025860) and after the loop (0258c8–0258cd). Either nonzero branch reaches025a50, loads **084720**, then calls028240 at025a5c using that same updated error pair.

On this branch normal greeting cleanup, initial frame read0280d0, local strong release and next accept rearm have not completed. The audit does not infer EH cleanup from the normal body. The initial incoming nonzero accept-error logging branch is separate and is not this write exception.

## Nonzero category lineage through02a200

Complete02a200 listing spans02a200–02a447,584 bytes. Every modeled nonzero output path in this function assigns the actual028620 category:

- Invalid socket:02a244 prepares code0x2719 (10009),02a24c calls028620, then writes that code/category to the supplied output pair.
- Send path:02a2d7 calls the imported WSASend;02a2df obtains028620 category,02a2e7 reads WSAGetLastError, and the code/category are published to the output. The category lookup/error-read order is the observed native order, not a guessed rewrite.
- Error64 remaps to0x2746 (10054) using another028620 call at02a310; error1234 remaps to0x274d (10061) using028620 at02a337. Both remaps retain native system-category identity.
- Successful WSASend sets output code0; the all-empty/state0x10 shortcut also clears code0. Those zero paths cannot themselves trigger the caller's nonzero write raise.
- Retry qualification compares the output category identity against028620 and checks10035 or1237. If select fails,02a3e2 calls028620 and02a3ea captures WSAGetLastError before returning the updated error pair. Terminal/retry-exit failures therefore also carry that native category.

No new generic or misc category is assigned on a nonzero send output in this selected body. A subsequent nonzero025720 write raise is consequently a concrete system-category path, even though its initial code0 pair usedA10F8.

## Existing constructor/category supplement

The fresh worker-error028240 audit proves nonempty context + `: ` + supplied category.message(code), retaining the original pair. Fresh system-category evidence binds028620 toA1110/vtable84248/message020950, whose exact formatting differs from the selected compiler system-category provider. The native default/equivalent signature evidence and module-owned source category already address that provider's identity/value domain; this write finding requires no new exception framework.

For the selected nonzero write error, normal diagnostic value is `write: ` plus the actual020950 message. The context is already correct. The category-message source is the remaining difference, not a newly missing028240 body.

## Actual source route

Read-only source at audit time: Policy.cpp SHA`7a7603dc92eb5d8617d1efa6599aa0cb7f8f6542514b9d28352b81bf56d26771`.

- `anadius-session-flow-source/flow.cpp:49-68` implements the accepted session/greeting send loop and calls raise_write(error,084720) before/after the loop on nonzero error. It does not free the greeting or start frame read before that raise.
- `anadius-socket-source/operations.cpp:35-68` implements02a200's native-category assignments and remaps. The actual Flow calls this function with its full logical category identity, not the separate compatibility SynchronousTransport result that retains only an enum.
- DomainFlowApi forwards the error unchanged; Runtime::FlowApplication forwards to actual Profile Application; `profile.cpp:489` preserves the source error/context while intentionally poisoning Profile before Policy.
- `Policy::raise_write` at policy.cpp:267 validates084720 but constructs `std::system_error(e.value,compiler_category(v.index(e.category)),"write")`.

A bounded follow-up can select the cached module-native system category only for validated index1 in this write projection, preserving the write context and existing catch/signed-code/terminal prefix. Generic/misc explicit HOST inputs and other uninspected literal mappings need not change. No loader, work counter, async scheduler or extra retirement condition is implicated.

## Retained evidence and limits

| Full reply | EID |
|---|---|
| bytes-180084720.json |ev_6adfbb63ed4c7c296d5a5ed0bbcad09446403228182773601e3797823d7f4d77 |
| pseudo-180025720.json |ev_9cc33f060f094cdd6280d11e010deecbbe1e1092a97b5940bd65949cdb6eeca0 |
| instructions-180025720.json |ev_8bdfda3b871d81ddbbec5df1f883dbfaa1223c31cdc328c76eb8866e4d76e50d |
| pseudo-18002a200.json |ev_2a44f57d793f47ce0e6fbbf55cdda9cca87eea49b8894172811198363316bc4d |
| instructions-18002a200.json |ev_e0059ff0beee55c9c9d3d1306b9cfc4ec2b4fefc3223a2dbb59ad4c358df5d57 |

All full tool replies, export/close results and original-hash proof are retained. Zero bundle Unknown objects do not establish native EH/ABI or runtime parity. No actual SDK write/OS message result was executed, no generic028240/code0 constructor claim is made, and no whole-goal completion percentage follows.
