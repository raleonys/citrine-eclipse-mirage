#!/usr/bin/env python3
"""Validator must fail closed; synthetic text is never native acceptance."""
import json,pathlib,tempfile,sys,zipfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'scripts/qa'))
from test_native_evidence import validate_junit,validate_receipt_evidence,verify_zip,REQUIRED
neg=0
for data in (b'<testsuite><testcase name="gf_windows_export_shape"/></testsuite>',
             b'<testsuite><testcase name="gf_windows_export_shape"/><testcase name="gf_windows_export_shape"/></testsuite>',
             b'<testsuite><testcase name="gf_windows_export_shape"/><testcase name="gf13_original_dispatch_real_windows"><failure/></testcase></testsuite>',
             b'<testsuite><testcase name="gf_windows_export_shape"/><testcase name="gf13_original_dispatch_real_windows"><skipped/></testcase></testsuite>',
             b'not XML',b'<testsuite tests="2" failures="1"><testcase name="gf_windows_export_shape"/><testcase name="gf13_original_dispatch_real_windows"/></testsuite>'):
 try:validate_junit(data)
 except Exception:neg+=1
 else:raise AssertionError('SYNTHETIC_JUNIT_WAS_ACCEPTED')
assert validate_junit(b'<testsuite tests="2"><testcase name="gf_windows_export_shape"/><testcase name="gf13_original_dispatch_real_windows"/></testsuite>')
for summary in ({},
 {'schema':'greenfield.gf14.native-receipts.v1','classification':'LOCAL_TEST_NOT_RELEASE'},
 {'schema':'greenfield.gf14.native-receipts.v1','classification':'LOCAL_TEST_NOT_RELEASE','result':'BOTH_RELEASE_DEBUG_ACTUALLY_EXECUTED','gf13_parent_sha256':'incorrect'},
 {'schema':'greenfield.gf14.native-receipts.v1','classification':'LOCAL_TEST_NOT_RELEASE','result':'BOTH_RELEASE_DEBUG_ACTUALLY_EXECUTED','gf13_parent_sha256':'e67305952326ef7ecd4db77cf503f9f38cc0cd381d92362a0dadcfff8380e17d','consumer_game':'PASS','live_lsx':'NOT_RUN'}):
 try:validate_receipt_evidence({'SUMMARY.json'},lambda n:json.dumps(summary).encode())
 except Exception:neg+=1
 else:raise AssertionError('FORGED_SUMMARY_ACCEPTED')
with tempfile.TemporaryDirectory() as d:
 p=pathlib.Path(d)/'bad.zip'
 for entries in ({'SUMMARY.json':b'not-json'},
                 {'../SUMMARY.json':b'{}'},
                 {'SUMMARY.json':b'{}','Release/fake.txt':b'injection'}):
  with zipfile.ZipFile(p,'w') as z:
   for k,v in entries.items():z.writestr(k,v)
  try:verify_zip(p)
  except Exception:neg+=1
  else:raise AssertionError('BAD_ZIP_ACCEPTED')
print('GF14_NATIVE_EVIDENCE_NEGATIVES_PASS',neg,'/13')
