#!/usr/bin/env python3
import pathlib,sys,tempfile,shutil
ROOT=pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'scripts/qa'))
sys.path.insert(0,str(ROOT/'scripts/build'))
from check_source_provenance import verify
from build_native_acceptance import produce
from test_native_evidence import CRT,REQUIRED
r=verify(ROOT)
assert r['parent_gf13_sha256'].startswith('e673')
assert set(CRT)=={'Release','Debug'} and len(REQUIRED)==2
with tempfile.TemporaryDirectory() as d:
 try:produce(ROOT,pathlib.Path(d)/'out.zip')
 except ValueError as e:assert 'REAL_WINDOWS_X64_NATIVE_TOOLS_REQUIRED' in str(e)
 else:raise AssertionError('FALSE_LINUX_NATIVE_PASS')
print('GF14_SOURCE_AND_NATIVE_ENV_GUARD_PASS')
