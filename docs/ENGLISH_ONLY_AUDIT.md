# First-party English-only audit boundary

This is an active first-party content audit, **not** a claim that the
entire repository has achieved semantic English-only compliance.
The repository's technical identifiers, historical evidence and
third-party snapshots are independent preservation contracts.

## Exact inventory and baseline

The starting snapshot is the accepted `main` commit
`4fb1f23882b7118320380901148ea60b6693cbf9` after PR #47.
GitHub's recursive tree query returned 3,966 entries without truncation.
Excluding `third_party/`, `metadata/evidence/` and
`metadata/vendor_files/` leaves **708 tracked first-party files**,
including non-text assets. This is an inventory count, not a count of
files requiring translation.

| Active family | Tracked files |
|---|---:|
| `src/` | 139 |
| `include/` | 138 |
| `tests/` | 250 |
| `scripts/` | 45 |
| `docs/` | 58 |
| `metadata/` (active root entries) | 43 |
| `cmake/` | 9 |
| `tools/` | 6 |
| `.github/` | 4 |
| `resources/` | 4 |
| `governance/` | 1 |
| `exports/` | 1 |
| Root-level files | 10 |
| **Total** | **708** |

Historical vendor/evidence content is intentionally outside the
migration scope. Do not alter its original bytes to make language
counts appear cleaner. Data fixtures containing deliberate Unicode
characters are not automatically prose requiring translation.

## Reproducible first-party check

```sh
python -B tests/infrastructure/test_english_only_audit.py
python -B scripts/check_english_only.py --check
python -B scripts/check_english_only.py --json
```

The checker uses `metadata/ACTIVE_SOURCE_MANIFEST.json` to
enumerate pinned active files, rather than traversing untracked
working-tree additions. It rejects missing/symlinked active text
files and unsafe inventory paths. For each UTF-8 text file it
records the file, line and Unicode code-point identifiers when
a non-ASCII **letter** occurs.

- Markdown prose: signals block the `--check` gate and require
  editorial review; invalid UTF-8 also blocks.
- Source, tests, scripts, metadata, fixtures and other text:
  signals are **review required**, not automatic failures.
  A C++ string, parser fixture or identifier can carry
  deliberate Unicode without violating the prose policy.
- Vendor snapshots, locked historical evidence and binary assets
  are excluded from the scan. Neither exclusion authorizes
  modifications to those files.
- The `--json` output is deterministic and can be saved outside
  the tracked source tree for triage. The ordinary output is bounded
  to a short preview.

**Limitations:** Character-based scanning cannot determine language
or certify ASCII-only prose. Code comments, string literals,
configuration messages, product-visible text and text with no
language-specific characters still need human semantic review.
A clean gate is not a public-readiness or native acceptance result.

The checker and its isolated tests are included in the source-QA
sequence, so the same repository source determines both policy
and reproducibility. The scope is intentionally read-only:
it does not rewrite, re-encode or publish source files.

## Confirmed content and protected follow-ups

The immediate preceding translation batches completed the
first-party `docs/*.md` prose pass. The remaining identified
foreign-language prose in `tools/README.md` is included in
PR #48. Active top-level metadata and scripts received a
character-level screening; that screening alone did not
establish the meaning of every field or code string.

The root `README.md` still includes a pre-transfer GitHub
Actions link and historical public-facing naming. That is
a **separate public-identity/README decision**, not an automatic
translation or authorization to rename C++ targets, ABI exports,
resources or release artifacts.

`GOVERNANCE.md` creation and movement of
`governance/CURRENT_SCOPE.md` remain deferred by owner decision.

## External compiler diagnostic language

The first-party language audit examines repository-authored text, not messages
loaded from a user's Visual Studio installation. MSVC's diagnostic resources
may be localized independently of the source language; a Turkish C4834 warning,
for example, is not necessarily a Turkish diagnostic string in our source.

The shared `scripts/build_orchestration/build_profile.ps1` now sets
`VSLANG=1033` (US English) after the Visual C++ environment is loaded and
before running CMake, the MSVC compiler, linker, and test tools. This request
applies to Compile, Debug, Native, CRT and Golden builds. It is scoped to the
build process, and the existing `finally` block restores the operator's
original environment.

English compiler message resources must be available in the installed Visual
Studio toolset. The locale request cannot translate arbitrary operating-system
or vendor diagnostics, and it cannot certify zero warnings. If the installed
toolset still produces localized messages, check that English language resources
are installed and capture the actual compiler message-code and toolchain details.
A freshly downloaded source snapshot is also needed to verify fixes merged
after an earlier ZIP was created.

## Acceptance boundaries

Review the changed files and both source pin ledgers before
promotion. Require complete local source QA, followed by the
relevant hosted source/portable checks. Windows/MSVC, Golden,
game acceptance, repository visibility and release publication
remain separate gates. No such acceptance is inferred from a
language report.
