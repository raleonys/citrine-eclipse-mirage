#!/usr/bin/env python3
"""GF14: genuine in-process MSVC+CTest evidence capture, never a synthetic PASS.
Runs only under real Windows x64 VS Native Tools. Non-release, isolated build.
"""
import hashlib,json,os,pathlib,queue,re,shutil,subprocess,sys,tempfile,threading,time,zipfile
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[1]/"qa"))
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[1]))
from check_source_provenance import verify as verify_source, GF13_SHA
from test_native_evidence import verify_zip,CRT,REQUIRED,validate_junit
from gf10_pe_export_gate import check as check_pe

def sha(data):return hashlib.sha256(data).hexdigest()

def run_stage(cmd,log,timeout=1200):
 """Stream real combined tool output to console and exact stage log; bounded."""
 print('[GF14 EXEC]',subprocess.list2cmdline([str(x) for x in cmd]),flush=True)
 q=queue.Queue()
 with open(log,'wb') as out:
  proc=subprocess.Popen([str(x) for x in cmd],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,bufsize=0)
  def pump():
   try:
    while True:
     line=proc.stdout.readline()
     if not line:break
     q.put(line)
   finally:q.put(None)
  threading.Thread(target=pump,daemon=True).start()
  done=False;start=time.monotonic()
  while not done:
   if time.monotonic()-start>timeout:
    try:
     if os.name=='nt':subprocess.run(['taskkill','/F','/T','/PID',str(proc.pid)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10)
     else:proc.kill()
    except Exception:proc.kill()
    raise ValueError('NATIVE_STAGE_TIMEOUT:'+str(log))
   try:x=q.get(timeout=.25)
   except queue.Empty:continue
   if x is None:done=True
   else:
    out.write(x);out.flush()
    print(x.decode('utf-8',errors='replace'),end='',flush=True)
  rc=proc.wait(timeout=10)
  if rc:raise ValueError('NATIVE_STAGE_FAILED:'+str(log)+':'+str(rc))
  return log.read_bytes()

def only(root,name):
 matches=[p for p in root.rglob(name) if p.is_file() and p.stat().st_size>0]
 if len(matches)!=1:raise ValueError('MISSING_OR_AMBIGUOUS_NATIVE_ARTIFACT:'+name+':'+str(len(matches)))
 return matches[0]

def produce(root,out):
 if os.name!='nt' or os.environ.get('VSCMD_ARG_TGT_ARCH','').lower()!='x64':
  raise ValueError('REAL_WINDOWS_X64_NATIVE_TOOLS_REQUIRED')
 # No change to the GF13 inherited source bytes or shipping DLL.
 prov=verify_source(root)
 for tool in ('cl','cmake','ctest','dumpbin'):
  if shutil.which(tool) is None:raise ValueError('MSVC_TOOL_MISSING:'+tool)
 if out.exists():raise ValueError('EXISTING_EVIDENCE_ZIP_WILL_NOT_OVERWRITE')
 with tempfile.TemporaryDirectory(prefix='GF14_REAL_WINDOWS_') as workdir:
  work=pathlib.Path(workdir)
  archives={}
  records={}
  for cfg in ('Release','Debug'):
   print('[GF14]',cfg,'real MSVC x64 build + live CTest',flush=True)
   b=work/cfg/'build';b.mkdir(parents=True)
   evidence=work/cfg/'results';evidence.mkdir(parents=True)
   run_stage(['cmake','-S',root,'-B',b,'-G','NMake Makefiles',f'-DCMAKE_BUILD_TYPE={cfg}',
              '-DGF13_ENABLE_ORIGINAL_DISPATCH_NATIVE_LINK=ON'],evidence/'cmake_configure.log')
   run_stage(['cmake','--build',b,'--target','gf_preloader','gf_export_host','gf13_original_dispatch_native','--verbose'],
             evidence/'cmake_build.log',timeout=2100)
   junit=b/'gf14_ctest.xml'
   log=evidence/'ctest.log'
   output=run_stage(['ctest','--test-dir',b,'-V','--output-on-failure','--output-junit',junit,
              '-R','^(gf_windows_export_shape|gf13_original_dispatch_real_windows)$'],log,timeout=600)
   if b'GF00_WINDOWS_EXPORT_SHAPE_PASS_NOT_RUNTIME_PARITY' not in output:
    raise ValueError('REAL_EXPORT_HOST_OUTPUT_NOT_SEEN')
   if b'100% tests passed' not in output:raise ValueError('CTEST_FINISH_NOT_SEEN')
   if not junit.is_file():raise ValueError('LIVE_CTEST_JUNIT_NOT_EMITTED')
   validate_junit(junit.read_bytes())
   shutil.copyfile(junit,evidence/'ctest.xml')
   for name in ('gf_preloader.dll','gf_export_host.exe','gf13_original_dispatch_native.exe'):
    original='preloader_l.dll' if name=='gf_preloader.dll' else name
    shutil.copyfile(only(b,original),evidence/name)
   check_pe((evidence/'gf_preloader.dll').read_bytes())
   for src,short in (('dll_entry.cpp.obj','dll_entry'),('avatar_path.cpp.obj','avatar_path'),('test_dispatch.cpp.obj','test_dispatch')):
    obj=only(b,src)
    raw=run_stage(['dumpbin','/directives',obj],evidence/(short+'.directives.txt'),timeout=120)
    if CRT[cfg] not in raw:raise ValueError('MSVC_CRT_DIRECTIVES_MISMATCH:'+cfg+':'+short)
   files={f'{cfg}/{p.name}':sha(p.read_bytes()) for p in evidence.iterdir() if p.is_file()}
   records[cfg]={'configuration':cfg,'test_cases':sorted(REQUIRED),'files':files}
  summary={
   'schema':'greenfield.gf14.native-receipts.v1',
   'classification':'LOCAL_TEST_NOT_RELEASE','result':'BOTH_RELEASE_DEBUG_ACTUALLY_EXECUTED',
   'gf13_parent_sha256':GF13_SHA,'source_manifest_sha256':prov['source_manifest_sha256'],
   'consumer_game':'NOT_RUN','live_lsx':'NOT_RUN',
   'runtime_scope':'ORIGINAL_DISPATCH_UNIT_TEST_AND_DLL_EXPORT_ONLY',
   'configurations':records
  }
  payload={'SUMMARY.json':(json.dumps(summary,sort_keys=True,indent=2)+'\n').encode('utf-8')}
  for cfg in ('Release','Debug'):
   for path in (work/cfg/'results').iterdir():payload[f'{cfg}/{path.name}']=path.read_bytes()
  out.parent.mkdir(parents=True,exist_ok=True)
  tmp=out.with_suffix(out.suffix+'.partial')
  with zipfile.ZipFile(tmp,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
   for name in sorted(payload):
    info=zipfile.ZipInfo(name,(2026,10,20,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED
    z.writestr(info,payload[name],compress_type=zipfile.ZIP_DEFLATED,compresslevel=6)
  if verify_zip(tmp)!=21:
   raise ValueError('NATIVE_BUNDLE_SELF_VERIFICATION_FAIL')
  tmp.replace(out)
  print('GF14_NATIVE_BUNDLE_GENERATED',out,sha(out.read_bytes()),flush=True)
  return out

if __name__=='__main__':
 try:
  if len(sys.argv)!=2:raise ValueError('USAGE: gf14_windows_runner.py EXACT_SOURCE_ROOT')
  root=pathlib.Path(sys.argv[1]).resolve()
  out=root.parent/(root.name+'_GF14_WINDOWS_RESULTS_FULL_EVIDENCE.zip')
  produce(root,out)
 except Exception as e:
  print('[GF14 BLOCKED]',type(e).__name__,str(e),flush=True)
  sys.exit(2)
