# Tools

- `laureondo_source_host`: dynamically loads the development DLL and executes
  the calendar, SHA-1, and parser calls.
- `laureondo_crt_host`: checks the compiler's standard CRT
  heap/unwind/container/thread/TLS behavior.
- `inspect_dll.py <dll>`: produces a read-only PE/import/export/resource report
  and verifies the exact profile/fallback avatar bytes in read-only C++ data.
  VERSIONINFO is the only permitted PE resource; avatars are not RCDATA.
  Run with `python -B`; `--final` additionally checks finalized PE policy.

Golden tools live under `scripts/`; build commands live under
`scripts/build_orchestration/`. Brownfield-specific
native/debug/session ABI hosts are not copied. These tools exercise
existing SURGE interfaces; they do not establish complete runtime
or game acceptance.
