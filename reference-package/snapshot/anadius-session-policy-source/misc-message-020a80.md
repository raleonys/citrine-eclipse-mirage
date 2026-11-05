# asio.misc message slot020a80 — 2026-11-03

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

An offline refresh examined307 canonical entries in 0x18001ff00..0x180038fff
against577 current production source/header files. Before this change156 entries
had tagged/explicit source presence,9 only composition forwards and142 no exact
mapping. These are heuristic presence classes, not implemented/native-equivalent
function counts. Old inventory and frozen build evidence were preserved.

The highest ranked candidates mostly belong to MSVC exceptions, string/tree
helpers or already supplied category boundaries. Reviewing the asio.misc getter
0285b0 led to a concrete discrepancy in message slot020a80. The current compiler
category formatted every code as `asio.misc error <number>`. The retained native
body has four exact branches and one shared fallback:

| int32 code | Native / reconstructed value |
| --- | --- |
| 1 | Already open |
| 2 | End of file |
| 3 | Element not found |
| 4 | The descriptor does not fit into the select call's fd_set |
| All other values | asio.misc error |

`misc_message_020a80` now implements this value/control-flow body. The existing
MiscErrorCategory::message delegates to it. Generic/system compiler categories,
category identity and the explicit format_override option remain as before.
Unknown codes no longer acquire a numeric suffix. Existing native finish paths
still suppress actual misc EOF2; successful write0 is not forced through a fake
error-handler continuation in the new integration scenario.

Evidence is retained, not newly observed: pseudocode
ev_f52c22ab6d0482e58f7ca56d36cbfec5a1c0913b03e9f41689c3a3276162843d and assembly
ev_604360fb740f3377313684b0bfdc1e461a65ae4fedb7995928d16dbc8f4448c0.
Assembly comparisons at020a91/020ac9/020b0d/020b58 select1..4; fallback starts020bc9.
Native lengths are12/11/17/57 and15 for fallback. Native3/4 allocate32/64 bytes;
this std::string value reconstruction does not reproduce MSVC SSO/capacity,
out-parameter calling convention, allocation failure or exception ABI.

Complete function slices for020a70(name),020a80(message),0285b0(getter) and their
parent listing hashes are in ../dll-critical-path/current-application-map/
selected-native/manifest.json. The first eight candidate pseudocode bodies were
also retained. No original binary, REA target or compiler was opened/executed.

After the implementation, a separately retained source map reports157 source-
present entries,9 composition-only and141 unmapped in the same range. A per-function
review override recognizes this new leaf within the otherwise composition-only
Policy file. That increase is one source body, not a percentage or native ABI closure.
The150/149 review-candidate counts also include composition-only entries and
exclude thunks; they are not counts of missing application behavior.

New unexecuted test sources cover all four values, default0/5/-1/INT32_MIN/MAX,
and actual default-category write-finish string/log ownership for nonzero errors,
including EOF suppression. None was compiled/run. Three source/header files
changed;55 files remain pending versus the frozen full build. Build/link/test
receipts and ZIP are unchanged. The next bounded candidate is028240's typed
category/context propagation; do not turn every exception/STL helper into a new
application function or resume generic CRT automatically.
