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

## Current product targets

Following the owner's instruction to complete missing
targets, four profiles entered `READY_LOCAL_TARGET`
status. These are Yellowfield product rules;
fixed fixture identities under
`tests/support/golden/` are not activated.

| Profile | Current rule |
|---|---|
| URSYNEX | Apply Brownfield's explicitly selected timestamp 0xCD268EB1, linker 14.0 and no-Rich-header profile to the copy, then recompute checksum. Following the owner's later instruction, permute complete import descriptors into the fixed DLL order; preserve names/thunks/IAT data. |
| ARYRA | MSVC six sections, or seven with the static UCRT `.fptable`: verify ordering, alignment, file bounds and non-overlap. `.fptable` must have virtual 0x100, raw 0x200, index 4, characteristics 0xC0000040 and all-zero data. Other section RVAs/sizes are not fixed. |
| MUSTAVEX | AMD64/PE32+/Windows GUI, OS/subsystem 6.0, linker 14.0, timestamp 0xCD268EB1, no Rich header, Brownfield VERSIONINFO byte-for-byte. No PNG/RCDATA/manifest or COFF symbol/debug/certificate. Internal export name `preloader.unsigned.dll`. |
| CORVEXON | Exactly three C exports/ordinals. The CRT entrypoint is separate from exported DllMain and lies in executable `.text`. No static imports outside CRYPT32/WS2_32/KERNEL32/USER32/SHELL32/MSWSOCK; MSWSOCK only AcceptEx/GetAcceptExSockaddrs, CRYPT32 only CryptQueryObject. `/MT`, writable image-backed `/GS` cookie and valid checksum. |

OLE32 originated in the earlier COM known-folder adapter
and was removed by restoring RE environment-variable
lookup. The final owner correction uses
`crypt32.lib` and the direct SDK
`&CryptQueryObject` target for hook 11;
CRYPT32 is a normal PE import. The earlier
five-module restriction excluded this authorized
extension and was corrected. PNG is ordinary
read-only C++ data. Brownfield's VERSIONINFO
file bytes remain unchanged.

```sh
python -B scripts/run_golden_pipeline.py --check-target
```

A `READY_LOCAL_TARGET` / exit 0 result from that
command only shows that target definitions are
ready. Full Golden requires `build_production.cmd`
to build an actual MSVC Release DLL and run all
six stages. The first stage verifies compiler
identity, linker command, and source/build
receipt hashes. LLVM/Wine or a synthetic PE
does not replace MSVC build evidence.

Exit 0 from the full pipeline represents only
local Golden acceptance. Exit 1 is a validation
or build error; exit 3 means BLOCKED because
a required product target is missing. Target
definition readiness is not actual MSVC Golden
or game PASS. A successful local candidate
receives a companion `.dll.sha256`; nothing
is published. The source ZIP packager also
creates a `.zip.sha256` beside each archive.

Candidate hashes must remain unchanged during
all read-only stages. URSYNEX may change only
permitted timestamp, linker, full Rich-header
block and checksum fields; any other byte
mutation must stop the pipeline. The original
build DLL's unchanged bytes are also checked
at the end. Fixture and malformed-input
regressions are retained separately; fixtures
cannot activate actual product policy.

## Local dist output

After all six stages, the finalized copy is
passed to the existing runtime host as
**preloader_l.dll**. Three actual
load/start/socket/profile/stop/unload cycles
must pass without mutation of the candidate
bytes. Only then are these files written:

- `dist/preloader_l.dll`
- `dist/preloader_l.dll.sha256`
- `dist/LOCAL_DIST_RECEIPT.json`

The runtime name thus matches the Bluefield
hook-11 filter. The raw link output remains
`preloader.unsigned.dll`; copying does not
change the export-directory internal name
or the three C exports. The earlier
hash-named build candidate is also retained.
If Golden or final runtime validation fails,
no new dist is published; any prior successful
bundle is preserved with its hash/source
receipt. Results of the current attempt are
in `golden-local/GOLDEN_REPORT.json` under
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

The full field matrix and correction of the
former preserve-compiler decision are in
[metadata audit](REFERENCE_METADATA_AUDIT.md).

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
