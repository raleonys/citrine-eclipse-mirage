#!/usr/bin/env python3
from pathlib import Path
import subprocess, sys
root = Path(__file__).resolve().parents[1]
script = root/'scripts'/'test_regressions.py'
for shard in range(8):
    print(f'=== MINHOOK BEHAVIORAL SHARD {shard + 1}/8 ===', flush=True)
    result = subprocess.run([
        sys.executable, str(script), '--shard-count', '8', '--shard-index', str(shard)
    ], cwd=root)
    if result.returncode:
        print(f'MINHOOK_BEHAVIORAL_QA: FAIL ({shard + 1}/8)')
        sys.exit(result.returncode)
print('MINHOOK_BEHAVIORAL_QA: PASS (8/8)')
