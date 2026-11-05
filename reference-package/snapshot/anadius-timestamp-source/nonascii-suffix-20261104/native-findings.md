# Timestamp suffix / byte-domain audit — 2026-11-04

Confirmed bounded source mismatch: the default ClassicIso provider globally rejects any non-ASCII byte, including an unread suffix after a complete ISO timestamp. The native fixed-format route has no such full-input validation. Removing that scan preserves the recovered classic grammar while allowing the original prefix-parse behavior. No source edit had occurred when this evidence report was written.

## Identity and evidence

`binary-session.json` verifies REA 6.8.0 at `C:/tools/rea-6.8.0/node_modules/rea-agents/scripts/rea.mjs`; Ghidra 12.1.4 is supported and available under `C:/tools/ghidra_12.1.4_PUBLIC`, Java 25.0.5. `open-binary.json` identifies the original immutable x64 PE. Own isolated session was exported to `native-evidence-bundle.json` (13 records, 209826 bytes, zero unknown records), then closed normally (`close-binary.json`). Original SHA256 after close remains `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. No target execution, compilation, tests, linking or packaging occurred.

Full structured replies, SHA256 digests and EIDs are indexed in `native-evidence-index.json`. This is static control-flow/value evidence, not a replay of any example input.

## Exact control flow

- **041e10 input/consumer:** `0x180041efb` loads the source string's explicit byte count, `041f17` passes data+length to string-buffer construction; this is not a whole-input C-string scan. `041f26` selects `%Y-%m-%dT%H:%M:%S`; `041f49` calls extraction. `041f57` tests only fail/bad mask 6; a clear mask calls calendar conversion at `041f62`. There is no trailing-byte/full-consumption check. EIDs `ev_bb488e7c9c05d541688f19d1bd20db92dab969ad2657820771861c4d59b50ae8`, `ev_0f2ba5e57e103ffe9cf8f9f408e34543a14795dd71b240dab144dc9ff33d3375`.
- **Actual facet binding:** fallback construction at `03e942–03e949` installs time_get vtable `0x1800869f0`; fresh bytes at `0x180086a38` (+0x48) are `90f1038001000000` → `0x18003f190`. This ties the format driver's virtual per-field call to the measured field dispatcher in the base-facet domain. EIDs `ev_572a6afc6e3d230a173dedc4f41555ae2dd681965209965746617d7c580ff031`, `ev_8fbefcadbd4dba9b6d8c9c3dbbe29ad7b5cbadbbd35efedd2c758d5e7b8ae6ef`, `ev_269637d3b846b14306c703a0bfcab02ceb7e77b39a0ef8a5c662a3e72f0b0a2a`.
- **Format completion before another input check:** `0417f2–0417f5` initially checks format-end. At each field end `041b31` increments the format pointer; `041b34–041b3d` exits immediately when it reaches format-end. Thus the completed final `%S` is not followed by validation of remaining input. Native state/non-end mismatch paths remain distinct. EIDs `ev_49fd2c63ab063676ce0d34d3f486816559bcf701596943b7a02c1d37a88227ff`, `ev_64192c7682feb63591798acc439eeb35a03eb1893f41647edeab8428a6f41a82`.
- **Final seconds and wrapper:** dispatcher case 0x53 at `03fdbe–03fdd4` supplies maximum 60, routes through `03fc1f–03fc2b` to `03ef80`; wrapper `03efc1` calls `03d8f0`. The pseudo-code inlines that wrapper; instructions establish the actual extra call. EIDs `ev_7f58fdbb52591779a7cc4713cb437d2294b8fb6357d8f82796eeff2f3a49fc17`, `ev_a851dc58c87e63921ae8f33f1f14b69bc17bd8a96150e0f7581d84e0b150c813`, `ev_6587e9fd26ef25425babc8a6012e9fed5c3f3cc7394fa49b72df193f0db1d1e4`, `ev_df380ed7569cd5e7d4adbf9c464cac4fcd6b96eb8955e518ef720ddcb0a2a652`.
- **Numeric peek vs consume:** number extraction derives a two-character field width from maximum 60. After digits it can peek the next byte (`03de24–03de39`); it stops if that byte is not a narrowed decimal digit (`03de42–03de45`) OR the width is already reached (`03de4b–03de4f`). It does not consume the suffix on either stop. It validates converted/range result at `03decd–03deef`; only conversion/range failure adds bit 2. End iterator comparison supplies EOF bit 1, not bad bit 4. The final field dispatcher likewise only adds EOF when its iterator actually equals end (`04035b–04038d`). A nonempty suffix therefore need not be entirely unread at the peek level; it is unconsumed and is not globally validated. EIDs `ev_b6352e8a39be4af80496d2ac3923b93a0cde39c65b923c54252b857a8479eb8e`, `ev_560f6f4161dd17d86960ac9615dbcbbc8a150f4757d817c73eaba02bbf06535b`.

## Source reachability and bounded examples

`anadius-config-storage-source/persistence.hpp:18` defaults to ClassicIso. `persistence.cpp:29` supplies it to timestamp conversion. `anadius-achievement-storage-source/storage.cpp` reads the Grant attribute as a C-string-bounded byte view, retains its UTF-8 bytes, and at line 89 converts each achievement's timestamp when SteamAppId is nonempty. Thus a persisted XML Grant such as `2000-01-01T00:00:00` followed by UTF-8 `c3 a9` can trigger the mismatch: the measured native fixed-format route accepts the parsed prefix, whereas the old source precheck throws before parsing. Native early zero return on fail is different from that exception.

Under the existing explicit classic/base-facet projection:

- Complete ISO timestamp + nonempty arbitrary byte suffix (including 00, 80, ff, UTF-8 or ordinary ASCII suffix): final parsed fields succeed, state 0 because bytes remain; suffix is not required to be valid text. A suffix-free exact timestamp can return EOF bit 1, which 041e10 accepts.
- Non-ASCII/NUL **inside a required separator**, before completion: separator mismatch sets fail; no global exception is needed. Non-ASCII/NUL replacing the first digit of a required numeric field fails conversion. A nondigit after an already valid partial final seconds field can terminate that number successfully; do not replace this with a blanket "all non-ASCII inside fields fail" rule.
- Explicit-length native input means an embedded NUL is an ordinary stream byte, not EOF. A NUL after the completed timestamp is allowed by prefix parsing. An XML attribute boundary may truncate earlier at its first NUL; these are different layers.

No arbitrary global/native locale replacement, custom-facet equivalence, stream/allocator/EH ABI completion, or reentrant facet behavior is established. The correction should remove only the global byte restriction and preserve the existing fixed-format classic comparisons and numeric state algorithm.

## Pre-change source hashes

| Source | SHA256 |
|---|---|
|anadius-timestamp-source/timestamp.cpp|443dc4a0dea62b717d4513f6d8432f84f8d51ff6edf3bc26bd12f6578c02e52c|
|anadius-timestamp-source/timestamp.hpp|bdf47aadc38b11393e9317ed9c2e4b43067f8c81c02256a4001289da31ba017e|
|anadius-config-storage-source/persistence.hpp|e3b7167f45394b7cb9b9a4f697909f05ce9e1f9ffa04664da8dfce51dba6f9ce|
|anadius-config-storage-source/persistence.cpp|6ffbee117646ae15491c5e22383cd8d3adb351a1137991c14a060f2d0371967b|
|anadius-achievement-storage-source/storage.cpp|2a2f23e278f35e8920047c2a9444e1fd150220d01bb2d727d1f3378a43684d13|
