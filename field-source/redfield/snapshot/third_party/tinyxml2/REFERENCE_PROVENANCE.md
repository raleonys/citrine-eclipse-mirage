# TinyXML2 production source provenance (HF11 experimental)

- Upstream repository: `leethomason/tinyxml2`.
- Exact source Git commit: `8224e427b655b83dae5e2298f1e6919523a78737` (untagged `master` snapshot provided by user).
- Source-reported version remains **11.0.0**; **not** a newly tagged 11.0.1 or 12.0.0 release.
- Vendored files `tinyxml2.cpp`, `tinyxml2.h`, `LICENSE.txt` are imported byte-for-byte without local patches.
- `metadata/TINYXML2_REFERENCE_PROVENANCE.json` retains per-file source integrity hashes and the historical 11.0.0 tag identity independently of the current commit.
- Archive ZIP SHA-256 is evidence of transport/file integrity, **not** an alternative upstream version identity; the Git commit is the primary source revision.
- Archive ZIP comment asserted this exact full commit SHA, and the corresponding public GitHub commit was independently checked. That does not constitute a local Git commit-object byte verification.
- **Security status:** upstream Issue #1082 / PR #1086 data-corruption fix remains unmerged in this snapshot; invalid named/numeric entity handling (A02-B) remains open. Do not assert strict XML 1.0 input validation.
- **Release status:** Candidate12 experimental source only; official HF10 remains canonical. Full eight-shard QA, exact Windows/MSVC/CRT/native and CollegeFB27/Madden27 acceptance are not inherited from Candidate11.
