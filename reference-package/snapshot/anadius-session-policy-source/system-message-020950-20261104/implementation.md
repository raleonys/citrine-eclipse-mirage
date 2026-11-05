# Concrete system category message020950

Status: SOURCE IMPLEMENTED; no compilation, tests, linking, DLL execution or packaging performed. Original binaries unchanged. Root and independent reviewer own subsequent source review and evidence freeze.

## Production change

The selected Policy system-category message path now calls socket_recovered::system_message_020950 instead of the compiler's system_category().message. Generic and misc formatting remain their existing providers, and an explicit Options.format_override still takes precedence for every category. Terminal std::system_error construction and its compiler category/what()/EH ABI remain unchanged and open; this stage closes only the concrete message-value path.

SystemMessageImports injects the actual FormatMessageA and LocalFree signatures, defaulting to those SDK imports. Options.system_message is appended after format_override so the legacy aggregate prefix is preserved. Both pointers must be nonnull even with an override, matching the selected provider's complete-construction contract. The HOST aggregate layout changes and all consumers require a consistent future rebuild; no existing build proof is updated.

## Native evidence and exact value behavior

Fresh 028620 returns object 0x1800a1110, whose table 0x180084248 selects message slot +0x10 = 0x180020950. The complete020950 listing is 275 bytes at 0x180020950–0x180020a62. Native findings and the six raw records are retained in this stage.

- 020964 initializes the output pointer to NULL. FormatMessageA at020989 receives flags0x1300, null source, DWORD code, language0x400, address of that pointer, size0 and null arguments. The adapter uses the same arguments.
- 0209a0–0209cf remove at most one trailing LF and then at most one CR using the returned count. Emptying the counted result selects the fallback. Other whitespace/repeated endings are retained.
- 0209e4–0209f8 scan to the first NUL for the subsequent copy, independently of the returned count. The adapter constructs the success string from the C string, preserving leading/embedded-NUL shortening without inventing a fallback.
- 020a04–020a43 construct the exact fallback `asio.system error` (17 bytes). The original32-byte allocation/capacity31 is not a HOST std::string ABI claim.
- 02098f–020997 save the allocated pointer. Both normal paths call LocalFree(saved pointer) at020a47/020a4a, including NULL on normal failure and a nonnull failure pointer. Copy/allocation precedes free. LocalFree's return is ignored.
- Native contains no GetLastError call or entry-error restoration. The adapter leaves imported SDK error effects visible. Only optional HOST admission checkpoints preserve LastError around their metadata work.

Positive count requires the real SDK contract's valid nonnull, accessible, NUL-terminated output. Arbitrary invalid injected-provider buffers are outside that admitted contract; no unsupported-input fallback is fabricated.

Primary fresh evidence: pseudo ev_0274fa3cd88e934ca61666cd5c1171f80edce8cbc7cd1a7dbe6e851dfdf2f0aa; complete instructions ev_0c36cbad633abd179eac63d16b40d5c52eabd28764feead934ec5968b478eefc. Native-findings.md records all six records and category linkage.

## SDK failure-prefix retention

SystemMessageState is caller-retained HOST state, held by Policy::Impl. It records buffer, entered and release_entered. The helper refuses a nonempty retained prefix. Optional synchronous nonmutating admission checkpoints run before/after SDK effects and before release. The final checkpoint runs after LocalFree while the entered/release markers still exist; only successful return from that checkpoint clears the state. There is no later opaque effect before clear and the result move-return.

A throwing FormatMessage call, copy/allocation, LocalFree or admission checkpoint therefore leaves the entered prefix instead of retrying or replaying release. No RAII free, automatic retry, new unload/drain requirement or native exception-cleanup proof is added. Existing poisoned Policy admission/close behavior remains. The exact native EH/unwind cleanup remains unknown.

## Frozen production hashes

| Current file | SHA256 |
|---|---|
| outputs/anadius-socket-source/native.hpp | 054f0a03884610c94ec0683f67735ce41fb2025130cbbd757f6c818da31d8e5c |
| outputs/anadius-socket-source/native.cpp | 718bfac524e2b8d445e522b433abba165dee811e1301cc50a0ecc9fdc9a41a39 |
| outputs/anadius-session-policy-source/policy.hpp | aec906fed7543fcafeb9862cfef347b3391ccdf13c845d5b2529432cff8e388b |
| outputs/anadius-session-policy-source/policy.cpp | 5c42ccbff002069dc4b1bdb111f6ed59efcb634ecc01efaef7890fd55b06307f |

The baseline files in before/ were reconstructed after editing and exactly checked against independent historical hashes. See before-recovery.md for the recording error, recovery method and authorities. They must not be described as contemporaneous captures or fresh runtime validation.
