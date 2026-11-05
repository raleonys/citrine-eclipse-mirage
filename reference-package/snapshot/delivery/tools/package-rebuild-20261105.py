"""2026-11-05: package verified fresh outputs, source/header proofs and text history."""
from pathlib import Path,PurePosixPath
from zipfile import ZipFile,ZIP_DEFLATED
from datetime import datetime,timezone
import hashlib,json,os,re,shutil,time
R=Path.cwd();O=R/'outputs';S=O/'source-workspace';D=O/'dll-critical-path';archive=O/'dll-reverse-engineering.zip'
run=Path((R/'work/current-rebuild-path.txt').read_text(encoding='utf-8-sig').strip()).resolve(strict=True);now=datetime.now(timezone.utc).isoformat();started=time.monotonic()
assert run.parent==(R/'work').resolve() and run.name.startswith('rebuild-')
sha_cache={}
def sha(p):
 st=p.stat();key=(st.st_dev,st.st_ino,st.st_size,st.st_mtime_ns)
 if key not in sha_cache:
  with p.open('rb') as f:sha_cache[key]=hashlib.file_digest(f,'sha256').hexdigest()
 return sha_cache[key]
def digest(data):return hashlib.sha256(data).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def enc(v):return (json.dumps(v,ensure_ascii=False,indent=2)+'\n').encode('utf-8')
def write(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(enc(v))
def safe(name):
 p=PurePosixPath(name);assert name and not p.is_absolute() and '..' not in p.parts and ':' not in name and '\\' not in name,name
 return name
def built(name):
 if name.startswith('delivery/build-logs/'):return False
 return any(p=='build' or p.startswith('build-') for p in PurePosixPath(name).parts[:-1])
binary={'.o','.obj','.a','.lib','.exe','.dll','.pdb','.res','.map','.exp','.ilk','.d'}
full=read(S/'full-build-verification.json');common=read(S/'build-record.json');snapshot=read(S/'source-snapshot.json');link=read(O/'dll-link-gate-source/current-full-link.json')
assert full['result']==common['result']=='PASS' and full['run']==run.relative_to(R).as_posix()
assert sha(S/'build-record.json')==full['build_record']['sha256']==snapshot['build_record_sha256']==link['common_build_record_sha256']
assert sha(S/'source-snapshot.json')==full['source_snapshot']['sha256']==link['source_snapshot_sha256']
additional=snapshot.get('additional_dependency_headers',[])
assert additional==full.get('additional_dependency_headers',[])
canonical=snapshot['sources'];source_rows=canonical+additional
assert len(canonical)==full['source_snapshot']['sources']
assert {'anadius-application-io-source/call.hpp','anadius-socket-source/memory_lookup.hpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.h'}<={r['path'] for r in additional}
assert len({r['path'] for r in source_rows})==len(source_rows),'Duplicate canonical/additional source paths'
for row in source_rows:
 source_path=(O/safe(row['path'])).resolve(strict=True)
 assert source_path.is_relative_to(O.resolve()) and source_path.suffix.lower() not in binary,row['path']
 assert sha(source_path)==row['sha256'],row['path']
for row in full['objects']+full['executables']:assert sha(R/row['path'])==row['sha256']
for row in full['archives']:assert sha(S/'build'/row['file'])==row['sha256']
# The normal executable list excludes the hook fixture; the synthetic DLL is
# also a separately recorded current-run artifact, never an original input.
for row in [full['hook_fixture'],full['own_synthetic_DLL']]:assert sha(R/row['path'])==row['sha256']
assert sha(R/full['hook_fixture']['record_path'])==full['hook_fixture']['record_sha256']
# Validate the actual current link before changing any delivery metadata.
assert link['schema']=='fresh-full-archive-dll-link.v3'
assert link['rebuild_run']==full['run'] and link['expected_missing_production_hooks']==[]
assert link['entry_source_sha256']==sha(O/'dll-link-gate-source/entry.cpp')
assert link['header_sha256']==sha(O/'dll-link-gate-source/production_image.hpp')
assert not link['dll_loaded_or_executed'] and not link['originals_accessed']
artifact=link['dll_artifact']
if link['dll_created']:
 assert artifact is not None
 artifact_path=(O/artifact['path']).resolve()
 assert artifact_path.parent==(O/'dll-link-gate-source/build-current-full').resolve()
 assert artifact_path.name=='anadius64.reconstruction.dll'
 assert artifact_path.is_file() and artifact_path.stat().st_size==artifact['bytes']>0
 assert sha(artifact_path)==artifact['sha256']
else:
 assert artifact is None and not (O/'dll-link-gate-source/build-current-full/anadius64.reconstruction.dll').exists()
linked=link['link_exit_code']==0 and link['dll_created'] and not link['undefined_symbols']
assert link['dll_link_succeeded']==linked
assert link['diagnostic_validation']==('LINKED_NOT_EXECUTED' if linked else 'UNEXPECTED_LINK_FAILURE')
assert link['production_dll_readiness']==('LINKED_NOT_RUNTIME_VALIDATED' if linked else 'BLOCKED')
run_state=read(run/'run.json')
run_state.update(status='BUILD_AND_LINK_COMPLETE_PACKAGING',common_build_result='PASS',production_DLL_link_succeeded=link['dll_link_succeeded'],production_DLL_created=link['dll_created'])
write(run/'run.json',run_state)
prior=read(O/'package-receipt.json');old_sha=sha(archive);assert old_sha==prior['sha256']
backup=run/'previous-delivery.zip'
if not backup.exists():
 try:os.link(archive,backup)
 except OSError:shutil.copy2(archive,backup)
assert sha(backup)==old_sha;write(run/'previous-package-receipt.json',prior)
assert shutil.disk_usage(O).free>archive.stat().st_size*2
objects=full['native_objects'];groups=full['normal_groups'];terminals=full['framework_terminal_cases']
canonical_count=len(canonical);additional_count=len(additional);source_count=len(source_rows)
blocked=not link['dll_link_succeeded'];state=link['production_dll_readiness']
symbols='; '.join(link['undefined_symbols']) if blocked else 'Link completed; DLL was not loaded or executed.'
build_pointer={'path':'outputs/source-workspace/full-build-verification.json','sha256':sha(S/'full-build-verification.json')}
link_pointer={'path':'outputs/dll-link-gate-source/current-full-link.json','sha256':sha(O/'dll-link-gate-source/current-full-link.json')}
p=read(D/'pending-source-status.json')
if not (run/'pending-before-rebuild-delivery.json').exists():write(run/'pending-before-rebuild-delivery.json',p)
p.update(recorded_at=now,changed_sources_since_last_full_build=[],current_source_matches_last_full_build=True,latest_full_build=build_pointer)
write(D/'pending-source-status.json',p)
plan=read(D/'active-work-plan.json')
plan['pending_source_status_proof']={'path':'outputs/dll-critical-path/pending-source-status.json','sha256':sha(D/'pending-source-status.json')}
plan['delivery_status']={'updated_at':now,'scope':'User-authorized fresh full compile/tests/link and ZIP delivery','full_common_build_run':True,'objects':objects,'normal_groups':groups,'framework_terminal_cases':terminals,'compiler':common['compiler'],'production_DLL_link_attempted':True,'production_DLL_link_succeeded':link['dll_link_succeeded'],'full_build_proof':build_pointer,'link_proof':link_pointer}
plan['delivery_milestone_pending']='Current clean compile/tests verified; authorized ZIP packaging is pending atomic archive verification. Production loader/runtime integration remains open.'
write(D/'active-work-plan.json',plan)
v=read(D/'validation.json');v.update(result='COMMON_BUILD_PASS_PRODUCTION_LINK_'+('BLOCKED' if blocked else 'NOT_RUNTIME_VALIDATED'),scope='Current full synthetic common build verified; production DLL status separately reported',full_build_or_link_run=True,planned_counts_executed=True,current_source_matches_last_full_build=True,pending_source_changes=[],full_common_build=build_pointer,full_production_link=link_pointer)
v['latest_build_validation_scope']='All normal synthetic suites and framework terminal cases; original targets and production DLL not executed. Historical source-only records retain their dates/hashes.'
v['packaging']={'updated_at':now,'mode':'Only current fresh build artifact trees plus sources and dated text evidence','full_common_build_run':True,'production_DLL_link_run':True,'compiled_production_DLL':link['dll_created']}
for name in ['blockers.json','active-work-plan.json','pending-source-status.json']:v['artifacts'][name]=sha(D/name)
write(D/'validation.json',v)
notice=f'''<!-- CURRENT-FULL-BUILD -->
Güncel temiz tam derleme ({now}): **{objects} nesne, {groups} test grubu ve {terminals} framework terminal senaryosu PASS**. Derleyici: {common['compiler']}. {canonical_count} kanonik kaynak ve {additional_count} ek bağımlılık başlığı, toplam {source_count} dosya güncel hashlerle doğrulandı; bekleyen derlenmemiş fark yok. Üretim DLL link durumu: **{state}**. {symbols} Ayrıntı: `source-workspace/full-build-verification.json`, `dll-link-gate-source/current-full-link.json`, `SON-DURUM.txt`. Önceki aşama/araç kayıtları tarihseldir.
<!-- END-CURRENT-FULL-BUILD -->

'''
docs=[S/'README.md',O/'KAYNAK-DURUMU.md',O/'TAMAMLANMA-TAHMINI.md',O/'KOD-REHBERI.md',O/'YOL-HARITASI.md',O/'DLL-ANALIZ-RAPORU.md']
for path in docs:
 if not path.exists():continue
 text=path.read_text(encoding='utf-8-sig')
 text=re.sub(r'<!-- PENDING-SOURCE-CHANGES -->.*?<!-- END-PENDING-SOURCE-CHANGES -->\s*','',text,flags=re.S)
 text=re.sub(r'<!-- CURRENT-FULL-BUILD -->.*?<!-- END-CURRENT-FULL-BUILD -->\s*','',text,flags=re.S)
 text=text.replace('Yeni araçlarla proje\nderlenmedi.','Bu teslimde proje yeni LLVM-MinGW araçlarıyla derlendi.')
 path.write_text(notice+text,encoding='utf-8')
fresh_roots=[S/'build',O/'hook-engine-source/testing/build',O/'dll-link-gate-source/build-current-full']
status={'schema':'current-rebuild-delivery.v1','updated_at':now,'full_common_build_run':True,'current_sources':source_count,'objects':objects,'normal_groups':groups,'framework_terminal_cases':terminals,'compiler':common['compiler'],'full_build_proof':build_pointer,'source_snapshot':{'path':'outputs/source-workspace/source-snapshot.json','sha256':sha(S/'source-snapshot.json')},'production_DLL_link':{**link_pointer,'readiness':state,'dll_created':link['dll_created'],'undefined_symbols':link['undefined_symbols']},'source_repairs':read(run/'source-repairs.json') if (run/'source-repairs.json').exists() else [],'fresh_artifact_roots':[p.relative_to(O).as_posix() for p in fresh_roots],'original_inputs_rechecked':False,'original_targets_executed':False,'blocked_version_files_accessed':False,'toolchain_included':False,'synthetic_DLL_note':'owned_reference_fixture.dll is our own synthetic test fixture, not the production replacement'}
status['version_proxy_scope']='version-rebuild text/source files are preserved from the previous ZIP with their original manifest hashes; this run did not rebuild or access the blocked version DLLs.'
status.update(canonical_sources=canonical_count,additional_dependency_header_count=additional_count,total_verified_source_files=source_count,additional_dependency_headers=additional)
write(O/'delivery/latest-source-status.json',status)
write(O/'delivery/current-sources.json',{'schema':'current-source-delivery.v4','recorded_at':now,'canonical_source_count':canonical_count,'additional_dependency_header_count':additional_count,'current_source_count':source_count,'sources':[{'path':r['path'],'bytes':(O/r['path']).stat().st_size,'sha256':r['sha256']} for r in canonical],'additional_dependency_headers':[{'path':r['path'],'bytes':(O/r['path']).stat().st_size,'sha256':r['sha256']} for r in additional],'current_common_source_snapshot':{'path':'source-workspace/source-snapshot.json','sha256':sha(S/'source-snapshot.json')},'full_common_build_run':True,'production_DLL_created':link['dll_created']})
(O/'delivery/verify-package.py').write_text("from pathlib import Path\nimport hashlib,json\nroot=Path(__file__).resolve().parents[1]\nm=json.loads((root/'package-manifest.json').read_text(encoding='utf-8-sig'))\nfor row in m['files']:\n p=(root/row['path']).resolve();assert p.is_relative_to(root),row['path']\n assert p.stat().st_size==row['bytes'],row['path']\n with p.open('rb') as f:assert hashlib.file_digest(f,'sha256').hexdigest()==row['sha256'],row['path']\nprint('PASS:',len(m['files']),'manifested files; no target execution')\n",encoding='utf-8')
(O/'SON-DURUM.txt').write_text(f'''GUNCEL TEMIZ DERLEME — {now}\n\nPASS: {objects} nesne; {groups} test grubu; {terminals} framework terminal senaryosu.\nDerleyici: {common["compiler"]}\n{canonical_count} kanonik kaynak + {additional_count} ek baslik = {source_count} dosya guncel hashlerle dogrulandi.\n\nPRODUCTION DLL: {state}\n{symbols}\n\nZIP sadece bu temiz rebuild'in derleme agaclarini, guncel kaynaklari ve tarihli metin kanitlarini icerir.\nowned_reference_fixture.dll kendi sentetik test DLL'idir. Production DLL yuklenmedi/calistirilmadi.\nOnceki tam build kayitlari source-workspace/history/{run.name} altinda korunur.\nOrijinal DLL/EXE/CFG dosyalarina erisilmedi, toolchain ZIP'e eklenmedi.\nAyrinti: delivery/latest-source-status.json\n''',encoding='utf-8')
with ZipFile(archive) as old:
 old_manifest=json.loads(old.read('package-manifest.json'));old_files={r['path']:r for r in old_manifest['files']}
 assert len(old.namelist())==len(set(old.namelist()))==len(old_files)+1
 retained={n for n in old_files if not built(n) and PurePosixPath(n).suffix.lower() not in binary and not n.startswith(('validation-work/','delivery/'))}
 overlays={}
 def add(path,name=None):overlays[safe(name or path.relative_to(O).as_posix())]=path
 for folder in O.iterdir():
  if folder.is_dir() and folder.name.endswith('-source'):
   for path in folder.rglob('*'):
    name=path.relative_to(O).as_posix()
    if path.is_file() and not built(name) and path.suffix.lower() not in binary and not any(p in {'.git','__pycache__'} for p in path.parts):add(path)
 for folder in [S,D]:
  for path in folder.rglob('*'):
   name=path.relative_to(O).as_posix()
   if path.is_file() and not built(name) and path.suffix.lower() not in binary and '__pycache__' not in path.parts and not name.startswith('delivery/tools/'):add(path)
 for row in source_rows:add(O/row['path'])
 for name in ['latest-source-status.json','current-sources.json','verify-package.py']:add(O/'delivery'/name)
 fresh=set()
 for folder in fresh_roots:
  for path in folder.rglob('*'):
   if path.is_file():add(path);fresh.add(path.relative_to(O).as_posix())
 for path in docs+[O/'SON-DURUM.txt']:
  if path.exists():add(path)
 for name in ['verify-rebuild-20261105.py','start-rebuild-20261105.ps1','record-rebuild-status-20261105.py','package-rebuild-20261105.py','link-current-rebuild.py','use-tools.ps1']:add(R/'work'/name,'delivery/tools/'+name)
 for path in run.glob('*.log'):
  if path.name!='package.log':add(path,'delivery/build-logs/'+path.name)
 for path in run.glob('attempt-*-failure.txt'):add(path,'delivery/build-logs/'+path.name)
 diagnostics=run/'diagnostics'
 if diagnostics.is_dir():
  for path in diagnostics.rglob('*'):
   if path.is_file() and path.suffix.lower() in {'.json','.log'}:
    assert path.resolve(strict=True).is_relative_to(diagnostics.resolve()),path
    add(path,'delivery/build-logs/'+path.relative_to(run).as_posix())
 for name in ['source-repairs.json','run.json','process-result.json','compile-checkpoint.json','compile-checkpoint-seed.json']:
  if (run/name).exists():add(run/name,'delivery/build-logs/'+name)
 previous_docs=S/'documentation-snapshot.json'
 if previous_docs.exists():
  historical=S/'history'/run.name/'documentation-snapshot.json'
  historical.parent.mkdir(parents=True,exist_ok=True)
  if not historical.exists():historical.write_bytes(previous_docs.read_bytes())
  add(historical)
 write(previous_docs,{'recorded_at':now,'scope':'Current rebuild documentation; earlier snapshots dated in history','build_record_sha256':sha(S/'build-record.json'),'documents':[{'path':n,'sha256':sha(p)} for n,p in sorted(overlays.items()) if n.endswith('.md')]});add(previous_docs)
 names=sorted((retained|set(overlays))-{'package-manifest.json'})
 assert {n for n in names if built(n)}==fresh,{'unexpected':sorted({n for n in names if built(n)}-fresh),'missing':sorted(fresh-{n for n in names if built(n)})}
 assert all(n in fresh for n in names if PurePosixPath(n).suffix.lower() in binary)
 expected={};hashes={n:sha(path) for n,path in overlays.items()};temp=O/'dll-reverse-engineering.current.tmp.zip';assert not temp.exists()
 print(f'Packaging {len(names)+1} entries from current rebuild.',flush=True)
 with ZipFile(temp,'w',ZIP_DEFLATED,compresslevel=6,allowZip64=True) as new:
  for i,name in enumerate(names):
   safe(name)
   if name in overlays:data=overlays[name].read_bytes();assert digest(data)==hashes[name],name
   else:data=old.read(name);assert len(data)==old_files[name]['bytes'] and digest(data)==old_files[name]['sha256'],name
   new.writestr(name,data);expected[name]={'path':name,'bytes':len(data),'sha256':digest(data)}
   if i%1000==0:print(f'Packed {i+1}/{len(names)}',flush=True)
  manifest={'format':'deliverable-manifest-v1','created_at':now,'scope':'User-authorized fresh full common rebuild and sources; production DLL status '+state,'exclusions':old_manifest.get('exclusions',[])+['Previous compiled artifacts and validation-work trees; toolchain binaries'],'omitted_generated_artifacts':old_manifest.get('omitted_generated_artifacts',[]),'delivery_status':{'path':'delivery/latest-source-status.json','sha256':hashes['delivery/latest-source-status.json']},'files':[expected[n] for n in names]}
  manifest_bytes=enc(manifest);new.writestr('package-manifest.json',manifest_bytes)
print('Verifying every ZIP member and exact current build membership.',flush=True)
with ZipFile(temp) as new:
 assert len(new.namelist())==len(set(new.namelist()))==len(expected)+1
 assert {n for n in new.namelist() if built(n)}==fresh
 for name,row in expected.items():
  data=new.read(name);assert len(data)==row['bytes'] and digest(data)==row['sha256'],name
 assert new.read('package-manifest.json')==manifest_bytes
 assert not any(n.startswith('validation-work/') for n in new.namelist())
 assert all(PurePosixPath(n).suffix.lower() in {'.json','.log'} for n in new.namelist() if n.startswith('delivery/build-logs/diagnostics/'))
 assert all(not built(n) and PurePosixPath(n).suffix.lower() not in binary for n in new.namelist() if n.startswith('version-rebuild/'))
 for name in new.namelist():
  if name.startswith('version-rebuild/'):assert expected[name]['sha256']==old_files[name]['sha256']
 for row in source_rows:assert expected[row['path']]['sha256']==row['sha256']
for name,path in overlays.items():assert sha(path)==hashes[name],name
new_sha=sha(temp);new_bytes=temp.stat().st_size;assert sha(archive)==old_sha and sha(backup)==old_sha
os.replace(temp,archive);assert sha(archive)==new_sha;(O/'package-manifest.json').write_bytes(manifest_bytes)
receipt={'path':str(archive),'updated_at':now,'bytes':new_bytes,'sha256':new_sha,'entries':len(expected)+1,'all_manifested_entries_hash_verified':True,'canonical_build_membership_verified':True,'all_packaged_compiled_artifacts_fresh':True,'full_common_build_run':True,'objects':objects,'normal_groups':groups,'framework_terminal_cases':terminals,'production_DLL_link_attempted':True,'production_DLL_created':link['dll_created'],'production_DLL_readiness':state,'previous_delivery_backup':{'path':str(backup),'sha256':old_sha},'unavailable_inputs':prior.get('unavailable_inputs',[]),'omitted_generated_artifacts':prior.get('omitted_generated_artifacts',[]),'original_inputs_accessed':False,'toolchain_bundled':False}
write(O/'package-receipt.json',receipt);write(run/'package-result.json',{'result':'PASS','seconds':round(time.monotonic()-started,3),'archive':receipt})
run_state.update(status='DELIVERED',finished_at=datetime.now(timezone.utc).isoformat(),archive_sha256=new_sha,archive_bytes=new_bytes)
write(run/'run.json',run_state)
print(json.dumps({'result':'PASS','bytes':new_bytes,'sha256':new_sha,'entries':len(expected)+1,'production_DLL':state},indent=2),flush=True)
