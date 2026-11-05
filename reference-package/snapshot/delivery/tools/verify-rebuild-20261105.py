"""Verify the authorized 2026-11-05 clean run after its builder records PASS.

Does not compile or run tests. The only subprocess lists archive members with
llvm-ar t. Historical pending evidence, canonical source/dependency hashes and
documented same-run repairs are checked before publishing new PASS records.
"""
from pathlib import Path
from datetime import datetime, timezone
import hashlib,json,re,subprocess,os
R=Path.cwd();O=R/'outputs';S=O/'source-workspace';B=S/'build'
run=Path((R/'work/current-rebuild-path.txt').read_text(encoding='utf-8-sig').strip())
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
sha_cache={}
def sha(p):
 p=p.resolve();st=p.stat();key=(str(p).lower(),st.st_size,st.st_mtime_ns)
 if key not in sha_cache:
  with p.open('rb') as f:sha_cache[key]=hashlib.file_digest(f,'sha256').hexdigest()
 return sha_cache[key]
def write(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def output_path(value):
 p=Path(value)
 if not p.is_absolute():p=R/p if p.parts and p.parts[0].lower()=='outputs' else O/p
 p=p.resolve();p.relative_to(O.resolve());return p
def output_relative(value):return output_path(value).relative_to(O.resolve()).as_posix()
record=read(S/'build-record.json');assert record['result']=='PASS'
run_info=read(run/'run.json');assert run_info['user_authorized_rebuild'] is True
current_recipe=(S/'build.ps1').read_text(encoding='utf-8-sig').split('$units=[ordered]@{',1)[1]
initial_recipe=(run/'historical-common/build.ps1').read_text(encoding='utf-8-sig').split('$units=[ordered]@{',1)[1]
assert current_recipe==initial_recipe,'Compile/archive/test recipes changed during this run'
checkpoint=read(run/'compile-checkpoint.json')
assert checkpoint['schema']=='same-clean-run-compiler-checkpoint.v1' and Path(checkpoint['run_directory']).resolve()==run.resolve()
checkpoint_headers={str(Path(row['path']).resolve()).lower():row for row in checkpoint['headers']}
assert len(checkpoint_headers)==len(checkpoint['headers']),'Duplicate checkpoint header path'
current_header_paths={str(p.resolve()).lower() for directory in O.iterdir() if directory.is_dir() and directory.name.endswith('-source') for p in directory.rglob('*') if p.is_file() and p.suffix.lower() in {'.hpp','.h'}}
assert set(checkpoint_headers)==current_header_paths,'Recursive checkpoint header inventory changed'
for row in checkpoint['headers']:assert sha(Path(row['path']))==row['sha256']
checkpoint_rows={row['key']:row for row in checkpoint['entries']}
reused=[c for c in record['commands'] if c.get('execution')=='REUSED_VERIFIED_SAME_CLEAN_RUN']
for command in reused:
 assert Path(command['checkpoint_run']).resolve()==run.resolve()
 args=command['arguments'];target=Path(args[args.index('-o')+1]);assert sha(target)==command['output_sha256']
 assert str(target.resolve()).lower().startswith(str(B.resolve()).lower()+os.sep) and target.suffix in {'.o','.exe'}
 for row in command['input_hashes']:assert sha(Path(row['path']))==row['sha256']
 key=hashlib.sha256((command['program'].lower()+'\0'+'\0'.join(args)).encode('utf-8')).hexdigest()
 cached=checkpoint_rows[key]
 assert cached['output_sha256']==command['output_sha256'] and cached['compiler_sha256']==sha(Path(command['program']))
assert record['native_objects']==296 and record['cpp_objects']==292 and record['c_objects']==4
assert record['mock_test_suites']==356 and len(record['results'])==356 and all(r['result']=='PASS' for r in record['results'])
assert len(record['terminal_boundary_cases'])==19 and all(r['result']=='PASS' and r['actual_exit']==r['expected_exit'] for r in record['terminal_boundary_cases'])
script=(S/'build.ps1').read_text(encoding='utf-8-sig')
units=dict(re.findall(r"(?m)^\s*(\w+)='([^']+)'",re.search(r'\$units=\[ordered\]@\{(.*?)\n\}',script,re.S)[1]));assert len(units)==296
tests=dict(re.findall(r"@\{name='([^']+)';file='([^']+)'",script));assert len(tests)==355
toolchain=Path(os.environ['ANADIUS_TOOLCHAIN_ROOT']).resolve();compiler=toolchain/'bin/clang++.exe';cc=compiler.with_name('clang.exe');ar=compiler.with_name('llvm-ar.exe')
assert sha(compiler)==record['compiler_sha256'] and sha(cc)==record['c_compiler_sha256'] and sha(ar)==record['archiver_sha256']
objects=[];seen=set();started=datetime.fromisoformat(run_info['started_at'].replace('Z','+00:00')).timestamp()
for command in record['commands']:
 args=command['arguments']
 if '-c' not in args or '-o' not in args:continue
 target=Path(args[args.index('-o')+1]).resolve()
 if target.parent!=B.resolve() or target.suffix!='.o':continue
 name=target.stem;assert name in units and name not in seen;seen.add(name)
 source=(O/units[name]).resolve();assert str(source) in [str(Path(a).resolve()) for a in args if a.endswith(('.cpp','.c'))]
 assert sha(Path(command['program']))==record['c_compiler_sha256' if units[name].endswith('.c') else 'compiler_sha256']
 assert all(flag in args for flag in ['-O2','-Wall','-Wextra','-Werror']) and target.stat().st_mtime>=started
 objects.append({'path':target.relative_to(R).as_posix(),'source':units[name],'bytes':target.stat().st_size,'sha256':sha(target),'mtime_utc':datetime.fromtimestamp(target.stat().st_mtime,timezone.utc).isoformat()})
assert seen==set(units) and len(list(B.glob('*.o')))==296
executables=[]
for result in record['results']:
 if result['module']=='hook_engine_fixtures':continue
 assert result['module'] in tests
 target=B/(result['module']+'_mock_tests.exe');assert sha(target)==result['executable_sha256'] and target.stat().st_mtime>=started
 commands=[c for c in record['commands'] if 'expected_exit' not in c and Path(c['program']).resolve()==target.resolve()];assert len(commands)==1
 executables.append({'path':target.relative_to(R).as_posix(),'module':result['module'],'sha256':sha(target),'bytes':target.stat().st_size})
assert len(executables)==355
compat={'hook_api','minhook_hooks','minhook_buffer','minhook_trampoline','minhook_hde'};preloader={'preloader','preloader_hook_adapter','preloader_native'};archives=[]
for row in record['archives']:
 target=B/row['file'];assert sha(target)==row['sha256'] and target.stat().st_size==row['bytes']
 members=subprocess.run([str(ar),'t',str(target)],capture_output=True,text=True,check=True).stdout.splitlines()
 expected=compat if row['file']=='hook_engine_compat.a' else preloader if row['file']=='preloader_core_partial.a' else set(units)-compat-preloader
 assert len(members)==len(set(members))==len(expected) and set(members)=={name+'.o' for name in expected}
 archives.append({**row,'members':members})
fixture=read(O/'hook-engine-source/fixture-validation.json');assert fixture['result']=='PASS'
fixture_path=O/'hook-engine-source/testing/build/hook-engine-fixture-tests.exe';assert sha(fixture_path)==fixture['executable']['sha256'] and fixture_path.stat().st_mtime>=started
own_dll=B/'owned_reference_fixture.dll';assert own_dll.is_file() and own_dll.stat().st_mtime>=started
historical=run/'historical-common';old=read(historical/'source-snapshot.json');assert sha(historical/'build-record.json')==old['build_record_sha256']
history=S/'history'/run.name;assert history.resolve().parent==(S/'history').resolve()
source_paths=[output_relative(row['path']) for row in old['sources']]
assert len(source_paths)==len(set(source_paths))==1017
# Canonical dependency headers come from the delivered snapshot, not from the
# much broader recursive compiler-cache invalidation inventory (which includes
# frozen before/ snapshots and other uncompiled historical headers).
dependency_paths=[output_relative(row['path']) for row in old.get('additional_dependency_headers',[])]
assert len(dependency_paths)==len(set(dependency_paths)) and not set(source_paths)&set(dependency_paths)
expected={output_relative(row['path']):row['sha256'] for row in old['sources']+old.get('additional_dependency_headers',[])}
pending=read(historical/'pending-source-status.json')
pending_paths=pending['changed_sources_since_last_full_build']
assert all(isinstance(path,str) for path in pending_paths),'Current pending schema must be a list of paths'
pending_paths=[output_relative(path) for path in pending_paths]
assert len(pending_paths)==len(set(pending_paths))
evidence_ref=pending['source_evidence'];evidence_path=output_path(evidence_ref['path'])
assert sha(evidence_path)==evidence_ref['sha256'],'Historical pending source evidence changed'
evidence=read(evidence_path);pending_rows=evidence['pending_sources']
assert len(pending_rows)==evidence['pending_source_count']==len(pending_paths)
assert {output_relative(row['path']) for row in pending_rows}==set(pending_paths)
safe_path=O/'anadius-application-graph-source/scoped-runtime-integration-20261105/safe-pause-checkpoint.json'
safe=read(safe_path)
assert safe['source_evidence']==evidence_ref and safe['pending_sources']==pending_rows
assert safe['pending_source_count']==len(pending_rows)
safe_pending=next(row for row in safe['critical_records'] if output_relative(row['path'])=='dll-critical-path/pending-source-status.json')
assert sha(historical/'pending-source-status.json')==safe_pending['sha256']
new_pending={}
for row in pending_rows:
 path=output_relative(row['path'])
 if path in expected:
  assert not row.get('added_since_build',False),path
  assert expected[path]==row['build_sha256'],path
  expected[path]=row['current_sha256']
 else:
  assert row['build_sha256'] is None and row.get('added_since_build') is True,path
  assert path not in new_pending and re.fullmatch('[0-9a-f]{64}',row['current_sha256']),path
  new_pending[path]=row['current_sha256']
baseline_path=R/'work/authorized-rebuild-baseline-20261105.json'
baseline=read(baseline_path)
assert output_path(baseline['paused_source_checkpoint'])==safe_path.resolve()
assert baseline['pending_source_count']==len(pending_rows)
baseline_rows={output_relative(row['path']):row for row in baseline['current_sources']}
assert len(baseline_rows)==len(baseline['current_sources'])==1020
for path,value in expected.items():assert baseline_rows[path]['sha256']==value,path
added_dependency_paths=sorted(set(baseline_rows)-set(expected))
assert set(added_dependency_paths)=={'anadius-application-io-source/call.hpp','anadius-socket-source/memory_lookup.hpp'}
assert set(added_dependency_paths)==set(new_pending)
for path in added_dependency_paths:
 row=baseline_rows[path];assert row.get('added_since_prior_build') is True
 assert row['sha256']==new_pending[path],path
 expected[path]=row['sha256'];dependency_paths.append(path)
assert set(expected)==set(baseline_rows) and len(dependency_paths)==3
repairs=read(run/'source-repairs.json') if (run/'source-repairs.json').exists() else []
for repair in repairs:
 path=output_relative(repair['path']);assert repair.get('reason'),path
 assert expected[path]==repair['before_sha256'],path
 expected[path]=repair['after_sha256']
source_rows=[{'path':path,'sha256':sha(O/path)} for path in source_paths]
for row in source_rows:assert row['sha256']==expected[row['path']],row['path']
assert len(source_rows)==1017 and len({r['path'] for r in source_rows})==1017
additional_headers=[{'path':path,'sha256':sha(O/path)} for path in dependency_paths]
for row in additional_headers:assert row['sha256']==expected[row['path']],row['path']
canonical_paths=set(source_paths)|set(dependency_paths)
inventory_only_headers=[{'path':output_relative(row['path']),'sha256':row['sha256']} for row in checkpoint['headers'] if output_relative(row['path']) not in canonical_paths]
source_authority={'pending_status':{'path':(historical/'pending-source-status.json').relative_to(R).as_posix(),'sha256':sha(historical/'pending-source-status.json')},'source_evidence':{'path':evidence_path.relative_to(R).as_posix(),'sha256':sha(evidence_path)},'safe_checkpoint':{'path':safe_path.relative_to(R).as_posix(),'sha256':sha(safe_path)},'authorized_rebuild_baseline':{'path':baseline_path.relative_to(R).as_posix(),'sha256':sha(baseline_path),'total_files':len(baseline_rows),'new_dependency_headers':added_dependency_paths},'pending_sources':len(pending_rows),'repair_record':{'path':(run/'source-repairs.json').relative_to(R).as_posix(),'sha256':sha(run/'source-repairs.json')} if repairs else None,'compile_source_repairs':repairs}
# Publish only after all source, binary, command and outcome checks above pass.
history.mkdir(parents=True,exist_ok=True)
for name in ['source-snapshot.json','build-record.json','full-build-verification.json']:
 dest=history/name
 if dest.exists():assert sha(dest)==sha(historical/name)
 else:dest.write_bytes((historical/name).read_bytes())
snapshot={'recorded_at':datetime.now(timezone.utc).isoformat(),'scope':'Current source bytes verified with this fresh clean rebuild','build_record_sha256':sha(S/'build-record.json'),'previous_snapshot':(history/'source-snapshot.json').relative_to(S).as_posix(),'sources':source_rows}
snapshot['additional_dependency_headers']=additional_headers
snapshot['source_authority']=source_authority
write(S/'source-snapshot.json',snapshot)
result={'schema':'fresh-full-common-build.v2','result':'PASS','checked_at':datetime.now(timezone.utc).isoformat(),'clean_build':'Previous common build and hook fixture moved to the rebuild run history before fresh compilation; no prior objects/executables reused','run':run.relative_to(R).as_posix(),'build_record':{'path':'outputs/source-workspace/build-record.json','sha256':sha(S/'build-record.json')},'source_snapshot':{'path':'outputs/source-workspace/source-snapshot.json','sha256':sha(S/'source-snapshot.json'),'sources':len(source_rows)},'native_objects':296,'cpp_objects':292,'c_objects':4,'normal_groups':356,'normal_executables':355,'framework_terminal_cases':19,'object_compile_commands_verified':296,'objects':objects,'executables':executables,'archives':archives,'hook_fixture':{'record_path':'outputs/hook-engine-source/fixture-validation.json','record_sha256':sha(O/'hook-engine-source/fixture-validation.json'),'path':fixture_path.relative_to(R).as_posix(),'sha256':sha(fixture_path)},'own_synthetic_DLL':{'path':own_dll.relative_to(R).as_posix(),'sha256':sha(own_dll)},'compiler':record['compiler'],'compiler_path':str(compiler),'compiler_sha256':record['compiler_sha256'],'c_compiler_sha256':record['c_compiler_sha256'],'archiver_sha256':record['archiver_sha256'],'all_current_source_hashes_verified':True,'builder_sha256':sha(S/'build.ps1'),'command_status_evidence':'Builder records PASS only after every normal command exits0 and all19 framework terminal cases match; extra child cases are invoked by their normal suites','original_input_access':False,'original_target_execution':False,'original_modification':False,'compiled_production_DLL':False}
result.update(clean_build='Previous common build and hook fixture moved aside before clean compilation. All delivered binaries originate in this run. Verified outputs from earlier successful compiler invocations within the same run were reused after test-only warning repairs; all tests and archive commands executed again.',compiler_checkpoint={'path':'delivery/build-logs/compile-checkpoint.json','sha256':sha(run/'compile-checkpoint.json'),'seed_sha256':sha(run/'compile-checkpoint-seed.json'),'reused_same_clean_run_compiler_commands':len(reused),'reused_objects':sum(Path(c['arguments'][c['arguments'].index('-o')+1]).suffix=='.o' for c in reused),'reused_test_compilations':sum(Path(c['arguments'][c['arguments'].index('-o')+1]).suffix=='.exe' for c in reused),'test_execution_reused':False,'historical_binary_reuse':False,'compile_archive_test_recipes_unchanged':True})
result['additional_dependency_headers']=additional_headers
result['source_authority']=source_authority
result['recursive_header_checkpoint_inventory']={'headers':len(checkpoint['headers']),'inventory_only_headers':inventory_only_headers,'scope':'Recursive same-run compiler checkpoint invalidation inventory; inventory-only paths include historical before/ copies and are NOT claimed to be compiled dependency headers. Canonical additional_dependency_headers are the previous delivered tinyxml2 header and the two added production headers from the authorized rebuild baseline.'}
result['clean_build']='Previous delivery outputs were moved aside before this clean run. Only hash-verified compiler outputs from the same run may be reused after documented repairs; archives and all tests execute again. Header changes invalidate the compiler checkpoint.'
write(S/'full-build-verification.json',result)
write(run/'common-verification.json',result)
print(json.dumps({k:result[k] for k in ['result','native_objects','normal_groups','framework_terminal_cases','compiler','all_current_source_hashes_verified']},indent=2))
