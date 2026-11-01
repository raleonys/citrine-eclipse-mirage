# Third-party known vulnerabilities — active dependency register

This register covers **all current and future vendored third-party dependencies**, not only TinyXML2. The machine-readable source of truth in this source tree is `metadata/THIRD_PARTY_KNOWN_VULNERABILITIES.json`; the Library-owned `KNOWN_VULNERABILITIES_THIRD_PARTY` register is its governance master for the current review.

- TinyXML2: exact upstream commit `8224e427b655b83dae5e2298f1e6919523a78737`; A02-A entity data corruption is open (issue #1082 and unmerged PR #1086); A02-B non-strict invalid reference acceptance is open. Neither has been fixed by changing the vendored upstream. No CVE or LSX exploit is asserted.
- MinHook: exact vendored commit `8af6b4acae5a9388fd742b56fa79ece89d96f823`; there is no **independently verified** open upstream vulnerability in current evidence. That means monitoring required, **not** that no bugs exist. The project applies the existing SAFE_FREEZE patch only to the build-tree copy; MinHook source is untouched in this update.

Review each new upstream merge, issue or release and its affectedness; explicit user approval is required before any dependency source change. Revalidate pinned provenance, regressions, 8-shard source QA in the working tree and clean extraction, and affected native/CRT/game acceptance before release.

Open risk records are not waiver, release approval or evidence that a defect is externally exploitable. Current HF10 remains canonical until independent HF11 publication acceptance.


## HF12 cumulative intermediate vendor coverage (13 October 2026)

The in-source governance mirror now additionally tracks **TAO PEGTL** (`3b075432e94f6f906425350b43d03cd9cebfa4f1`, BSL-1.0, shipping grammar), **gulrak/filesystem** (`3812f8a6f31ed67d32272ac33fe2b85c6b2c947e`, MIT, shipping path only, upstream WIP), and **Standalone Asio** (`8806a6803cde7054c3049d3666d3ec36786568c5`, BSL-1.0, **vendored test-only**).

For these three dependencies `issues: []` means no verified issue has been added to the active register; **no independent full security/advisory/CVE scan has been completed**. It is not an assurance of safety. Native Windows and game acceptance is pending for PEGTL/filesystem; Asio's shipping backend is not yet integrated. The WIP designation is a compatibility/support risk, not a security finding by itself. Per-file vendor pin/license/byte integrity and the exact usage statuses are enforced by `metadata/DEPENDENCY_SOURCE_CONTRACT.json` and the existing repository QA. No silent version update or vendor fork is permitted; user authorization is required before any source-byte change. The original TinyXML2 A02-A/A02-B findings and MinHook records remain unchanged.
