# SPDX-License-Identifier: GPL-3.0-only

"""Explicit CTest unavailability receipt; never reports an unsupported vendor as PASS."""
from pathlib import Path
import sys

cache=Path(sys.argv[1])
text=cache.read_text(encoding='utf-8')
probe='CITRINE_ECLIPSE_MIRAGE_PEGTL_PUBLIC_INPUT_WORKS:INTERNAL='
records=[line for line in text.splitlines() if line.startswith(probe)]
if len(records)!=1:
    print('INVALID: PEGTL toolchain probe cache record is missing or ambiguous.')
    raise SystemExit(1)
value=records[0][len(probe):]
if value not in ('', '0', '1'):
    print('INVALID: PEGTL toolchain probe cache value is malformed.')
    raise SystemExit(1)
if value == '1':
    print('PEGTL toolchain probe is available; run vendor_pegtl_smoke')
    raise SystemExit(0)
print('UNAVAILABLE: supplied PEGTL public inputs do not compile with this exact toolchain.')
print('Vendor remains byte-exact. Compiler diagnostics: '+str(cache.parent/'CMakeFiles/CMakeConfigureLog.yaml'))
print('This is not production-use or runtime-test PASS.')
raise SystemExit(77)
