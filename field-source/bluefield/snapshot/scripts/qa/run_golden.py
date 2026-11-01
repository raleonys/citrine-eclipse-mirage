# SPDX-License-Identifier: GPL-3.0-only
"""Run all five Golden gates on a staged MSVC DLL; publish only after success."""
import argparse,hashlib,json,os,shutil,subprocess,sys,tempfile
from pathlib import Path
sys.dont_write_bytecode=True
from _project import ROOT,source_digest,save,sha

STAGES=('URSYNEX','ARYRA','MUSTAVEX','CORVEXON','FALCION')
class GoldenFailure(RuntimeError):pass
def confined(path,parent):
 p=Path(path).resolve();p.relative_to(Path(parent).resolve());return p
def run_pipeline(root,dll,build_dir,output,report,runner=None):
 root=Path(root).resolve();receipt={'classification':'LOCAL_BUILD','status':'FAIL','published':False,'stages':[],'game_acceptance':'NOT_RUN','official_release_acceptance':False}
 result=1
 try:
  for name in ('build','dist'):(root/name).resolve().relative_to(root)
  build_dir=confined(build_dir,root/'build');dll=confined(dll,build_dir)
  output=confined(output,root/'dist');report=confined(report,root/'build')
  if dll==output or not dll.is_file() or dll.suffix.lower()!='.dll':raise GoldenFailure('invalid MSVC input DLL')
  receipt.update(input_sha256=sha(dll),source_digest=source_digest(root),output=output.relative_to(root).as_posix())
  work_parent=build_dir/'golden_work';work_parent.mkdir(parents=True,exist_ok=True)
  scripts=root/'tools/golden/scripts'
  with tempfile.TemporaryDirectory(prefix='stage_',dir=work_parent) as temp:
   work=Path(temp);payload=work/'payload';payload.mkdir();staged=payload/'preloader_l.dll';shutil.copyfile(dll,staged)
   commands=[
    [sys.executable,str(scripts/'ursynex_golden_finalizer.py'),str(staged)],
    [sys.executable,str(scripts/'verify_aryra_golden_layout.py'),str(staged)],
    [sys.executable,str(scripts/'verify_mustavex_golden_metadata.py'),str(staged)],
    [sys.executable,str(scripts/'verify_corvexon_golden_trail.py'),str(staged),'--build-dir',str(build_dir)],
    [sys.executable,str(scripts/'check_falcion_golden_cleaning.py'),'--root',str(root),'--payload',str(payload)]
   ]
   policy_root=root/'tools/golden/metadata'
   receipt['policy_sha256']={p.name:sha(p) for p in sorted(policy_root.glob('*.json'))}
   receipt['policy_sha256']['FALCION_GOLDEN_CLEANING.json']=sha(root/'metadata/FALCION_GOLDEN_CLEANING.json')
   for name,command in zip(STAGES,commands):
    before=sha(staged)
    if runner is None:
     proc=subprocess.run(command,cwd=root,capture_output=True,text=True,timeout=120,env=dict(os.environ,PYTHONDONTWRITEBYTECODE='1'))
    else:proc=runner(name,command,staged)
    after=sha(staged) if staged.exists() else None
    text=(proc.stdout+proc.stderr).replace(str(work),'${STAGING_ROOT}').replace(str(root),'${PROJECT_ROOT}')
    row={'name':name,'exit_code':proc.returncode,'sha256_before':before,'sha256_after':after,'output':text,'status':'PASS' if proc.returncode==0 else 'FAIL'}
    if name!='URSYNEX' and before!=after:row['status']='FAIL';row['error']='read-only Golden gate changed DLL bytes'
    receipt['stages'].append(row)
    print(name+': '+row['status'],flush=True)
    if row['status']!='PASS':raise GoldenFailure(name+' failed: '+row.get('error',text[-2400:]))
   if source_digest(root)!=receipt['source_digest']:raise GoldenFailure('source changed during Golden execution')
   if sha(dll)!=receipt['input_sha256']:raise GoldenFailure('MSVC input changed during Golden execution')
   final=sha(staged);output.parent.mkdir(parents=True,exist_ok=True)
   fd,pending=tempfile.mkstemp(prefix=output.name+'.',suffix='.pending',dir=output.parent)
   try:
    with os.fdopen(fd,'wb') as stream:
     stream.write(staged.read_bytes());stream.flush();os.fsync(stream.fileno())
    if sha(Path(pending))!=final:raise GoldenFailure('staged publication hash mismatch')
    os.replace(pending,output)
   finally:
    if os.path.exists(pending):os.unlink(pending)
   receipt.update(status='PASS',published=True,output_sha256=final)
   result=0
 except (OSError,ValueError,GoldenFailure,subprocess.SubprocessError) as error:
  receipt['error']=str(error).replace(str(root),'${PROJECT_ROOT}')
 finally:
  # Report path is itself confined, including failures before the first gate.
  try:
   (root/'build').resolve().relative_to(root)
   save(confined(report,root/'build'),receipt)
  except (OSError,ValueError) as error:
   print('Golden report failure: '+str(error),file=sys.stderr);result=1
 return result

def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('--dll',type=Path,required=True);ap.add_argument('--build-dir',type=Path,required=True)
 ap.add_argument('--output',type=Path,required=True);ap.add_argument('--report',type=Path,required=True);args=ap.parse_args()
 return run_pipeline(ROOT,args.dll,args.build_dir,args.output,args.report)
if __name__=='__main__':sys.exit(main())
