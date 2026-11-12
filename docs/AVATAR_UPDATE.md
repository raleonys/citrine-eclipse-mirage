# Current profile avatar

The owner-provided `surge.zip` contains `surge.png`: a 208 x 208 RGB PNG
of exactly 4,478 bytes, with SHA-256
`5cc62afa98d4e97ac2f780cc7e2d7edd4ae538ece62d81d0da3b61858a0a08a6`.
`resources/branding/surge.png` is byte-identical to the supplied PNG; the
image has not been redrawn or re-encoded. The supplied ZIP has SHA-256
`9644e3b5a19447673b66fdfa58fff4bc9d3ad0cff89a5048eab73d34a2b792bf`.

The existing `scripts/generate_avatar_data.py` generator validates the PNG
against `metadata/AVATAR_DATA_CONTRACT.json` and converts its exact bytes
into private, read-only C++ data during the build. No PNG PE resource or
external runtime avatar file is introduced. Normal startup writes that
embedded profile PNG into the local user-data directory, except when the
configured User/Avatar selection skips the profile write.

The separate original 3,882-byte `resources/branding/default-avatar.png`
fallback, its C++ embedding and the default-avatar write are unchanged.
The existing project VERSIONINFO policy, Windows resource script, DLL exports,
runtime C++ source and vendor inputs are unchanged by this avatar update. This change updates only the profile
image, its current metadata/hash pins and the documentation.

The prior avatar validation files under
`metadata/evidence/avatar-update/` refer to an earlier image and are
historical evidence, not new acceptance of this replacement. The build must
regenerate its private avatar header from the new pinned PNG. Source QA,
portable tests and native Windows/MSVC plus game acceptance must be assessed
for this exact PR head before any release claim. No binary or release output
is committed or published by this image update.
