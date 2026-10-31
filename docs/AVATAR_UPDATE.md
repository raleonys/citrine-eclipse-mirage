# Current profile avatar

The owner explicitly chose to retain the existing embedding approach. The
latest supplied PNG is 208 x 208 pixels and 3,840 bytes, with SHA-256
`df06a4efdf98a12f430753ac8d2629e10445e926725bc0620026c7e299636e0e`.
`resources/branding/surge.png` is a byte-identical copy of that file.

The existing generator converts the complete file into a private, read-only
C++ array. It neither redraws pixels, re-encodes PNG data, nor adds a PNG PE
resource. Normal startup writes the profile PNG to the local user-data
directory. The CFG User/Avatar selection continues to skip this profile write.
The original 3,882-byte fallback and the separate `default.png` write remain
unchanged. An external avatar file is not required.

Application, test, generator, vendor, and export source code did not change.
Only the image input and its current records were updated. Previous TLS/cleanup
and allocation-injection test corrections were retained. This change neither
explains nor resolves the VirusTotal Files Dropped difference.

[Validation](../metadata/evidence/avatar-update/VALIDATION.json) records local
cross-build/Wine results. Those local checks do not prove native Windows/MSVC,
Golden, or game acceptance of the binary containing the new image. Builds use
the source package; this work did not update or publish a distribution artifact.

Local results: the relevant Release tests passed 4/4 and Debug tests passed
2/2. The actual DLL was loaded and started three times, returned an LSX profile
response, shut down, and unloaded. The profile PNG written to disk was
byte-identical to the new input; the fallback was byte-identical to the
original input. The raw DLL contained the new PNG, did not contain the old
profile PNG, and had no PNG PE resource. The Release application test did not
create a log in a clean data directory. The pre-existing global Debug log's
timestamp remained earlier than the DLL execution.

Source QA passed 17/17 and vendor regressions passed 19/19; no vendor test was
skipped.
