#!/usr/bin/env python3
from pathlib import Path
import re, sys

r = Path(__file__).resolve().parents[1]
errors = []

for rel in (
    "src/runtime/fatal_termination.cpp",
    "src/runtime/heap_allocator.cpp",
    "src/runtime/ptd_fls.cpp",
):
    t = (r / rel).read_text(encoding="utf-8", errors="replace")
    for macro in ("WIN32_LEAN_AND_MEAN", "NOMINMAX"):
        guarded = re.search(
            rf"#ifndef\s+{macro}\s*\n#define\s+{macro}\s*\n#endif",
            t,
            re.MULTILINE,
        )
        if not guarded:
            errors.append(f"{rel}: {macro} is not guarded")

if errors:
    print("WINDOWS_MACRO_HYGIENE: FAIL")
    for e in errors:
        print(" -", e)
    sys.exit(1)

print("WINDOWS_MACRO_HYGIENE: PASS (3 files / 6 guarded definitions)")
