# SPDX-License-Identifier: GPL-3.0-only

"""Explicit CTest unavailability receipt; never reports an unsupported vendor as PASS."""
from pathlib import Path
import sys

cache=Path(sys.argv[1])
text=cache.read_text(encoding='utf-8')
key='CITRINE_ECLIPSE_MIRAGE_PEGTL_PUBLIC_INPUT_WORKS'
probe=key+':INTERNAL='
# Count the logical key even when its type or separator is malformed: a
# second record must not disappear merely because it is not INTERNAL.
records=[line for line in text.splitlines()
         if line == key or line.startswith((key+':', key+'='))]
if len(records)!=1:
    print('INVALID: PEGTL toolchain probe cache record is missing or ambiguous.')
    raise SystemExit(1)
if not records[0].startswith(probe):
    print('INVALID: PEGTL toolchain probe cache record type or syntax is malformed.')
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
