# Next Licensing shared-root audit

Read-only source/retained-evidence audit after the Provider stage froze at 108 pending sources. This additive plan is not a change to that stage's frozen evidence. No production edits, new REA queries, original binary access, build, tests, linking, execution or packaging.

## Ownership split and concrete conversion constraints

| Item | Actual current source | Required ownership in a future view |
|---|---|---|
| Invocation bookkeeping | `licensing-effects/effects.cpp:18–42,152–199`: Core/Handlers `job`, `busy`, `binding`, `checking`, mutable handler/admission callback pointers, Node/Xml adapters | Private execution/adapters/context. Sharing these singletons would preserve the current body strand or misdirect admission. Do not install a child's callback into the root Core. |
| File prefix | Core GuardedFiles at `effects.cpp:44`, hashing/token adapters at 59–80 | Private guarded files/hash/token adapters per view; unknown adopted raw files retained with that execution. Same borrowed lower providers and backing identities remain. |
| Authentication identity/history | `license/auth_state.hpp`: immutable forty-byte AuthenticationCode; `license.cpp:34–39,57,124`: `authentication_`, `used_` | Owner binds/captures the exact stable cell once before licensing view publication. `used_` is HOST-only bind-before-dispatch history, absent from the native bodies; execution-local reconstructed Handlers can have a private `used_` if owner binding is already sealed. Do not invent a shared native used-bit write. Unknown diagnostics do not set this licensing-handler history today. |
| Attempt/block cells | `effects.hpp:16`, `effects.cpp:207` binds the supplied shared AttemptState; `license.cpp:125–155` | SAME `attempted_files` and `blocked`, never fresh zero defaults per view. Counter increments happen after actual token loading; blocked is set before fatal branches/ticket attribute reads. |
| In-memory token | `effects.cpp:207` binds `configuration.denuvo_token`; `license.cpp:60,145–152` | SAME string across RequestLicense and InvalidateLicense. No per-message token copy, reset, fabricated rollback or retry. Native/source invalidation clears only this string, not attempts/block/file state. |
| Config map | `license.cpp:79,81`: `game_attributes[...]` while building ticket text | SAME map; missing-key insertion is a real write, not a read-only formatting operation. Hash/path/content-ID fields are borrowed live inputs, not captured configuration snapshots. |
| Online result | `effects.cpp:95–96`; `license.cpp:23–31,44–45`; `online-query/query.cpp:71–78` | Preserve OwnedAuthEffects selection: foreign slot30 result is scanned/copied while QueryUse retains object/code. Each handler uses its own owned string. A lifetime reservation does not prove a foreign provider's shared return buffer is thread-safe; raw-pointer compatibility remains the narrower caller contract. |

An execution-local `license_recovered::Handlers` must bind the shared StateReferences and its own Core effect view, rather than copying true business cells or concurrently writing the old shared `used_`. Owner authentication binding/retirement must not race view publication. Caller checkpoints, file wrappers and options retire while the exact context is live; later closed-wrapper destruction is metadata-only. Preserve NoResponseRequired as healthy source control flow only when it originates in the source action; admission failure throwing that same type remains unknown, as the current Core/Handlers scopes distinguish.

## Retained native evidence and limits

`outputs/anadius-license-source/evidence.json` identifies original SHA `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. The current retained aggregate assembly/pseudocode were inspected, not regenerated:

- Auth `0x18002e3b0`: asm `ev_592ffe3ba5103bc35cd252aa302b431fa51364ef65e2a49229ba39bdf25731a6`, pseudo `ev_33624c87853573a8e7b3efe065f830b8be05beda34cb0c9af0879cf7456993d4`. Slot30 at `02e413`, immediate C-string scan and local copy; no observed direct handler mutex call.
- Invalidate `0x180030c50`: asm `ev_11169253548ac4c11bc638b18cdcccad4a04c43710db7fe5c0c7af8c4d8fe263`, pseudo `ev_d1dc86b5f6657591728da569e088134c9c5dae2ace13ea7bb1fd3cca8a7f42c5`. `030c68–88` writes length zero/first NUL to global token `a3b58`; no body-level lock observed.
- Request `0x1800320c0`: asm `ev_f589cb381fc9264d56d27cb114d0044bb60d3c34eddeafa46a7a36ee411e5883`, pseudo `ev_5c9367a9fdfab1d294c42fc48cbdb827f0cb9f4e56c3f220270debd2c4ab2732`. Direct globals: blocked `0320ee/0322c6`, signed counter `032188/0322a1–a9`, token load/store `0321c6/0321d1`, map lookup `0323d1–d8/03241d–24`. No native body mutex demonstrated. This does not establish well-defined arbitrary overlapping token/string/map mutations or exclude every possible enclosing caller gate.

Retained parallel-frame proof establishes `025480 → 028d30 → 033d70` before response scheduling, not a licensing business transaction lock. A future broad simultaneous Request/Invalidate policy needs its own caller/data-race qualification; do not add a whole-license mutex merely by analogy with existing HOST busy flags.

## Narrow supported overlap

UnknownMessage (`effects.cpp:114–123,214`) accesses request-owned diagnostic text and shared stream/dialog only. Each shared-log lease ends before modal UI. GetConfig (`handlers.cpp:6`, Router case `02e5f0`) emits static service literals to the invocation's writer and reads no licensing cells. Core UTC (`effects.cpp:106`) uses private adapters over caller-supplied clock/formatter and no token/attempt/map state. These paths support the intended held-modal/other-message progress without claiming arbitrary concurrent license mutation. Keep registries warmed, independent message XML/writers, normal plaintext/no shared cipher mutation, and show-LSX/log conditions qualified by the existing native scenario. Core UTC's null-epoch mode1 filename formatting is distinct from Information's mode0 response time and must stay so.

## Audited current SHA-256

| Source/evidence | SHA-256 |
|---|---|
| `anadius-licensing-effects-source/effects.hpp` | `7a84578546cecb74312818a094cdccdabbb641ab67f0c63787e7bfdf56684d07` |
| `anadius-licensing-effects-source/effects.cpp` | `c66cf34ed9b2e23772c48e834810e5afca0200083dd14b30e0eb508547e7e5d9` |
| `anadius-license-source/license.hpp` | `21da0faefca86f6c3ea18a49fc0624be8f7f636bce72b528e8577036d31684ec` |
| `anadius-license-source/license.cpp` | `87ea1eebab23fa2bbaff52689e4a076428e5cd8f292e028fbba985d4ce4376c0` |
| `anadius-license-source/auth_state.hpp` | `713f25d7dc4af5e59d05364bec8a9573a90cdb3e7976df39906e773d373f56b5` |
| `anadius-online-query-source/query.cpp` | `aef7a62d8a77ca6426d88b3a5a9275d67230cc573f58022003c83dc374039287` |
| `anadius-all-functions.asm.txt` | `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42` |
| `anadius-all-functions.pseudo.c` | `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb` |
