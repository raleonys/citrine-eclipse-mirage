# Token-file loader source

`token.cpp` reconstructs `anadius64.dll` VA `0x18002afa0` as application-level C++20. It uses the recovered AES implementation directly. Its file status, filesystem exception, and whole-file reader remain mandatory interfaces in `token.hpp`; no missing operation returns an invented success.

A concrete whole-file reader and license composition adapter are now available in `../anadius-file-source`. They were exercised with actual IO/CBC on generated synthetic fixtures only. Native status/path/CRT/ABI equivalence remains outside that result.

The sequence is status with flags `3`, binary read, AES-128 CBC with static `.rdata` key and zero IV, decryption beginning at byte `65`, then an explicit-length byte search for `<GameToken>` and `</GameToken>`. This is **not XML parsing**. The first exact opening sequence at or after byte `65` wins; the first closing sequence after it ends the value. Entities, embedded NUL, nested tags, and arbitrary byte text are preserved literally. Candidate start positions include the final position where each entire tag still fits.

The decryption result is deliberately ignored, as at `0x18002b068`. A nonaligned encrypted suffix is searched unchanged. Invalid padding leaves decrypted bytes in place and does not prevent searching. The 65-byte prefix is outside CBC processing, although the original padding check can still inspect/remove its trailing bytes when no ciphertext blocks exist.

For input shorter than 65 bytes, the original unsigned alignment check rejects most lengths safely, producing an empty result. Lengths **1, 17, 33, and 49** instead enter CBC with a wrapped enormous length and leave the supported memory domain. This reconstruction throws `crypto_recovered::OutOfEvidenceInput` for those four lengths. It does not claim to reproduce undefined memory behavior. Length zero is safely rejected by the alignment check; it does not take the separate cipher wrapper's generic empty-input error.

An error from status throws unless `((uint32_t(kind)-1)&0xfffffff7)==0`, corresponding to kinds `1` and `9`. Reading happens only for kind `2`. The separate status routine's original `permissions` field is unused here. Its mapping and the whole-file reader's exact MSVC stream/OS behavior are not implemented in this folder. Exceptions from those interfaces propagate.

`constants.hpp` contains static original binary constants, not a user credential. `evidence.json` records the input SHA-256, raw ranges, constant hashes, retained REA Evidence IDs, and relevant complete assembly. No original DLL/EXE was executed or loaded. No actual user token, ticket, or configuration was opened by this module or its checks.

Run `build.ps1` to compile `token.o` and execute the synthetic test executable. The checks cover concrete CBC, ignored false returns, decryption mutation before padding rejection, short-input domains, error order, exact tag bounds, raw entities, and NUL preservation. They establish consistency with these recovered contracts; they do not establish full DLL runtime equivalence or original MSVC ABI compatibility.
