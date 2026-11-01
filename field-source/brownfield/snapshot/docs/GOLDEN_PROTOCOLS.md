## HOTFIX07 allowlisted curio recovery (2026-10-11)
Reference binary SHA-256 `a9d964beec1a82b13e3172228f9b03cbd661bcd0123eca1fca7bf7ef5fd9ba8b` is an input PE, **not** a compiled HOTFIX07 DLL. Only VERSIONINFO fixed numeric FileVersion/ProductVersion fields (#1/#2/#3) and PE TimeDateStamp `0xCD268EB1` (#11) are recovered. `resources/preloader_version.rc`, MUSTAVEX and URSYNEX contracts are synchronized. The reference contains Authenticode, debug and a non-allowlisted resource type, all excluded. Golden verification on an actual newly linked HOTFIX07 DLL remains pending.

# Golden protocols

## 3.2.0 current-status note

The Golden authority model is inherited. Exact current-candidate Windows/final-binary acceptance remains pending. The detailed predecessor contract follows unchanged.

## HF12 /MT static UCRT `.fptable` acceptance (2026-10-13, non-release candidate)

The user-approved static CRT migration permits one additional `.fptable` in ARYRA, **only** when the shipping runtime contract is `/MT (MultiThreaded)` and the actual final PE uses the exact seven-section order `.text .rdata .data .pdata .fptable .rsrc .reloc`. The historical six-section structural profile stays supported and unchanged. The seventh section must have `VirtualSize=0x100`, `SizeOfRawData=0x200`, section flags `0xC0000040` (initialized, readable/writable, non-executable) and zero-filled raw bytes, as observed in the user-built MSVC static-UCRT DLL. These exact `.fptable` attributes are a **toolchain-specific whitelist**, not a general Windows ABI invariant; unexpected drift requires review, never automatic widening. All preexisting alignment, bounds, overlap, VA/raw-order and later Golden controls remain mandatory. ARYRA stays read-only; there is no section deletion, merge, byte patch, or binary repackaging. The source checkpoint is NOT_RELEASE pending exact rebuilt Windows binary, CORVEXON, native/CRT and complete QA acceptance.

<!-- DOC_PRESERVE_BEGIN:HF22_V5_GOLDEN_PROTOCOLS -->
# Canonical Golden Protocols

The canonical shipping sequence is:

`MSVC > URSYNEX > ARYRA > MUSTAVEX > CORVEXON > FALCION`

The protocol names are exactly:

- **URSYNEX Golden Finalizer** — sole bounded mutating post-link authority.
- **ARYRA Golden Layout** — read-only PE structural authority.
- **MUSTAVEX Golden Metadata** — read-only allowlisted shipping-metadata authority.
- **CORVEXON Golden Trail** — read-only final-binary acceptance authority.
- **FALCION Golden Cleaning** — release/package cleaning boundary.

## ARYRA boundary

ARYRA validates the structural AMD64/PE32+ six-section profile `.text .rdata .data .pdata .rsrc .reloc`, rejects raw section extents beyond EOF, and rejects overlapping loaded virtual section ranges. Exact RVA, VirtualSize/RawSize, raw offsets, SizeOfImage/SizeOfCode and exact entrypoint RVA remain non-parity targets.

## 3.1.9 CRT boundary

3.1.9 shipping uses `/MD + /GS` and the linker-selected `_DllMainCRTStartup`. CORVEXON no longer treats `DllMain @1` as the PE entrypoint. Instead it requires the entrypoint to be in executable `.text`, distinct from `DllMain @1`, while preserving the export/ordinal ABI.

The supported runtime family is VCRUNTIME140/VCRUNTIME140_1 plus UCRTBASE/API-MS-WIN-CRT when emitted. Legacy `MSVCR*` and `MSVCP*` families remain outside the approved shipping runtime surface.

CORVEXON also requires the final `/GS` image to expose a valid mapped Load Config SecurityCookie pointer whose owning section is writable. MUSTAVEX enforces both historical and current-release forbidden project-identity tokens across the final DLL bytes.

## Authority handoff

URSYNEX is the sole mutator and finalizes PE checksum after its approved mutation set. ARYRA and MUSTAVEX remain read-only structural/metadata authorities. CORVEXON performs the final read-only checksum recompute/compare together with final-binary ABI acceptance.
<!-- DOC_PRESERVE_END:HF22_V5_GOLDEN_PROTOCOLS -->
