# SPDX-License-Identifier: GPL-3.0-only
"""Build and execute the actual application core and real TCP fixtures."""
import argparse,concurrent.futures,hashlib,json,os,shutil,subprocess,sys
from pathlib import Path
from _project import ROOT,sha,save,source_digest
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--build-root',type=Path);ap.add_argument('--sanitize',action='store_true');ap.add_argument('--profile',choices=['debug','production','native','crt','diagnostic'],default='debug');ap.add_argument('--jobs',type=int,default=3);a=ap.parse_args()
 if os.name=='nt':print('Use the MSVC Native/CRT launchers on Windows.');return 2
 compiler=shutil.which('g++')
 if not compiler:print('g++ is required.');return 2
 profile=('sanitizer' if a.profile=='debug' else 'sanitizer_'+a.profile) if a.sanitize else a.profile;out=(a.build_root.resolve() if a.build_root else ROOT/'build/portable')/profile;out.mkdir(parents=True,exist_ok=True)
 lock=out/'.build_lock'
 try:lock.mkdir()
 except FileExistsError:print('This profile is already building; inspect .build_lock before retrying.');return 2
 records=[];results=[];start_source=source_digest()
 try:
  graph=json.loads((ROOT/'metadata/BUILD_GRAPH.json').read_text());objects=out/'objects';binaries=out/'bin';logs=out/'logs'
  for p in [objects,binaries,logs]:p.mkdir(exist_ok=True)
  flags=['-std=c++20','-O0','-g','-pthread','-fPIC','-I'+str(ROOT),'-isystem',str(ROOT/'third_party/asio/include'),'-isystem',str(ROOT/'third_party/pegtl/include'),'-isystem',str(ROOT/'third_party/filesystem/include'),'-DASIO_STANDALONE=1','-DASIO_NO_DEPRECATED=1']
  flags+=['-DPRELOADER_L_DISK_DIAGNOSTICS='+('1' if a.profile in ['debug','diagnostic'] else '0')]
  if a.sanitize:flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
  headers=hashlib.sha256(''.join(p.relative_to(ROOT).as_posix()+sha(p) for p in sorted(ROOT.rglob('*')) if p.is_file() and p.suffix in ['.h','.hpp','.ipp','.inl'] and not {'build','dist'}&set(p.parts)).encode()).hexdigest()
  def execute(command,log):
   p=subprocess.run(list(map(str,command)),cwd=ROOT,capture_output=True,text=True)
   log.write_text((p.stdout+p.stderr).replace(str(ROOT),'${PROJECT_ROOT}'))
   if p.returncode:raise RuntimeError(log.read_text()[-4000:])
  def compile_one(name):
   source=ROOT/name;obj=objects/(name.replace('/','_')+'.o');receipt=obj.with_suffix('.json')
   key=hashlib.sha256((sha(source)+headers+json.dumps(flags)+sha(Path(compiler))).encode()).hexdigest()
   if obj.is_file() and receipt.is_file():
    previous=json.loads(receipt.read_text())
    if previous.get('key')==key and previous.get('sha256')==sha(obj) and obj.stat().st_size:return obj
   temp=obj.with_suffix('.pending')
   if temp.exists():temp.unlink()
   execute([compiler,*flags,'-c',source,'-o',temp],logs/(obj.stem+'.log'))
   if not temp.is_file() or temp.read_bytes()[:4]!=b'\x7fELF':raise RuntimeError('invalid object: '+name)
   os.replace(temp,obj);save(receipt,{'key':key,'sha256':sha(obj),'source':name});return obj
  with concurrent.futures.ThreadPoolExecutor(max_workers=max(1,min(8,a.jobs))) as pool:built=list(pool.map(compile_one,graph['core']))
  # All dependent links happen after the complete object set has finished.
  tests=graph['unit_tests']+graph['integration_tests']
  def link_one(name):
   source=ROOT/name;target=binaries/(source.parent.name+'_'+source.stem);temp=target.with_suffix('.pending')
   extra=[ROOT/'tests/support/configuration/order.cpp'] if source.stem=='test_hash_table' else []
   extra += [ROOT/n for n in graph.get('test_support_sources',{}).get(name,[])]
   if temp.exists():temp.unlink()
   execute([compiler,*flags,source,*extra,*built,'-o',temp],logs/(target.name+'_link.log'))
   if temp.read_bytes()[:4]!=b'\x7fELF':raise RuntimeError('invalid executable: '+target.name)
   temp.chmod(0o755);os.replace(temp,target);return target
  with concurrent.futures.ThreadPoolExecutor(max_workers=max(1,min(4,a.jobs))) as pool:executables=list(pool.map(link_one,tests))
  for target in executables:
   before=sha(target)
   p=subprocess.run([str(target)],cwd=ROOT,capture_output=True,text=True,timeout=60)
   row={'test':target.name,'exit_code':p.returncode,'sha256':before,'output':(p.stdout+p.stderr).replace(str(ROOT),'${PROJECT_ROOT}')}
   results.append(row);print(target.name, 'PASS' if not p.returncode else 'FAIL',flush=True)
   if p.returncode:raise RuntimeError(row['output'][-4000:])
  for name in graph.get('tool_tests',[]):
   target=ROOT/name;p=subprocess.run([sys.executable,str(target)],cwd=ROOT,capture_output=True,text=True,timeout=60)
   row={'test':name,'exit_code':p.returncode,'sha256':sha(target),'output':p.stdout+p.stderr};results.append(row)
   print(name,'PASS' if not p.returncode else 'FAIL',flush=True)
   if p.returncode:raise RuntimeError(row['output'][-4000:])
  if source_digest()!=start_source:raise RuntimeError('source changed during build/test; no fresh acceptance issued')
  save(out/'RESULT.json',{'status':'PASS','scope':'portable application core + unit contracts + real loopback/persistence; not Windows DLL acceptance','source_digest':start_source,'compiler_sha256':sha(Path(compiler)),'compiler_flags':[v.replace(str(ROOT),'${PROJECT_ROOT}') for v in flags],'profile':a.profile,'disk_diagnostics':a.profile in ['debug','diagnostic'],'asan_options':os.environ.get('ASAN_OPTIONS','default'),'tests':results})
  print('PASS portable application; suites='+str(len(results)));return 0
 except Exception as e:
  save(out/'RESULT.json',{'status':'FAIL','source_digest':start_source,'error':str(e),'tests':results});print(str(e),file=sys.stderr);return 1
 finally:lock.rmdir()
if __name__=='__main__':sys.exit(main())
