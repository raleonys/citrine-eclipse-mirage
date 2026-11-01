#!/usr/bin/env python3
"""Receipt for *actual* Windows operator-run GF13 build after CTest success.
Not a test runner, only validates on-disk outputs and emits NOT_RELEASE evidence.
"""
import sys,pathlib,json,hashlib,datetime,xml.etree.ElementTree as ET
from gf10_pe_export_gate import check,PEError

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def record(build,cfg):
 if cfg not in ('Release','Debug'):raise ValueError('CONFIG')
 if not build.is_dir():raise ValueError('NO_BUILD_DIR')
 binaries={}
 for key,n in [('dll','preloader_l.dll'),('export_host','gf_export_host.exe'),('dispatch','gf13_original_dispatch_native.exe')]:
  items=list(build.rglob(n))
  if len(items)!=1 or not items[0].is_file() or items[0].stat().st_size==0:raise ValueError('NATIVE_BIN_MISSING_OR_AMBIGUOUS:'+key)
  binaries[key]={'sha256':sha(items[0]),'size':items[0].stat().st_size,'path':str(items[0])}
 check(next(build.rglob('preloader_l.dll')).read_bytes())
 junit=build/f'GF13_TESTS_{cfg}.xml'
 if not junit.is_file() or not junit.stat().st_size:raise ValueError('JUNIT_MISSING')
 try:rt=ET.fromstring(junit.read_bytes())
 except ET.ParseError as e:raise ValueError('JUNIT_INVALID') from e
 cases=rt.findall('.//testcase')
 names=[e.get('name') for e in cases]
 required={'gf_windows_export_shape','gf13_original_dispatch_real_windows'}
 if len(cases)!=2 or set(names)!=required:raise ValueError('JUNIT_TEST_SET_DRIFT')
 if any(e.find('failure') is not None or e.find('error') is not None or e.find('skipped') is not None for e in cases):raise ValueError('JUNIT_TEST_FAILURE')
 if any(int(e.get('failures','0'))>0 or int(e.get('errors','0'))>0 for e in rt.iter() if e.tag in ('testsuite','testsuites')):raise ValueError('JUNIT_SUITE_FAILURE')
 receipt={
   'schema':'greenfield.gf13.local-windows-receipt.v1','classification':'LOCAL_TEST_NOT_RELEASE',
   'configuration':cfg,'binaries':binaries,
   'runner':'RUN_WINDOWS_GF13.cmd',
   'ctest_junit_sha256':sha(junit),'ctest_confirmed_cases':sorted(required),
   'test_evidence_scope':'RUNNER_EXECUTED_NATIVE_CTEST_SUCCESS_PRECONDITION',
   'source_sha256':'VERIFY_IN_SOURCE_ZIP_SIDECAR',
   'shipping_surge':'NOOP','shipping_dllshutdown':'ZERO_OWNER',
   'actual_iocp_lsx_game':'NOT_RUN',
   'recorded_at_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()
 }
 dst=build/f'GF13_WINDOWS_{cfg}_LOCAL_RECEIPT.json'
 tmp=dst.with_suffix('.partial')
 tmp.write_text(json.dumps(receipt,sort_keys=True,indent=2)+'\n')
 tmp.replace(dst)
 print('GF13_LOCAL_WINDOWS_RECEIPT_CREATED',dst)
 return 0
if __name__=='__main__':
 try:
  if len(sys.argv)!=3:raise ValueError('USAGE')
  sys.exit(record(pathlib.Path(sys.argv[1]).resolve(),sys.argv[2]))
 except (ValueError,PEError,IOError) as e:
  print('GF13_NATIVE_RECEIPT_BLOCKED',str(e));sys.exit(2)
