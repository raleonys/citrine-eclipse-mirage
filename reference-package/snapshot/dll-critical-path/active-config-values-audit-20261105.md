# Active FilePass configuration values — 2026-11-05

**Bounded negative:** no new valid CFG/value discrepancy was established in this 12-helper/branch screen of the actual mutable application route. No fresh REA query or source patch is justified by the comparisons below. This does not establish the whole CFG input domain, native CRT side effects/locale/allocator/EH equivalence, or whole-DLL completion.

Only current source and retained original evidence were read. No user CFG file, project build/tests, DLL/EXE execution, linking, packaging or source/status mutation occurred. Already-closed legacy collection, duplicate assignment, XML status and timestamp suffix candidates were excluded. Production teardown sources remained frozen.

## Selected caller and compared value cases

The actual route is BootstrapConfiguration→FilePass→`config_application::load_and_apply`→`read_config2`/`apply_config2`. Current application.cpp performs mutable lookup/copy operations in native order and delegates the following scalar/value algorithms to configuration.cpp and the concrete path/encoding providers. These examples are **static deductions**, not executed cases.

| Branch/input class | Observed native/current agreement |
|---|---|
|ServerPort decimal, sign, trailing bytes|019ad0 calls01d6c0's signed base10 conversion and stores low16 without an extra port-range check. `"-1junk"` therefore gives65535; `"65536"` gives0 in the current source as well. `"default"`/`"random"` are exact spellings; no case normalization was invented.|
|Decimal edge/failure|Current parse_decimal admits signed int32 minimum, rejects no digits and positive/negative overflow, and ignores trailing characters/NUL after converted digits. `"2147483648"` is out of range rather than a valid wrapped port. Native01d6c0 has the same no-conversion/range rejection. No arbitrary locale digit rule is claimed.|
|Boolean counted value|01de40 uses counted length plus exact memcmp comparisons. Empty preserves destination; exact `true`/`1` and `false`/`0` assign; other spellings use caller fallback. Current source matches, including `"True"` and counted `true` plus an embedded NUL being fallback rather than accepted true.|
|Nonempty assignment / token lengths|01e1e0 preserves prior destination on empty text and otherwise copies the counted bytes. Current application retains prior hashes/token on empty override values. Nonempty retained DenuvoToken length<=99 is cleared; length100 is retained. Hash lowercase calls are at the same point; invalid signed high-byte CRT input is not recast as a new ASCII-hash defect.|
|Achievement set/IDs|01da80 gates all changes on nonempty set, assigns that set, splits IDs by semicolon with empty parts skipped, and inserts/resets each nonempty ID. Current helper matches (`set="S", ids=";A;;A;"` resets A in order; empty set leaves state untouched). The active caller explicitly looks up IDs before Set, preserving the already-established mutable lookup order.|
|Extra DLL comma paths|01df60 leaves the old vector on empty input. Nonempty input first clears it, splits literal commas, skips empty fields and resolves/appends one at a time. Current helper matches `",a,,b,"` and retains an early appended prefix if a later conversion throws; no CSV quoting/trim was added.|
|Registry path flag/order|01e0c0 tests nonempty text then sets spoof flag before0145d0 conversion. Current source does the same, including a conversion failure after the flag write. Game CLI-language precedence does not suppress this distinct registry-path operation.|
|UTF-8/counting and relative path|0145d0 converts an explicit string length in two calls with UTF-8 codepage65001 and retains first-pass output length; current widen_utf8 does likewise.0192d0 widens first and prepends executable parent only for relative paths. Current drive/UNC/root-name logic follows that branch; no environment expansion or extra canonicalization was introduced.|
|Quoted escaping|013640 decodes only backslash-quote/backslash-backslash.0141a0 falls back to regular bytes except CR/LF; the enclosing quoted rule terminates on quote. Source matches: unknown `\q` remains literal backslash+q; it does not gain JSON escapes or reject ordinary high/NUL bytes globally.|
|Parser whitespace|0135b0's mask plus formfeed yields HT/LF/VT/FF/CR/SP. Current source uses that exact six-byte set. It does not treat arbitrary Unicode whitespace as native separators.|
|Block comments|013ed0 terminates at first `*/`; source matches, without nested-comment support. Marker-mediated failed attempts rewind where specified; direct body failures retain the already-documented cursor prefix. No new diagnostic/cursor parity claim is made.|
|Base/override scalar flow|The active apply_config2 keeps base-only name/version/content/collections distinct from shared scalar overrides; empty values do not silently erase old state. No actual file option was found that changes the above numeric/parser helpers into a HOST-only legacy implementation. This reuses the established caller wiring without reopening that candidate.|

## Exact retained helper inventory

Each of these 12 bodies was inspected, wholly or for the relevant narrow branch. The containing019ad0/014310 producer context and prior consumer audit were reused; no new all-functions search/mapping result was treated as a missing implementation.

| RVA | Purpose | Retained pseudo EID |
|---|---|---|
|01d6c0|signed stoi wrapper / no-conversion and range checks|ev_24f65e3a70566aa074382cef0ca1484a16e596e0788bb0d7ee737f4315808c0d|
|01da80|achievement set / semicolon IDs|ev_aa6ac8b4f94f3ff8a5525250c681d8d7a20eeafc192a94494e59431d8b2e21c4|
|01de40|counted boolean comparison|ev_841789359663bcf3e1c72535a4b8e1f7e0ba973419df5a9100eeaa5edf0f8c91|
|01df60|comma paths / clear-before-conversion|ev_0f777f53f1cbc552d32785f34f043481962889f43845ae957c842fe7023469fa|
|01e0c0|registry flag before conversion|ev_8df3eb3fce38faf8b755e983d17deab9a740db27a7ab0a358b9b06f97ebccb50|
|01e1e0|nonempty counted assignment|ev_eb9c130f11089cde3d30a926444fb7803630c41cdc2e0b439b966239d747d371|
|0145d0|UTF-8 counted two-pass conversion|ev_b7669ec63c588e3e0c3c3ea478118b765778ea8ceeaac18635081dec14844c4d|
|0192d0|relative-path resolution|ev_f97924b6178fd1c17ef606a5e5153562cafc5f04e36eecb08e5cdb93f2a31286|
|0135b0|ASCII whitespace mask|ev_cf672da92f1051620841d06002c7c85794df53a64e1a661fef7b17b6e83a77d8|
|013640|two recognized escapes|ev_3f60561fa8bb94e022556127045c7f9d7e9e7253dcd5415ce3b2b253860bf8fc|
|0141a0|regular-byte fallback|ev_cdcd4963cda9e1c5b368f5ff49b12a1ee37224490f162fa258f9761648eb1d52|
|013ed0|block-comment body|ev_676055f36f693f0cb9ef567b1e7e20ccf84b3f785b5f632f173ac33d25ffc423|

Retained pseudo SHA256 `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`; retained assembly SHA256 `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`; original target identity `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

## Current source authority

| Path relative to outputs | SHA256 |
|---|---|
|anadius-config-application-source/application.cpp|27e32ac1010aa4c169148f189016deb02a78c32d881f99134aed0b3a8ec0d2bb|
|anadius-config-source/configuration.cpp|a2a2fa7980ddeeaece163b705213bdc5adfff48ae149ec39a122850032e25f6c|
|anadius-parser-source/parser.cpp|700479cfca81eae57f85d1fb8786426ffcc25d49edf9fa25359d00396252d2e2|
|anadius-bootstrap-policies-source/config_platform.cpp|312238a5246e1d1ef126cd146eeb40fb12bd26ed2627d28f6356c11ff46ad28c|
|anadius-bootstrap-providers-source/providers.cpp|a4e52f6553c0acf8c45e6d095e4ab6c861344ebc438b0e9a628688364d30f87e|
|anadius-path-source/paths.cpp|303335a49823dcc91aeea53d3b593d7be3c64c21136c0478d90f043ed127f67e|

The report deliberately does not equate value-level decimal behavior with the native wrapper's CRT thread-local errno mutation, signed-byte locale behavior or exception ABI. Those effects remain separate native/selected-CRT compatibility questions; no valid value/caller mismatch requiring a numeric rewrite was shown here. Likewise no native allocator-failure equivalence follows from matching the normal source sequence.

Next action: do not patch these branches or broaden parser/locale frameworks from this negative sample. Select a different concrete application caller/effect question if further evidence identifies one. No completed-function count, coverage percentage or runtime validation credit follows from this report.
