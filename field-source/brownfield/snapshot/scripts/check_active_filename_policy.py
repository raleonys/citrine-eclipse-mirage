#!/usr/bin/env python3
"""Fail-closed active-path vendor-name prohibition for project-owned source."""
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]
errors=[]
for p in r.rglob('*'):
    if not p.is_file():continue
    rel=p.relative_to(r).as_posix()
    if rel.startswith(('third_party/','build/','dist/')) or '__pycache__' in rel:continue
    if 'anadius' in rel.lower():errors.append('forbidden donor-branded active filename: '+rel)
active=('CMakeLists.txt','src','include','tests','scripts')
donor='ana'+'dius'
retired=(donor+'_extension_routes','test_'+donor+'_extension_routes',
         'test_'+donor+'_qp_qce_joint','test_'+donor+'_qp_qce_parity',
         donor.upper()+'_EXTENSION_LINEAGE')
for root in active:
    path=r/root
    paths=[path] if path.is_file() else [p for p in path.rglob('*') if p.is_file()]
    for p in paths:
        if '__pycache__' in p.as_posix():continue
        try: s=p.read_text('utf-8')
        except (OSError,UnicodeError):continue
        if any(x in s for x in retired):errors.append('stale operational path: '+str(p.relative_to(r)))
if errors:
    print('ACTIVE_FILENAME_POLICY: FAIL');[print(' -',e) for e in errors[:25]];sys.exit(1)
print('ACTIVE_FILENAME_POLICY: PASS (no vendor-branded active path or retired operational reference)')
