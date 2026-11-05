# Per-frame job allocation versus selected 64-entry ceilings

Fresh REA 6.8.0 / Ghidra 12.1.4 analysis of immutable anadius64.dll, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Seven complete query replies, session identity, open/close and exported bundle are retained here. Static evidence only; no target execution.

Observed instruction sites in 025480: closure size0x30 at `0x1800254ed`, allocator04b0bc at `0x1800254f2`, entry021770 selected at `0x18002554b`, launch05e8e0 at `0x180025556`, detach04a458 at `0x18002557f`, read restart0280d0 at `0x180025607`. This normal path allocates a closure for each completed frame and resumes reading after successful detach. There is no 64-entry job registry admission check in this examined body. Pseudocode Evidence `ev_c3f8d4f1f13de77237826f1090dad5208a88c12f1298934787e39cd45ffd6c6a`; instructions `ev_223374f43ffb4f8d5717d7130157603b9c1ac45a1e36d912f32f107e91d89848`.

Observed 05e8e0 prepares a per-thread parameter block via05e87c at `0x18005e91d` and calls the CreateThread import at `0x18005e94c`. Native allocation/OS failures remain possible and are handled in that body; this does not prove unlimited OS threads. Pseudocode `ev_da89cb81274b6720c4590a9ecdc118a1ba360f98df67a4277c31067d3ee83b09`; instructions `ev_4ce98b79108cf3f40e3dbf8e1c26b83fef80d05cc6dc6d65ac7b44ff33df84d6`.

Observed021770 invokes the stored function at `0x1800217aa`, performs04a47c cleanup at `0x1800217ac`, and releases the0x30 closure through04b0f8 at `0x180021802`. It does not release a slot in a native fixed64 row array. Pseudocode `ev_4e07be3971e944c0b4b45203a5ef8c2ce26e9cc9360c485270be3ba9f15b82ee`; instructions `ev_e4c5e990266c3487273d9dd5e30a53a7064bd3d4adf8b73b44b9e519e7b61647`.

Supporting0280d0 pseudocode `ev_a79d227d6dc5492ad73eec196d5b9dc94b7717d482b9cb570a1db99f7839e64f` shows the weak-owner acquisition and composed-read starter. It is not the closure allocator;025480 is the relevant allocation body.

Source inference: the selected Profile.jobs/closures ceiling, NativeJobs physical-row ceiling and PreparedMemory.maximum_job_borrows64 rejection are reconstruction constraints on this per-frame path. All three must be addressed coherently. The native comparison does not establish arbitrary concurrent allocator/new-handler safety, source EH ABI equivalence or safe live unload; those are not added to this correction.
