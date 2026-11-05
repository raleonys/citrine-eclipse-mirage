# Classic timestamp suffix correction — 2026-11-04

Production changes are limited to `timestamp.cpp` and `timestamp.hpp`. Removed the two-line whole-input non-ASCII rejection before ClassicIso's parsing loop. Updated its public contract comment: the fixed-format grammar remains the explicit classic/base-facet projection, and the unconsumed suffix can contain arbitrary bytes. The existing number/sign/width/range logic, separator comparisons, EOF/fail/bad handling, calendar conversion, HostGetTime provider and selected PersistenceResources wiring are unchanged.

This closes the measured input-domain restriction identified in `native-findings.md`: native 041e10 accepts format-prefix completion and only tests fail/bad mask 6; format driver 041760 ends before full-input validation; final `%S` dispatch 03f190→03ef80→03d8f0 can peek but not consume a remaining suffix. A persisted Grant with an ordinary complete ISO prefix plus UTF-8 suffix no longer throws solely because the suffix contains high bytes. High bytes in required separators or at a first required numeric digit flow through the existing classic mismatch/conversion-failure rules. No blanket rule that every high byte within a partially parsed field must fail was added.

The root captured actual before files and pending status before authorization under `before/` (plain filenames). The pre-change bytes exactly match the previously recorded source hashes. This author changed no tests, status, other production files, or original binaries. Root owns the separate test-source update. No compiler, tests, linker, DLL execution, ZIP/package operation or further REA query was run after the native session closed. These are source changes with static evidence, not runtime validation.

| File | Before SHA256 | Final SHA256 |
|---|---|---|
|timestamp.cpp|443dc4a0dea62b717d4513f6d8432f84f8d51ff6edf3bc26bd12f6578c02e52c|4b55091f581f50cee7f363a16f5e355d1b4cce7539ebc301d1840914bfb251c7|
|timestamp.hpp|bdf47aadc38b11393e9317ed9c2e4b43067f8c81c02256a4001289da31ba017e|6c71bab0ffd3d6527848582abe4240e228da2e93f997ea2bfbda17bf5c0095b2|

Native authority: 13 full fresh records in `native-evidence-index.json` (SHA256 `8870cab73f95ccdb3fbd94f73cda30a438a02b7b97c5d99262a1e69b7f300722`), `native-evidence-bundle.json`, normal export/close and original post-close SHA256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. The report explicitly bounds classic grammar and does not claim arbitrary native locale/custom-facet/allocator/stream/EH ABI equivalence.
