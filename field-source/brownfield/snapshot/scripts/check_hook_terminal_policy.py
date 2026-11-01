#!/usr/bin/env python3
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]
legacy=[
 'include/preloader_l/platform/x64_hook_allocator_model.hpp',
 'src/platform/inline_hook_backend_windows.cpp',
 'src/platform/inline_hook_allocator_windows.cpp',
]
errors=[]
for rel in legacy:
    if (r/rel).exists(): errors.append('retired custom terminal-policy prerequisite present: '+rel)
p=r/'src/platform/minhook_runtime.cpp'
if not p.is_file(): errors.append('MinHook runtime missing')
else:
    s=p.read_text()
    required={
      'failed-enable conservative ownership':'tickets[i]->enabled = 1u; // conservative ownership state',
      'failed-enable active publication':'publish_gate_active(gates[i]);',
      'disable failure retains ownership':'conservatively retain ownership for a retry',
      'enabled trampoline cannot be removed':'if (ticket->enabled) return false;',
      'runtime cannot uninitialize with owned hooks':'if (atomic_load(&g_created_hooks) != 0) return false;',
      'successful disable drains admitted calls':'wait_gate(gates[i]);',
    }
    for name,needle in required.items():
        if needle not in s: errors.append(name)
if errors:
    print('D2D4_TERMINAL_POLICY: FAIL')
    for e in errors: print(' -',e)
    sys.exit(1)
print('D2D4_TERMINAL_POLICY: PASS (legacy metadata-exhaustion terminal path retired; MinHook rollback/ownership retention fail-closed)')
