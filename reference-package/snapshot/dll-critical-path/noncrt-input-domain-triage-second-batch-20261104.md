# Application input-domain triage, second batch — 2026-11-04

This separate bounded sample found one concrete production input restriction: achievement Grant timestamps with a complete ISO prefix and a high-byte suffix were rejected by the selected ClassicIso provider, although the native prefix-parser accepts completion without full-input validation. Fresh focused native evidence subsequently confirmed it, and root authorized the two-line scan removal. This is an application persistence input-domain correction; it does not replace the native locale/CRT parser or claim its ABI complete.

Detailed authority and implementation are under `outputs/anadius-timestamp-source/nonascii-suffix-20261104/`:

- `native-findings.md` SHA256 `5e7213377ad52092170db092a5c08b7988a801f456a5c8de76b563c5f14287cf`.
- `native-evidence-index.json` SHA256 `8870cab73f95ccdb3fbd94f73cda30a438a02b7b97c5d99262a1e69b7f300722`; 13 full fresh records, 209826-byte bundle, isolated REA6.8.0/Ghidra12.1.4 session exported/closed, original SHA256 unchanged.
- `implementation.md` SHA256 `f3542e541c074a366db5a0795e7bdb87332153530acceff7563a3a7deba26827`.

Selected source chain: `config-storage-source/persistence.hpp:18` selects ClassicIso, `persistence.cpp:29` supplies it to conversion, `achievement-storage-source/storage.cpp:75` retains Grant bytes and line89 converts timestamps when SteamAppId is nonempty. The old source `timestamp.cpp` SHA256 `443dc4a0dea62b717d4513f6d8432f84f8d51ff6edf3bc26bd12f6578c02e52c` globally scanned every byte and threw for any byte>=80. The corrected cpp is `4b55091f581f50cee7f363a16f5e355d1b4cce7539ebc301d1840914bfb251c7`; corrected hpp `6c71bab0ffd3d6527848582abe4240e228da2e93f997ea2bfbda17bf5c0095b2`. Numeric/width/range/state/calendar algorithms are unchanged. Test sources are independently owned by root; no tests/builds were run by this audit.

## Bounded retained-body inventory

Twelve additional retained bodies (whole or focused relevant branches/instructions) were inspected beyond the first 20-body report. No broad Online/activation equivalence claim follows from source inventory reads. Source mapping and retained call relationships only selected candidates; address presence/absence did not establish a defect.

| RVA | Purpose / result | Retained pseudo EID |
|---|---|---|
|018480|command-line option filter: exact spelling, empty language and consumed next-token behavior matched current startup source|ev_691d99dfac41c8e1f0484feff33584d6291e41fff99d694fff05e78775aefb04|
|019910|environment override vs argv selection and space splitting matched current startup source|ev_90684472bf3c11d29dce5129d1c8a0e460e645004eaaef332b7623a91a4f03f3|
|02e5f0|service/facility response enumeration matched current fixed services array|ev_08c9cbb7d94139171f29ba86d86cd51101e76b2d6d913d183d8a51258f2dec5b|
|02efd0|GameInfoId lookup: absent/missing key false, second lookup after response open matched source|ev_ef8266017da897f5345aadbbdc4f690c0da75a591445b04cdc14b094c75ab673|
|02f7c0|SettingId lookup: same existing-key/second-lookup behavior matched source|ev_1b508c22e6541f42d9a3d84a7baa8e9429663fa9564d19f2080abc16619c08eb|
|02fa60|six inserting settings lookups and XML order matched source|ev_43da82c04eb58457c7bb8bb746f2ea56e82a26751dfe07349fc109fe011f1194|
|041e10|achievement timestamp extraction/caller fail mask; concrete global-precheck gap|ev_80df4678956b9b7658e3d0b5553defdaa1244bbcda5a6974597668481318d415|
|03d310|time_get facet selection and extraction state merge|ev_f4f2a1eb680a7fef54367c61f2f66e8b895c9fcacdc2fd88f07732159de7dea9|
|041760|format-end precedes next input validation|ev_d534decdbd37d824b4edeba1aa8b87354fb56ff73514b0f0a0b5a017fcfdf952|
|03d8f0|bounded numeric extraction, peek/width/EOF/fail distinctions|ev_5cc1de95c661c1e2eb1d38d7c96f2d8aa607569c484e2e5f97434d7d1283b971|
|03f190|focused fixed-format seconds dispatcher and final iterator-state branch|ev_a47bfad1bd8e9cd6e0bac59d9333647dcc99a349b817cd04a45e6332d2dbb91d|
|03e870|facet table installation, focused retained instructions plus fresh exact function|ev_631fc88d3cc57123aa3a67ab95f8f9d818ae598689c4b56a5e8e6b9fd15d50e0|

Mapped caller chain includes 038970→019910→018480 and 01eb00→041e10→03d310→041760. Handler02e5f0/02efd0/02f7c0/02fa60 have explicit source dispatch in `anadius-dispatch-source/router.cpp`; their empty heuristic direct-caller lists do not prove they are unused. The fresh native stage additionally inspected wrapper03ef80 and vtable slot bytes to resolve the actual `%S` call, rather than trusting inlined pseudo-code alone.

Current comparator hashes: `anadius-startup-source/startup.cpp` `87bcd90892dcea301fc3c1aceb589d1097875d9400d3bd69b64570d008740dea`; `anadius-handlers-source/handlers.cpp` `8180eb0f47bca1708c05ec2f6c72ef4e54da67a832cbcc5ad4b482e46a09c400`. Full retained artifact and heuristic-map hashes remain those recorded in the first-batch report. Online source reads (`online.cpp` `00020732bf6e36d53b9bfafe80d6cb85f303b8b329624562d45991f916e50cb8`) and license source reads (`license.cpp` `87ea1eebab23fa2bbaff52689e4a076428e5cd8f292e028fbba985d4ce4376c0`) only screened candidates; no native Online/license body audit or new equivalence was claimed in this batch.

Search stopped after the concrete timestamp discrepancy was confirmed and corrected, below the 20-additional-body limit. Already-closed config-duplicate/XML-status/response-family questions and the parallel WSAstartup ownership/order task were excluded. Remaining global locale/custom facet behavior, native support allocation/EH/ABI, and other application domains are still separate questions; this bounded result gives no whole-DLL percentage or completion credit.
