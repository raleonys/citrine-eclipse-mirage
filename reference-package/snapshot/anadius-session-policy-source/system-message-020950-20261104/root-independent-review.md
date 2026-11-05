# Root independent020950 source and regression review

2026-11-04. Root did not author these six source files. Read the fresh native pseudocode/records, current implementations and relevant caller/test source; no compiler, project test or execution was used. STATIC REVIEW ONLY.

The helper and selected Policy callback implement the newly observed message operation: exact Win32 argument values; one LF then one CR removed by API count; success copied to first NUL; exact fallback; output built before saved-pointer LocalFree on both normal branches, including NULL. Generic/misc and explicit formatter remain distinct. The callback is actually selected for index1 in Policy.text, consumed by existing category.message_slot10 and actual025480/025a70. Terminal compiler std::system_error category/what and native allocator/EH ABI are not certified by this change.

Root review found and corrected a post-effect prefix defect before freeze: the first helper candidate cleared state before the last checkpoint. The final helper checks after LocalFree while entered/release_entered remain set, then clears without another opaque effect. A thrown final check now keeps replay protection. Imported SDK LastError effects are not wrapped in entry restoration; only optional admission metadata checkpoints save/restore their own effect. The imports field was moved to the trailing Options position to preserve old aggregate prefixes. Both imports are validated by Policy construction.

Native test source has sixteen deterministic data cases distinguishing CRLF/LFCR/repeated endings, short returned count versus actual C-string copy, embedded/leading NUL, ANSI high bytes, zero-count and trim-empty fallback, large output, exact NULL/nonNULL saved pointer and ignored free return. The free callback checks mutated source bytes then overwrites storage, so it can detect a borrowed-pointer or copy-after-free mistake. SDK signatures/default construction, full code bits and call order are checked. A named checkpoint object avoids temporary callback destruction between helper return and immediate LastError capture. Format/Free exceptions and final post-free checkpoint exception retain exact phase and reject reentry without replay; no native allocation/unwind result is inferred.

Policy test source invokes the real default system category via existing read/write helpers, with format_override cleared and injected SDK entries. Seven lines cover system code2 (not misc EOF), fallback, large heap-backed Parent string, embedded NUL and counted trim. SDK observation occurs after visible prefix and before append. Each line checks flush and actual Parent string ticket retirement; explicit override bypass invokes neither system SDK method. Existing misc/override and twenty-seven retained terminal source cases remain present. Added missing-import validation is passive. None of these assertions has run.

No blocking issue found in this bounded final source review. Native valid-output pointer/count/NUL preconditions, unknown native EH, real OS/locale results, compiler compatibility and full DLL/runtime equivalence remain limitations. Four production baselines were reconstructed and independently SHA-checked, not contemporaneously captured; before-recovery.md explicitly records the recording error and authorities. Test before-images were captured before edit. Historical build/ZIP validation is not rewritten.

## Reviewed final file hashes

- `anadius-socket-source/native.hpp` SHA256 `054f0a03884610c94ec0683f67735ce41fb2025130cbbd757f6c818da31d8e5c`
- `anadius-socket-source/native.cpp` SHA256 `718bfac524e2b8d445e522b433abba165dee811e1301cc50a0ecc9fdc9a41a39`
- `anadius-session-policy-source/policy.hpp` SHA256 `aec906fed7543fcafeb9862cfef347b3391ccdf13c845d5b2529432cff8e388b`
- `anadius-session-policy-source/policy.cpp` SHA256 `5c42ccbff002069dc4b1bdb111f6ed59efcb634ecc01efaef7890fd55b06307f`
- `anadius-socket-source/native_tests.cpp` SHA256 `419114eeda6048e0beb73b816c837d7e1a97f1a8d84f167b435593e4fe193357`
- `anadius-session-policy-source/policy_tests.cpp` SHA256 `601d9f0e69ee3a4f1aa353c360ff2389d9445a9f4c9e1005bab272fbc869aa73`
