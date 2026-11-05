# Business-path triage — 2026-11-04

Read-only source triage; no edits to production, build/test execution, REA session, original access or tracker rewrite. This identifies a comparison target, not a confirmed behavioral defect. No additional normal-route unimplemented Information/Content/Licensing/achievement body was found in the focused code inspected.

## Priority candidate: XML parse status compatibility changes protocol routing

- `outputs/anadius-xml-reader-source/reader.hpp:5` explicitly selects upstream tinyxml2 11.0.0 and disclaims exact original embedded-release/malformed-input compatibility. `reader.cpp:55` returns that parser's numeric status.
- `outputs/anadius-native-profile-source/native.cpp:181` is the selected native004280 adapter; at184 it returns the upstream numeric code and projects the document's error flag.
- `outputs/anadius-socket-source/native_message.cpp:123` treats parsed==8 as the first encrypted-frame autodetection branch;125 treats parsed==7 as the one-time empty-version repair branch. This is application protocol behavior, not merely diagnostic text or a capacity concern.

Trigger: the first encrypted LSX frame before encrypted_c4 is set, or an LSX XML input containing the empty-version form handled by native034730. A difference in parser acceptance/error-number classification could select bad-XML/close behavior instead of decryption or repair, preventing an otherwise supported client handshake/request. No such mismatch is proven by this source-only triage.

Suggested fresh native selectors: original033d70 (`0x180033d70`) branches using the Parse return and document error field; Parse004280 (`0x180004280`); request-repair034730 (`0x180034730`); XMLDocument constructor001830 (`0x180001830`). Compare the exact error-number producers and empty-version handling before changing parser configuration or replacing the pinned library. Original0032d0 ErrorStr matters only secondarily for diagnostics. Header depth=100 is already evidence-backed and is not itself a proposed change.

## Deliberate boundaries excluded from the gap list

- `content-effects/effects.cpp`'s legacy one-argument online-avatar overload throws, but the selected Content implements AvatarRequest and QueryImage0314f0 uses its owner/size-aware route. It is not a missing selected avatar implementation.
- NativeProfile's untyped greeting()/process() UnsupportedInput branches are bypassed by the selected typed Profile route.
- Misc handlers intentionally returning the recovered "function ... not implemented" response are not reconstruction stubs.
- Achievement XML/INI writers discard lower bool results in current source. That alone does not prove a gap; native030010/03c1d0/01f9a0 call-result behavior would have to differ. No persistence change is recommended merely from that observation.

Recommendation: use the parser/handshake comparison as the one high-value business-path audit. Avoid inventing a production patch when fresh native comparison confirms the current statuses and parser behavior.
