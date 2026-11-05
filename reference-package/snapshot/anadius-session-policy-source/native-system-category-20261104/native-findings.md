# Native system category condition/equivalent methods

Result: the selected native system category uses the unchanged error value and its own category identity for default conditions and both equivalent overloads. There is **no Win32-to-generic condition mapping in these three complete bodies**. Inherited C++ error_category base defaults are the coherent normal-value projection for this concrete stable self-identity category; delegation to compiler std::system_category default_error_condition would introduce an unobserved mapping.

Fresh static REA6.8.0/Ghidra12.1.4, 2026-11-04. binary_session verified the configured server command/package/catalog as aligned and Ghidra12.1.4/Java25.0.5 available before opening the original. Target C:/Users/Skysect/Downloads/anadius64.dll SHA256 at open and after successful close: **9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c**. Seven complete Evidence records,32,775 exported bundle bytes. Post-close session reports open=false, cleanup_pending=false and alignment=aligned.

No source edits, native execution, compiler/tests/link, package update, version.dll access, annotation or provider reconfiguration. This is the exact category contract follow-up needed by the selected compiler-category migration, not broader CRT analysis. The previous name/message/A1110 binding records are supplementary authority and were not queried again.

## Exact table resolution

Fresh48-byte read at0x180084248 is complete. QWORD entries:

| Slot | Pointer | Role |
|---|---|---|
| +0x00 |0x18000b450 | Destructor, not reanalyzed |
| +0x08 |0x180020940 | Name, prior evidence supplement |
| +0x10 |0x180020950 | Message, prior evidence supplement |
| +0x18 |0x18000d120 | default_error_condition value projection |
| +0x20 |0x18000e0c0 | equivalent(code, integer) normal value projection |
| +0x28 |0x18000e0e0 | equivalent(integer, condition) normal value projection |

The semantic role labels are inferred from the concrete argument/return layouts and std::error_category contract. Addresses, instructions, field accesses, exact call order and table pointers are observed. Native and compiler C++ object/vtable ABIs are not equated.

## Complete selected bodies

**00d120**,11 bytes,0x18000d120–0x18000d12a:

- d120 writes incoming R8D to output[RDX].
- d123 returns that output pointer.
- d126 writes incoming RCX (this category) at output+8; d12a returns.

Thus default condition is unchanged low32 error value plus this category. There is no classification switch, external call, compiler system-category delegation or generic-category selection in this complete body. Native condition padding/layout is outside the HOST value projection.

**00e0c0**,25 bytes,0x18000e0c0–0x18000e0d8:

- e0c0 reads the supplied code pair's category pointer from RDX+8.
- e0c4 reads that category's field+8. e0c8 compares it with this category's field+8.
- If equal, e0ce compares the code pair's low32 value to incoming R8D.
- Returns AL1 only for both equal, otherwise AL0. Upper return-register residue is not part of the C++ boolean result.

**00e0e0**,63 bytes,0x18000e0e0–0x18000e11e:

- e0e6 loads this category vtable. e0e9 retains the incoming condition pair.
- e0ec moves the incoming integer into R8D and e0ef supplies a16-byte local output.
- e0f4 invokes this category's exact virtual slot+0x18, established above as00d120.
- e0f7–e103 load and compare field+8 of the returned/default category and the incoming condition category.
- e109/e10b compare their low32 values; returns AL1 only if both equal.

This overload uses its virtual default condition, not a separately manufactured compiler condition. All three bodies are complete single-range listings; combined99 bytes. No function beyond these three was queried.

## Category identity and bounded C++ recommendation

The native equivalent methods compare category **field+8**, rather than raw object pointers. The prior fresh complete32-byte read atA1110 (ev_eb486a9416f6cbfeed05d5815675c4a981ffb8d4bb337a32016db6ebf9b5f4aa) already contains:

- A1110+0 =84248 table.
- A1110+8 =A1110 itself.
- Adjacent genericA1120+8 =A1120 itself.

That supplement was read, not regenerated or copied into this fresh session. The preceding fresh028620 evidence establishes that A1110 is its returned system category object. Thus for the actual selected concrete native object, the comparison key is stable self identity. The compiler projection can keep one stable error_category object per admitted provider domain, return error_condition(ev,*this), and use inherited base equivalent overloads' category identity/value behavior. It should not delegate to compiler system_category's Win32/generic mapping. No override is needed just to simulate an unobserved numeric mapping.

This recommendation is bounded to the selected concrete self-key category and ordinary typed code/condition values. It does not reconstruct arbitrary foreign category objects with forged/shared field+8 keys, nor establish native MSVC RTTI/vtable/condition layout, cross-module C++ ABI, raw category lifetime or exception/EH compatibility. Any imports-keyed HOST pool lifetime/identity design remains a separate source implementation/review concern.

The name asio.system and concrete020950 message body remain the preceding stage's supplementary proof. The local LLVM-MinGW provider audit is separate current compiler-artifact authority; these new native bodies are not inferred from that compiler implementation.

## Fresh evidence

| Full reply | EID |
|---|---|
| bytes-180084248.json |ev_1ea5000d1c7df80f8cb644f6b4cc8c202b2846fd2dbdb12b1cf46e5dc7e1b38d |
| pseudo-18000d120.json |ev_2703e5d60e1552f400c965b6a04a319e112909726b223dc74229c85396952c5f |
| instructions-18000d120.json |ev_b710b3fa2de15968bfcc2b9ea304533dc40f29a40df9aa8603949b4a7480cb0b |
| pseudo-18000e0c0.json |ev_a758575bf117f7d07103c3cc4011ee9dae151c8d350e1373fd09cf786efe2083 |
| instructions-18000e0c0.json |ev_505ed7772e43b9adf17a5dd278792b432e3746fe0cf412849c3f042a2467d0fe |
| pseudo-18000e0e0.json |ev_ee607098f38b6d028b9f425789b3f0c6c1cc8ce9f22ffb7a7be69ce68a2c6444 |
| instructions-18000e0e0.json |ev_a89be8389c95e312fd9c2182efa59ce6696f5db3f474657ab55e0157bb04af67 |

Full server/open/export/close/post-close/hash replies are retained. Bundle unknown count0 is only its recorded object count, not proof of whole-program completeness or runtime equivalence.
