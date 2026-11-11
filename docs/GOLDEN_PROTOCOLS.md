# Golden tools and local pipeline

The actual tools selected and adapted from Brownfield:

`MSVC Build → URSYNEX Golden Finalizer → ARYRA Golden Layout → MUSTAVEX Golden Metadata → CORVEXON Golden Trail → FALCION Golden Cleaning`

| Stage | Implementation and boundaries |
|---|---|
| MSVC Build | `check_msvc_shipping_link_command.py`; actual target linker command and only the associated response files |
| URSYNEX | `ursynex_golden_finalizer.py`; sole bounded PE mutator, atomic replacement and checksum finalization |
| ARYRA | `verify_aryra_golden_layout.py`; bounds/alignment/overlap/section-order checks, read-only |
| MUSTAVEX | `verify_mustavex_golden_metadata.py`; bounded resource/metadata/export-name checks, read-only |
| CORVEXON | `verify_corvexon_golden_trail.py`; export/ordinal/import/CRT-cookie/checksum checks, read-only |
| FALCION | `check_falcion_golden_cleaning.py`; flat source package, matching source manifest, CRC/byte/hash and file-set checks |

`run_golden_pipeline.py` runs these stages in that
order and stops at the first failure. Only a separate
temporary candidate copy is mutated: the original
build DLL is preserved, and hashes must remain
unchanged across read-only stages. Once all six
stages and the final-named DLL runtime test pass,
the pipeline writes the hash-named build candidate
and the dist bundle described below. It does not
publish to a Library or official release pointer.

The MSVC link-command collector accepts only unredirected, target-local
CMake `link.txt` or `build.make` files beneath the selected build root. A
redirected direct rule fails explicitly; fallback discovery excludes aliases,
even when they resolve inside that root. Only response files referenced by
the selected invocation contribute linker-policy evidence. Portable fixtures
check this path policy; actual MSVC/Golden acceptance requires native output.

## Current product targets

Following the owner's instruction to complete missing
targets, four profiles entered `READY_TARGET`
status. These are Citrine Eclipse Mirage product rules;
fixed fixture identities under
`tests/support/golden/` are not activated.

| Profile | Current rule |
|---|---|
| URSYNEX | Preserve the native link output compilation timestamp; apply linker 14.0 and the no-Rich-header profile to the copy, then recompute checksum. Permute complete import descriptors into the fixed DLL order; preserve names/thunks/IAT data. |
| ARYRA | MSVC six sections, or seven with the static UCRT `.fptable`: verify ordering, alignment, file bounds and non-overlap. `.fptable` must have virtual 0x100, raw 0x200, index 4, characteristics 0xC0000040 and all-zero data. Other section RVAs/sizes are not fixed. |
| MUSTAVEX | AMD64/PE32+/Windows GUI, OS/subsystem 6.0, linker 14.0, native compilation timestamp preserved, no Rich header. Citrine Eclipse Mirage VERSIONINFO, Copyright 2026 SURGE, and matching selected numeric/string file and product versions. No PNG/RCDATA/manifest or COFF symbol/debug/certificate. Internal export name `citrine_eclipse_mirage.unsigned.dll`. |
| CORVEXON | Exactly three C exports/ordinals. The CRT entrypoint is separate from exported DllMain and lies in executable `.text`. No static imports outside CRYPT32/WS2_32/KERNEL32/USER32/SHELL32/MSWSOCK; MSWSOCK only AcceptEx/GetAcceptExSockaddrs, CRYPT32 only CryptQueryObject. `/MT`, writable image-backed `/GS` cookie and valid checksum. |

CORVEXON maps section RVAs to file bytes only within `SizeOfRawData`.
A section with zero raw size cannot supply file bytes, regardless of its raw
pointer or virtual size. Virtual-only sections may remain in the image, but
their RVAs cannot supply bytes to file-based checks. Ordinary raw-backed
section mappings and header-RVA mappings retain their existing behavior.

OLE32 originated in the earlier COM known-folder adapter
and was removed by restoring RE environment-variable
lookup. The final owner correction uses
`crypt32.lib` and the direct SDK
`&CryptQueryObject` target for hook 11;
CRYPT32 is a normal PE import. The earlier
five-module restriction excluded this authorized
extension and was corrected. PNG is ordinary
read-only C++ data. The owner's project identity decision supersedes reference
VERSIONINFO branding and random version values. Other resource structure and
Golden checks remain strict. Synthetic fixture metadata remains independent.

```sh
python -B scripts/run_golden_pipeline.py --check-target
```

A `READY_TARGET` / exit 0 result from that
command only shows that target definitions are
ready. Full Golden requires `build_production.cmd`
to build an actual MSVC Release DLL and run all
six stages. The first stage verifies compiler
identity, linker command, and source/build
receipt hashes. LLVM/Wine or a synthetic PE
does not replace MSVC build evidence.

Exit 0 from the full pipeline represents only
Golden acceptance. Exit 1 is a validation
or build error; exit 3 means BLOCKED because
a required product target is missing. Target
definition readiness is not actual MSVC Golden
or game PASS. A successfully validated DLL
receives a companion `.dll.sha256`. The source ZIP packager also
creates a `.zip.sha256` beside each archive.

Candidate hashes must remain unchanged during
all read-only stages. URSYNEX may change only
permitted linker, full Rich-header block and checksum fields, plus the
explicit import-descriptor permutation described below; the native
compilation timestamp must remain byte-exact; any other byte
mutation must stop the pipeline. The original
build DLL's unchanged bytes are also checked
at the end. Fixture and malformed-input
regressions are retained separately; fixtures
cannot activate actual product policy.

## Dist output

After all six stages, the finalized copy is
passed to the existing runtime host as
**citrine_eclipse_mirage.dll**. Three actual
load/start/socket/profile/stop/unload cycles
must pass without mutation of the candidate
bytes. Only then are these files written:

- `dist/citrine_eclipse_mirage.dll`
- `dist/citrine_eclipse_mirage.dll.sha256`
- `dist/DIST_RECEIPT.json`

The distribution name is the current project identity. Hook 11 deliberately
retains the independent `preloader_l.dll` compatibility suffix in
`include/citrine_eclipse_mirage/platform/crypt_query.hpp`; neither renaming the
shipping DLL nor running its host test proves selection of that suffix.
The raw link output remains
`citrine_eclipse_mirage.unsigned.dll`; copying does not
change the export-directory internal name
or the three C exports. The earlier
hash-named build candidate is also retained.
If Golden or final runtime validation fails,
no new dist is published; any prior successful
bundle is preserved with its hash/source
receipt. A persistent OS-held writer lock and a verified
rollback journal under `dist/.golden-transaction`
protect the three public filenames against process termination
between replacements. On the next local pipeline invocation
the previous receipt-bound bundle is restored **before** any
Golden stage runs. Recovery can also be requested explicitly
with `python -B scripts/run_golden_pipeline.py --recover-dist`.
The `.golden.lock` pathname is intentionally retained;
actual lock ownership is released by the OS when a writer exits.
A missing or tampered backup fails closed without overwriting
more files. A terminated writer can leave a temporarily mixed
bundle until recovery; direct consumers must not treat an
unverified dist directory as atomic publication.
Process-exit regressions cover interrupted promotion and replay;
NTFS crash recovery, junction/ACL races, power-loss durability and
Consumer acceptance remain separate evidence.
Results of the current attempt are
in `golden/GOLDEN_REPORT.json` under
the build directory. Compile, Debug, Native,
and CRT profiles cannot be promoted to dist,
and no official publication occurs.

All child Python interpreters use `-B` and/or
inherit `PYTHONDONTWRITEBYTECODE=1`.
The operator flow first removes only known
`__pycache__/*.pyc` files from manifest-owned
first-party Python tools. FALCION continues
to reject unexpected caches or additional
source files. See
[Production correction](PRODUCTION_IMPORT_FIX.md).

The [metadata audit](REFERENCE_METADATA_AUDIT.md) preserves the historical
field matrix and former fixed-timestamp decision. Current timestamp and
VERSIONINFO rules are the product targets and owning policies above.

The owner requires the same import descriptor order in every MSVC
application DLL profile, including `citrine_eclipse_mirage.unsigned.dll`. The shared
`scripts/import_descriptor_order.py` normalizes the unsigned image immediately
after the linker completes and before the build partition gate and raw-asset
inspection; `build_profile.ps1` independently verifies the resulting
unsigned DLL on both clean and incremental builds. It uses the exact same
six-module order as URSYNEX/CORVEXON/DIST, changes no other bytes, and
fails without replacing the file if the import module set is unexpected.

Mutating unsigned normalization rejects leaf symlinks and DLL files with a
link count other than one before reading or staging output. Read-only
`--verify` retains its existing behavior. This prevents a replacement from
certifying one pathname while an aliased build DLL remains unchanged; it is not
parent-directory reparse protection or a same-credential race guarantee.

This is explicitly **post-link normalization of the unsigned artifact**, not
a claim that MSVC LINK.exe itself emitted the descriptor records in this
order. The final Golden step remains independent, idempotent for already
ordered imports, and responsible for its separate metadata/checksum work.
Development source-API and test fixture DLLs are outside the six-import
shipping application contract.

The owner's latest import-order correction
permits URSYNEX, in addition to the
header/Rich/checksum fields above, to
permute only complete import-descriptor
records into CRYPT32/WS2_32/KERNEL32/
USER32/SHELL32/MSWSOCK order. Descriptor
name/RVA fields move with their own
records; import thunk/name/IAT bytes and
directory locations remain unchanged.
Reject the operation if the module set
differs. Recompute checksum afterwards.
CORVEXON verifies ordering read-only.
This explicit permission extends the
earlier header-only mutation scope.
