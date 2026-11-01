## HF13 Asio IOCP MSWSOCK import acceptance (2026-10-14)

The shipping MSVC `/MT` Asio IOCP build links the WinSock extension library
`mswsock.lib`, producing a direct `MSWSOCK.DLL` import for exactly `AcceptEx`
and `GetAcceptExSockaddrs`. These functions serve asynchronous accepted-socket
operations in the vendored Asio Windows IOCP implementation.

CORVEXON therefore permits `MSWSOCK.DLL` as an **optional module**, but if it
is present it requires exactly those two named imports (no ordinals, additional
functions, or unexpected modules). `WS2_32.DLL`, `KERNEL32.DLL` and
`CRYPT32.DLL` remain mandatory. Static `/MT` CRT forbiddance, `/GS`, PE checksum,
export ABI, and final-binary SHA immutability remain unchanged.

This is a read-only final PE acceptance policy correction, **not** authorization
to change shipping binary bytes, bypass CORVEXON, or claim Windows IOCP runtime
or native/game acceptance. Native Acceptance scenario `one` exit 42 is a separate
unresolved boundary.

> **HF12 prospective supersession (2026-10-13):** Authorized current HF12 source uses static `/MT` shipping and `/MTd` debug CRT. Historical detailed records below remain immutable lineage. See `docs/CRT_STATIC_POLICY.md`. This is NOT_RELEASE until exact Windows acceptance.

# CORVEXON Golden Trail

## 3.2.0 operational note

The inherited CRT/final-binary acceptance contract remains pending for the exact SCIURONYX candidate. Historical detailed policy follows unchanged.

<!-- DOC_PRESERVE_BEGIN:HF22_V5_CORVEXON_GOLDEN_TRAIL -->
# CORVEXON Golden Trail

CORVEXON Golden Trail is the read-only final-binary acceptance authority after URSYNEX Golden Finalizer, ARYRA Golden Layout and MUSTAVEX Golden Metadata.

## 3.1.9 CRT shipping contract

3.1.9 shipping uses MSVC `/MD` + `/GS` and the default linker-selected `_DllMainCRTStartup` path.

CORVEXON therefore requires:

- AMD64 PE32+ GUI image.
- Exact named export ABI: `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- No forwarded shipping exports.
- `AddressOfEntryPoint` resides in `.text`, the owning section carries `IMAGE_SCN_MEM_EXECUTE`, and the RVA is **not** the exported `DllMain @1` RVA.
- Required Win32 import modules remain present.
- Runtime import expansion is limited to `VCRUNTIME140.DLL`, `VCRUNTIME140_1.DLL`, `UCRTBASE.DLL`, and `API-MS-WIN-CRT-*` when emitted by `/MD`; `MSVCP*` and legacy `MSVCR*` families remain forbidden.
- `/GS` Load Config contains a nonzero image-backed SecurityCookie pointer and the owning section carries `IMAGE_SCN_MEM_WRITE`.
- ARYRA, MUSTAVEX and MSVC shipping-link policy pass first.
- The verifier does not mutate final DLL bytes; SHA-256 before/after must match.

The superseded 3.1.7 custom `/ENTRY:DllMain` comparison image is intentionally outside canonical CORVEXON acceptance.

## Checksum acceptance

CORVEXON recomputes the PE checksum without mutating the image and requires equality with the URSYNEX-finalized `IMAGE_OPTIONAL_HEADER.CheckSum`. This closes the finalizer/acceptance integrity handoff while preserving CORVEXON's read-only role.
<!-- DOC_PRESERVE_END:HF22_V5_CORVEXON_GOLDEN_TRAIL -->
