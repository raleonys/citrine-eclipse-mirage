#!/usr/bin/env python3
"""Review actual GF14 runner evidence; does not assert it ran on this computer."""
import hashlib,json,pathlib,xml.etree.ElementTree as ET,zipfile,sys
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[1]))
from gf10_pe_export_gate import check as check_pe, PEError
from check_source_provenance import ROOT
REQUIRED={'gf_windows_export_shape','gf13_original_dispatch_real_windows'}
CRT={'Release':b'RuntimeLibrary=MT_StaticRelease','Debug':b'RuntimeLibrary=MTd_StaticDebug'}
GF13_SHA='e67305952326ef7ecd4db77cf503f9f38cc0cd381d92362a0dadcfff8380e17d'

def sha(b):return hashlib.sha256(b).hexdigest()
def safe_name(x):
 p=pathlib.PurePosixPath(x)
 return bool(x) and not p.is_absolute() and '..' not in p.parts and p.as_posix()==x and '\\' not in x

def validate_junit(data):
 try:r=ET.fromstring(data)
 except ET.ParseError as e:raise ValueError('JUNIT_BAD_XML') from e
 cases=r.findall('.//testcase')
 if len(cases)!=2 or {c.get('name') for c in cases}!=REQUIRED:
  raise ValueError('JUNIT_CASE_MISMATCH')
 if any(c.find(t) is not None for c in cases for t in ('failure','error','skipped')):raise ValueError('JUNIT_FAILURE')
 for e in r.iter():
  if e.tag in ('testsuite','testsuites'):
   for field in ('failures','errors','skipped'):
    if int(e.get(field,'0')):raise ValueError('JUNIT_SUITE_FAILURE')
 return True

def validate_receipt_evidence(names,read):
 if 'SUMMARY.json' not in names:raise ValueError('SUMMARY_MISSING')
 obj=json.loads(read('SUMMARY.json').decode('utf-8'))
 if obj.get('schema')!='greenfield.gf14.native-receipts.v1' or obj.get('classification')!='LOCAL_TEST_NOT_RELEASE':raise ValueError('BAD_SCHEMA')
 if obj.get('gf13_parent_sha256')!=GF13_SHA or obj.get('result')!='BOTH_RELEASE_DEBUG_ACTUALLY_EXECUTED':raise ValueError('FALSE_STATUS')
 if obj.get('consumer_game')!='NOT_RUN' or obj.get('live_lsx')!='NOT_RUN':raise ValueError('FORGED_RUNTIME_CLAIM')
 expected_manifest=sha((ROOT/'metadata/manifest.json').read_bytes())
 if obj.get('source_manifest_sha256')!=expected_manifest:raise ValueError('SOURCE_PROVENANCE_NOT_BOUND')
 receipts=obj.get('configurations')
 if not isinstance(receipts,dict) or set(receipts)!={'Release','Debug'}:raise ValueError('BOTH_CONFIGS_REQUIRED')
 observed=set(['SUMMARY.json'])
 for cfg,record in receipts.items():
  if record.get('configuration')!=cfg or record.get('test_cases')!=sorted(REQUIRED):raise ValueError('CONFIG_CASE_DRIFT')
  items=record.get('files')
  if not isinstance(items,dict):raise ValueError('FILES_MISSING')
  required={f'{cfg}/ctest.xml',f'{cfg}/ctest.log',f'{cfg}/cmake_build.log',f'{cfg}/cmake_configure.log',f'{cfg}/gf_preloader.dll',f'{cfg}/gf_export_host.exe',f'{cfg}/gf13_original_dispatch_native.exe',f'{cfg}/dll_entry.directives.txt',f'{cfg}/avatar_path.directives.txt',f'{cfg}/test_dispatch.directives.txt'}
  if set(items)!=required:raise ValueError('EVIDENCE_SET_DRIFT')
  for n,digest in items.items():
   if not safe_name(n) or n not in names or sha(read(n))!=digest:raise ValueError('EVIDENCE_HASH_DRIFT:'+n)
   if not read(n):raise ValueError('EMPTY_EVIDENCE:'+n)
   observed.add(n)
  # An evidence label cannot turn an arbitrary non-PE blob into a native DLL.
  check_pe(read(f'{cfg}/gf_preloader.dll'))
  for executable in ('gf_export_host.exe','gf13_original_dispatch_native.exe'):
   data=read(f'{cfg}/'+executable)
   if len(data)<64 or data[:2]!=b'MZ' or int.from_bytes(data[0x3c:0x40],'little')+6>len(data) or data[int.from_bytes(data[0x3c:0x40],'little'):int.from_bytes(data[0x3c:0x40],'little')+4]!=b'PE\0\0' or data[int.from_bytes(data[0x3c:0x40],'little')+4:int.from_bytes(data[0x3c:0x40],'little')+6]!=b'\x64\x86':
    raise ValueError('EXECUTABLE_NOT_REAL_AMD64_PE:'+executable)
  validate_junit(read(f'{cfg}/ctest.xml'))
  log=read(f'{cfg}/ctest.log')
  if b'GF00_WINDOWS_EXPORT_SHAPE_PASS_NOT_RUNTIME_PARITY' not in log:raise ValueError('LOADLIBRARY_HOST_PASS_MISSING')
  if b'100% tests passed' not in log:raise ValueError('CTEST_COMPLETE_PASS_MISSING')
  for n in ('dll_entry','avatar_path','test_dispatch'):
   if CRT[cfg] not in read(f'{cfg}/{n}.directives.txt'):raise ValueError('CRT_MISMATCH:'+cfg+':'+n)
 return observed

def verify_zip(path):
 with zipfile.ZipFile(path) as z:
  if z.testzip():raise ValueError('ZIP_BAD_CRC')
  names=[i.filename for i in z.infolist()]
  if len(set(names))!=len(names) or any(not safe_name(n) for n in names):raise ValueError('ZIP_PATH_OR_DUPLICATE')
  observed=validate_receipt_evidence(set(names),z.read)
  if observed!=set(names):raise ValueError('UNMANIFESTED_EVIDENCE')
  return len(observed)

if __name__=='__main__':
 try:print('GF14_NATIVE_EVIDENCE_VERIFIED',verify_zip(pathlib.Path(sys.argv[1])))
 except Exception as e:print('GF14_NATIVE_EVIDENCE_REJECTED',str(e));sys.exit(2)
