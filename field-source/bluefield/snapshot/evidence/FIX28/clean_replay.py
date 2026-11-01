import hashlib,json,shlex,subprocess,sys,time,zipfile
from pathlib import Path
from run_profile import W,S,H,Q,C,T,run,preflight
archive=S/'dist/preloader_l_BF02_FIX28_APPLICATION_SOURCE_NOT_RELEASE.zip';digest=hashlib.sha256(archive.read_bytes()).hexdigest()
src=Q/'clean_source/preloader_l';build=Q/'clean_build';assert not src.exists() and not build.exists()
with zipfile.ZipFile(archive) as z:z.extractall(src.parent)
sys.path.insert(0,str(src/'scripts/qa'));from _project import source_digest
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest(src)==frozen
result={'status':'PENDING','archive_sha256':digest,'source_digest':frozen,'fresh_source':str(src.relative_to(W)),'fresh_objects':str(build.relative_to(W)),'profile':'PRODUCTION','build_type':'Release','generated_artifact_recovery':[]}
result['configure']=run([C,'-S',src,'-B',build,'-DCMAKE_BUILD_TYPE=Release','-DPRELOADER_L_PROFILE=PRODUCTION','-DBUILD_TESTING=ON'],H/'clean_configure.log',src)
start=time.monotonic()
with (H/'clean_build.log').open('w') as f:r=subprocess.run([str(C),'--build',str(build),'--parallel','4'],stdout=f,stderr=subprocess.STDOUT)
result['initial_build']={'exit_code':r.returncode,'seconds':round(time.monotonic()-start,3)}
if r.returncode:
 # Only unmistakably malformed generated objects may be rebuilt. Any ordinary
 # compiler/linker failure without such an artifact remains a hard failure.
 bad=[p for p in build.rglob('*.o') if p.read_bytes()[:4]!=b'\x7fELF'];assert bad,'build failed without malformed generated object'
 for p in bad:
  rel=p.relative_to(build).as_posix();target=rel.split('/')[1];cmd=['/usr/bin/gmake','-B','-f',f'CMakeFiles/{target}/build.make',rel]
  before=p.stat().st_size;run(cmd,H/f'clean_{target}_forced_object.log',build);assert p.read_bytes()[:4]==b'\x7fELF'
  result['generated_artifact_recovery'].append({'object':rel,'previous_bytes':before,'command':cmd,'recovered_bytes':p.stat().st_size})
 result['recovered_build']=run([C,'--build',build,'--parallel','4'],H/'clean_build_retry.log',src)
inventory=json.loads(subprocess.check_output([str(T),'--test-dir',str(build),'--show-only=json-v1']))
for t in inventory['tests']:
 p=Path(t['command'][0])
 if not p.is_relative_to(build) or p.read_bytes()[:4]==b'\x7fELF':continue
 before=p.stat().st_size;cmd=shlex.split((build/'CMakeFiles'/f'{p.name}.dir/link.txt').read_text())
 run(cmd,H/f'clean_{p.name}_relink.log',build);assert p.read_bytes()[:4]==b'\x7fELF'
 result['generated_artifact_recovery'].append({'executable':p.name,'previous_bytes':before,'command':cmd,'recovered_bytes':p.stat().st_size})
preflight(build,'clean')
(H/'CLEAN_BUILD.json').write_text(json.dumps(result,indent=2)+'\n')
print('PASS exact ZIP fresh-source/fresh-object configure/build/preflight; CTest pending',flush=True)
