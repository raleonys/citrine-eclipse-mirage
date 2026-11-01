# URSYNEX Golden Finalizer

## 3.2.0 current-status note

URSYNEX authority is inherited unchanged. Detailed predecessor policy follows unchanged.

<!-- DOC_PRESERVE_BEGIN:HF22_V5_URSYNEX -->
# URSYNEX Golden Finalizer

Status: **sole bounded mutating Golden authority**  
Lineage: **2.9.3 URSYNEX provenance-preserving derived authority**

URSYNEX Golden Finalizer is the only Golden step permitted to rewrite the linked shipping DLL. It runs after the MSVC shipping link and before every read-only Golden validator.

## Authority

URSYNEX may apply only the post-link mutations enumerated by `metadata/URSYNEX_GOLDEN_FINALIZER.json` and constrained by the approved metadata policy. The current bounded mutation surface is:

- #10 LinkerVersion -> 14.0;
- #11 TimeDateStamp -> `0xCD268EB1`;
- #36 Rich Header -> absent.

It must fail closed if the DLL violates required PE preconditions, including an unexpected Debug Directory.

## Non-authority

URSYNEX does not recover or target non-allowlisted PE metadata, Authenticode/certificate data, or exact PE addresses/sizes/layout. It does not replace ARYRA, MUSTAVEX, or CORVEXON. Those authorities remain read-only.

Canonical implementation: `scripts/ursynex_golden_finalizer.py`.

## Integrity finalization

URSYNEX reads structured mutation values from its JSON authority, verifies they remain bound to MUSTAVEX's declared LinkerVersion/TimeDateStamp/RichHeader contract, and recomputes `IMAGE_OPTIONAL_HEADER.CheckSum` after every approved mutation. Checksum recomputation is integrity finalization, not a new recovered metadata target. CORVEXON must independently verify it read-only.
<!-- DOC_PRESERVE_END:HF22_V5_URSYNEX -->
