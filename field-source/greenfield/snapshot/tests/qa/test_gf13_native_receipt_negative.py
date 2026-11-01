#!/usr/bin/env python3
import pathlib,tempfile,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'scripts'))
from gf13_record_native import record
count=0
with tempfile.TemporaryDirectory() as t:
 d=pathlib.Path(t)
 for cfg in ('Release','Debug','Invalid'):
  try:record(d,cfg)
  except ValueError:count+=1
  else:raise RuntimeError('FALSE_RECEIPT_'+cfg)
 for x in ('preloader_l.dll','gf_export_host.exe','gf13_original_dispatch_native.exe'):(d/x).write_bytes(b'')
 try:record(d,'Release')
 except ValueError:count+=1
 else:raise RuntimeError('ZERO_SIZE_ACCEPT')
 (d/'GF13_TESTS_Release.xml').write_text('<testsuite tests="2"><testcase name="gf_windows_export_shape"/><testcase name="gf13_original_dispatch_real_windows"/></testsuite>')
 for x in ('preloader_l.dll','gf_export_host.exe','gf13_original_dispatch_native.exe'):(d/x).write_bytes(b'fake')
 try:record(d,'Release')
 except Exception:count+=1
 else:raise RuntimeError('FAKE_PE_ACCEPT')
print('GF13_NATIVE_RECEIPT_NEGATIVES_PASS',count,'/5')
