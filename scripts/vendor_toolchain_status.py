# SPDX-License-Identifier: GPL-3.0-only

"""Explicit CTest unavailability receipt; never reports an unsupported vendor as PASS."""
from pathlib import Path
import sys

cache=Path(sys.argv[1])
text=cache.read_text(encoding='utf-8')
if 'PRELOADER_L_PEGTL_PUBLIC_INPUT_WORKS:INTERNAL=1' in text:
    print('PEGTL toolchain probe is available; run vendor_pegtl_smoke')
    raise SystemExit(0)
print('UNAVAILABLE: supplied PEGTL public inputs do not compile with this exact toolchain.')
print('Vendor remains byte-exact. Compiler diagnostics: '+str(cache.parent/'CMakeFiles/CMakeConfigureLog.yaml'))
print('This is not production-use or runtime-test PASS.')
raise SystemExit(77)
