import hashlib,json,os,stat,subprocess,sys,time
from pathlib import Path
W=Path(__file__).resolve().parents[1]
S=W/'fix30_work/preloader_l';H=W/'fix30_helpers';Q=W/'fix30_qa'
C=W/'fix28_runtime/cmake/data/bin/cmake';T=C.with_name('ctest')
def run(argv,log,cwd=S):
 start=time.monotonic()
 with log.open('w') as f:r=subprocess.run(list(map(str,argv)),cwd=cwd,stdout=f,stderr=subprocess.STDOUT)
 assert r.returncode==0,f'{log}: exit {r.returncode}'
 return {'exit_code':r.returncode,'seconds':round(time.monotonic()-start,3),'log':str(log.relative_to(W))}
def preflight(build,label):
 raw=subprocess.check_output([str(T),'--test-dir',str(build),'--show-only=json-v1'])
 (H/f'{label}_CTEST_INVENTORY.json').write_bytes(raw)
 tests=json.loads(raw)['tests'];assert len(tests)==51
 entries=[]
 for t in tests:
  p=Path(t['command'][0]).resolve()
  if p.read_bytes()[:4]!=b'\x7fELF' or not p.is_relative_to(build.resolve()):continue
  before=hashlib.sha256(p.read_bytes()).hexdigest();mode=stat.S_IMODE(p.stat().st_mode)
  if not mode&0o111:p.chmod(mode|0o111)
  assert hashlib.sha256(p.read_bytes()).hexdigest()==before
  entries.append({'test':t['name'],'file':str(p.relative_to(build)),'sha256':before,'before_mode':oct(mode),'after_mode':oct(stat.S_IMODE(p.stat().st_mode))})
 assert len(entries)==48
 receipt={'status':'PASS','tests':51,'owned_ELF':48,'scope':'execute-bit repair only on generated executables; byte hashes unchanged','entries':entries}
 (H/f'{label}_EXEC_PREFLIGHT.json').write_text(json.dumps(receipt,indent=2)+'\n')
 return receipt
if __name__=='__main__':
 profile=sys.argv[1];build=Q/profile;build.mkdir(parents=True,exist_ok=True)
 sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest
 frozen=source_digest();assert frozen==json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest']
 config='Debug' if profile=='debug' else 'Release'
 result={'profile':profile,'source_digest':frozen}
 result['configure']=run([C,'-S',S,'-B',build,f'-DCMAKE_BUILD_TYPE={config}',f'-DPRELOADER_L_PROFILE={profile.upper()}','-DBUILD_TESTING=ON','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'],H/f'{profile}_configure.log')
 result['build']=run([C,'--build',build,'--parallel','3'],H/f'{profile}_build.log')
 preflight(build,profile)
 (H/f'{profile}_BUILD.json').write_text(json.dumps(result,indent=2)+'\n')
 print('PASS configure/build/preflight',profile,flush=True)
