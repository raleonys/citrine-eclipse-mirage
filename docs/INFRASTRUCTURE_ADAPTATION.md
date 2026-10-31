# Brownfield infrastructure adaptation

Source: the owner-supplied HF20 full-source candidate ZIP.
The frozen Brownfield source was not modified. File-level input/output SHA
records are in [TOOLING_PROVENANCE.json](../metadata/evidence/TOOLING_PROVENANCE.json).

Selected components include the operator UX flow from five build helpers
and shared MSVC utilities; four actual PE Golden tools; the MSVC linker-policy
checker; and PE/restricted-import negative tests. Metadata ownership and
FALCION mechanics were adapted after removing Brownfield-specific application
and version dependencies.

Brownfield's Debug/runtime/session/socket/native/release hosts require its
own export ABI. Rather than copying them and pretending Yellowfield offers
exports that it does not have, the existing dynamic source host, a new
standard-CRT probe, and Yellowfield CTest targets are used. The presence of
these hosts is not evidence of completed native/game acceptance.

Old application route/state/Asio tests and historical release/Library/G0
records were not migrated. The rationale and inspected files are recorded
in the current transfer/adaptation records. The SURGE artwork is preserved
from the owner's original input; third-party Brownfield product metadata
was not added to the resource file.
