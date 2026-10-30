# Reference metadata, profile avatar and native acceptance fixes

The owner supplied a new metadata-reference DLL and a ZIP containing the new
SURGE avatar. The source adopts the selected metadata fields and preserves the
PNG bytes exactly. Delivery labels remain external to the source package.

| Field | New reference value |
| --- | --- |
| PE compilation timestamp | `0xA6FD7C8F` |
| `dwFileVersionMS` / `dwFileVersionLS` | `0x4F9CE840` / `0x8D58F00B` |
| `dwProductVersionMS` / `dwProductVersionLS` | `0xA49B7911` / `0x01A15B5D` |
| Visible ProductVersion | `3.9.5` (unchanged in the reference) |
| Linker / OS / subsystem versions | `14.0` / `6.0` / `6.0` |
| Avatar | 208 by 208 PNG, 4,181 bytes |

Reference DLL SHA-256:
`23c01053608af63359975cfba833764206a8eda410f111525ba1784d693e32d4`.
New avatar SHA-256:
`7ef474ef2d4f52bc6859a5db2d1fc164891eed5793330aa73f57e386056fa639`.

`resources/preloader_version.rc` and its source hash, the resource contract,
MUSTAVEX requirements and URSYNEX timestamp mutation agree on the new reference.
The historical synthetic parser policies keep their historical values. The
product-finalizer regression uses the active product timestamp. Original supplied
DLL bytes stay outside the product source; the extraction is read-only.

The existing three exports/ordinals, import order, compiler CRT, debug/signature
exclusions and structural section policy are unchanged. The supplied DLL's code,
certificate, debug records and manifest are not part of the selected metadata
adoption. Fixed resource metadata remains independent of the delivery identity.

The new PNG replaces only the embedded profile image, through the existing
generated read-only C++ array. The separate fallback avatar is unchanged. PNG is
not added to PE resources. Startup continues to write the default image when no
custom Avatar path is configured; configured user avatars retain their existing
policy.

## Reported native failures

The supplied Windows logs show successful Compile, Debug, Static CRT and
Production profiles. Static CRT ran six tests successfully. Native acceptance
passed 186 of 188 tests and reported these two failures:

* `infrastructure_ownership_packaging`: the namespace fixture wrote a text file
  with Windows CRLF conversion but hashed its LF input string. The fixture now
  writes the declared UTF-8 bytes directly. The production source verifier keeps
  its strict byte hashing. The original failure was reproduced and the corrected
  namespace fixture passed with Windows CPython under Wine.
* `listener_port_contracts`: the next service did not select the expected last
  free port. Windows `SO_REUSEADDR` permits endpoint sharing, unlike the intended
  single-owner bounded-range selection. The concrete Windows listener now sets
  `SO_EXCLUSIVEADDRUSE` before binding. The test also attempts a competing
  reuse-enabled listener. The pure RE server projection retains its original
  reuse argument. Range, retry/exhaustion and publication rules are unchanged.

Wine's socket behavior is distinct from native Windows; the simple reuse probe
already rejected sharing in Wine. A new native Windows run remains required to
confirm the reported Windows port regression is resolved on the owner's machine.

The owner also confirms Windows and game acceptance for the pre-update build.
That report and the successful Golden log are retained separately from acceptance
of the new metadata, avatar and exclusive-bind change. The combined metadata/native audit is retained in
[REFERENCE_METADATA_AVATAR.json](evidence/REFERENCE_METADATA_AVATAR.json). Latest
avatar verification is in [AVATAR_REFRESH.json](evidence/AVATAR_REFRESH.json).
