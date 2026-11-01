from pathlib import Path
from datetime import datetime,timezone
import hashlib,json,re,subprocess
R=Path.cwd();O=R/'outputs';S=O/'source-workspace';B=S/'build'
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def write(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
a=read(S/'build-record.json');assert a['result']=='PASS' and a['native_objects']==296 and a['cpp_objects']==292 and a['c_objects']==4 and a['mock_test_suites']==356
assert len(a['results'])==356 and all(t['result']=='PASS' for t in a['results'])
assert len(a['terminal_boundary_cases'])==19 and all(t['result']=='PASS' and t['actual_exit']==t['expected_exit'] for t in a['terminal_boundary_cases'])
script=(S/'build.ps1').read_text(encoding='utf-8-sig');unit_block=re.search(r'\$units=\[ordered\]@\{(.*?)\n\}',script,re.S)[1]
units=dict(re.findall(r"(?m)^\s*(\w+)='([^']+)'",unit_block));assert len(units)==296
tests=dict(re.findall(r"@\{name='([^']+)';file='([^']+)'",script));assert len(tests)==355
compiler=R/'work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64/bin/clang++.exe';cc=compiler.with_name('clang.exe');ar=compiler.with_name('llvm-ar.exe')
assert sha(compiler)==a['compiler_sha256'] and sha(cc)==a['c_compiler_sha256'] and sha(ar)==a['archiver_sha256']
compile_commands={};objects=[]
for c in a['commands']:
 args=c['arguments']
 if '-c' not in args or '-o' not in args:continue
 out=Path(args[args.index('-o')+1]).resolve()
 if out.parent!=B.resolve() or out.suffix!='.o':continue
 name=out.stem;assert name in units and name not in compile_commands
 src=(O/units[name]).resolve();assert str(src) in [str(Path(s).resolve()) for s in args if s.endswith(('.cpp','.c'))]
 assert all(f in args for f in ['-O2','-Wall','-Wextra','-Werror'])
 compile_commands[name]=c;objects.append({'path':out.relative_to(R).as_posix(),'source':units[name],'bytes':out.stat().st_size,'sha256':sha(out),'mtime_utc':datetime.fromtimestamp(out.stat().st_mtime,timezone.utc).isoformat()})
assert set(compile_commands)==set(units) and len(list(B.glob('*.o')))==296
executables=[]
for t in a['results']:
 if t['module']=='hook_engine_fixtures':continue
 assert t['module'] in tests
 p=B/(t['module']+'_mock_tests.exe');assert sha(p)==t['executable_sha256']
 commands=[c for c in a['commands'] if 'expected_exit' not in c and Path(c['program']).resolve()==p.resolve()]
 assert len(commands)==1
 executables.append({'path':p.relative_to(R).as_posix(),'sha256':sha(p),'bytes':p.stat().st_size,'module':t['module']})
assert len(executables)==355
archives=[];compat={'hook_api','minhook_hooks','minhook_buffer','minhook_trampoline','minhook_hde'};preloader={'preloader','preloader_hook_adapter','preloader_native'}
for row in a['archives']:
 p=B/row['file'];assert sha(p)==row['sha256'] and p.stat().st_size==row['bytes']
 proc=subprocess.run([str(ar),'t',str(p)],capture_output=True,text=True,check=True);assert not proc.stderr
 members=proc.stdout.splitlines();expected=compat if row['file']=='hook_engine_compat.a' else preloader if row['file']=='preloader_core_partial.a' else set(units)-compat-preloader
 assert len(members)==len(set(members))==len(expected) and set(members)=={n+'.o' for n in expected}
 archives.append({**row,'members':members})
fixture=read(O/'hook-engine-source/fixture-validation.json');assert fixture['result']=='PASS'
fixture_path=Path(fixture['executable']['path']) if 'path' in fixture['executable'] else O/'hook-engine-source/testing/build/hook-engine-fixture-tests.exe'
if not fixture_path.is_absolute():fixture_path=R/fixture_path
assert sha(fixture_path)==fixture['executable']['sha256']
own_dll=B/'owned_reference_fixture.dll';assert own_dll.is_file()
frozen=read(O/'delivery/current-sources.json');assert frozen['current_source_count']==1017
for row in frozen['sources']:assert sha(O/row['path'])==row['sha256'],row['path']
old_snapshot=R/'work/full-build-history/B54-source-snapshot.json';old_record=R/'work/full-build-history/B54-build-record.json';assert sha(old_record)==read(old_snapshot)['build_record_sha256']
history=S/'history/B54';history.mkdir(parents=True,exist_ok=True)
for src,name in [(old_snapshot,'source-snapshot.json'),(old_record,'build-record.json')]:
 target=history/name
 if target.exists():assert sha(target)==sha(src)
 else:target.write_bytes(src.read_bytes())
snapshot={'recorded_at':datetime.now(timezone.utc).isoformat(),'scope':'Current 1017 source files after fresh clean full common build, 296 objects /356 groups /19 framework terminal cases','build_record_sha256':sha(S/'build-record.json'),'historical_B54_snapshot':'history/B54/source-snapshot.json','sources':[{'path':r['path'],'sha256':r['sha256']} for r in frozen['sources']]}
temp=S/'source-snapshot.current.tmp';write(temp,snapshot);temp.replace(S/'source-snapshot.json')
result={'schema':'fresh-full-common-build.v1','result':'PASS','checked_at':datetime.now(timezone.utc).isoformat(),'clean_build':'Previous canonical build moved to work/full-build-history/B54-build before invoking unchanged builder; new build directory created by builder','build_record':{'path':'outputs/source-workspace/build-record.json','sha256':sha(S/'build-record.json')},'source_snapshot':{'path':'outputs/source-workspace/source-snapshot.json','sha256':sha(S/'source-snapshot.json'),'sources':1017},'native_objects':296,'cpp_objects':292,'c_objects':4,'normal_groups':356,'normal_executables':355,'framework_terminal_cases':19,'object_compile_commands_verified':296,'objects':objects,'executables':executables,'archives':archives,'hook_fixture':{'record_path':'outputs/hook-engine-source/fixture-validation.json','record_sha256':sha(O/'hook-engine-source/fixture-validation.json'),'path':fixture_path.relative_to(R).as_posix(),'sha256':sha(fixture_path)},'own_synthetic_DLL':{'path':own_dll.relative_to(R).as_posix(),'sha256':sha(own_dll)},'compiler_sha256':a['compiler_sha256'],'c_compiler_sha256':a['c_compiler_sha256'],'archiver_sha256':a['archiver_sha256'],'all_current_source_hashes_verified':True,'command_status_evidence':'Original builder reaches PASS only after every normal command exits0 and all terminal cases match; normal per-command exit fields were not separately recorded by that script','original_input_access':False,'original_target_execution':False,'original_modification':False,'compiled_production_DLL':False}
write(S/'full-build-verification.json',result)
print(json.dumps({k:result[k] for k in ['result','native_objects','normal_groups','framework_terminal_cases','all_current_source_hashes_verified']},indent=2))
