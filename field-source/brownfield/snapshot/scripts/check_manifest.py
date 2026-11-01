#!/usr/bin/env python3
from pathlib import Path
import json,hashlib,sys
r=Path(__file__).resolve().parents[1]
m=json.loads((r/'metadata/ACTIVE_SOURCE_MANIFEST.json').read_text())
errors=[]
for rel,expected in m['files'].items():
    p=r/rel
    if not p.is_file(): errors.append('missing '+rel); continue
    got=hashlib.sha256(p.read_bytes()).hexdigest()
    if got!=expected: errors.append('hash '+rel)
actual={p.relative_to(r).as_posix() for p in r.rglob('*') if p.is_file() and p.relative_to(r).as_posix()!='metadata/ACTIVE_SOURCE_MANIFEST.json' and not p.relative_to(r).as_posix().startswith(('dist/','build/'))}
expected=set(m['files'])
if actual!=expected:
    errors.append(f'file-set delta missing={len(expected-actual)} extra={len(actual-expected)}')
if errors:
    print('ACTIVE_MANIFEST: FAIL'); [print(' -',x) for x in errors[:50]]; sys.exit(1)
print(f'ACTIVE_MANIFEST: PASS ({len(expected)}/{len(expected)})')
