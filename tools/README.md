# Tools

- `preloader_l_source_host`: dynamically loads the development DLL and executes
  the calendar, SHA-1, and parser calls.
- `preloader_l_crt_host`: checks the compiler's standard CRT
  heap/unwind/container/thread/TLS behavior.
- `inspect_dll.py <dll>`: produces a read-only PE/import/export/resource report
  and verifies SHA equality of the original SURGE image embedded as RCDATA 101.
  Run with `python -B`.

Golden tools live under `scripts/`; build commands live under
`scripts/build_orchestration/`. Brownfield-specific
native/debug/session ABI hosts are not copied. These tools exercise
existing SURGE interfaces; they do not establish complete runtime
or game acceptance.
