from pathlib import Path,PurePosixPath
from zipfile import ZipFile,ZIP_DEFLATED
from datetime import datetime,timezone
import hashlib,json,os,re,shutil,time
R=Path.cwd();O=R/'outputs';W=R/'work';S=O/'source-workspace';D=O/'dll-critical-path';archive=O/'dll-reverse-engineering.zip';started=time.monotonic();now=datetime.now(timezone.utc).isoformat()
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def digest(data):return hashlib.sha256(data).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def enc(v):return (json.dumps(v,ensure_ascii=False,indent=2)+'\n').encode('utf-8')
def write(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(enc(v))
def safe(n):
 p=PurePosixPath(n);assert n and not p.is_absolute() and '..' not in p.parts and ':' not in n and '\\' not in n,n
 return n
def build_path(n):return any(p=='build' or p.startswith('build-') for p in PurePosixPath(n).parts[:-1])
binary={'.o','.obj','.a','.lib','.exe','.dll','.pdb','.res','.map','.exp','.ilk','.d'}
full=read(S/'full-build-verification.json');common=read(S/'build-record.json');snapshot=read(S/'source-snapshot.json');link=read(O/'dll-link-gate-source/current-full-link.json')
assert full['result']=='PASS' and full['native_objects']==296 and full['normal_groups']==356 and full['framework_terminal_cases']==19
assert sha(S/'build-record.json')==full['build_record']['sha256']==snapshot['build_record_sha256']==link['common_build_record_sha256']
assert sha(S/'source-snapshot.json')==full['source_snapshot']['sha256']==link['source_snapshot_sha256']
assert link['diagnostic_validation']=='PASS_EXPECTED_BLOCKER' and not link['dll_created'] and not link['dll_link_succeeded']
for row in snapshot['sources']:assert sha(O/row['path'])==row['sha256']
for row in full['objects']+full['executables']:assert sha(R/row['path'])==row['sha256']
for row in full['archives']:assert sha(S/'build'/row['file'])==row['sha256']
prior=read(O/'package-receipt.json');prior_sha=sha(archive);assert prior_sha==prior['sha256']
backup=W/'package-backups/dll-reverse-engineering-before-full-build.zip';backup.parent.mkdir(exist_ok=True)
if backup.exists():assert sha(backup)==prior_sha
else:
 try:os.link(archive,backup)
 except OSError:shutil.copy2(archive,backup)
assert sha(backup)==prior_sha
write(W/'package-backups/before-full-build-package-receipt.json',prior)

notice=f'''<!-- CURRENT-FULL-BUILD -->
Güncel temiz tam ortak derleme ({now}): **296 nesne (292 C++ / 4 C), 356 test grubu ve 19 terminal senaryosu PASS**. Güncel kaynak snapshot'ı 1017 dosyadır. Tam arşiv seçimiyle üretim DLL linki denendi; `require_loader_attach_runtime` ve `require_loader_detach_quiescence` tanımları eksik olduğundan **BLOCKED**. Çalışan production DLL üretilmedi. Ayrıntı: `source-workspace/full-build-verification.json`, `dll-link-gate-source/current-full-link.json` ve `SON-DURUM.txt`. Aşağıdaki B54 ve önceki aşama kayıtları tarihseldir.
<!-- END-CURRENT-FULL-BUILD -->

'''
for p in [S/'README.md',O/'KAYNAK-DURUMU.md',O/'TAMAMLANMA-TAHMINI.md',O/'KOD-REHBERI.md',O/'YOL-HARITASI.md',O/'DLL-ANALIZ-RAPORU.md']:
 if p.is_file():
  text=p.read_text(encoding='utf-8-sig');text=re.sub(r'^<!-- CURRENT-FULL-BUILD -->.*?<!-- END-CURRENT-FULL-BUILD -->\s*','',text,flags=re.S);p.write_text(notice+text,encoding='utf-8')
plan=read(D/'active-work-plan.json');plan['delivery_milestone_pending']='Fresh full common-build and current-source ZIP delivery complete; production DLL/image integration remains blocked by missing loader contracts.'
plan['delivery_status']={'updated_at':now,'scope':'Fresh clean full common build + current sources + whole-archive blocked DLL link diagnostic','full_common_build_run':True,'objects':296,'normal_groups':356,'framework_terminal_cases':19,'production_DLL_link_attempted':True,'production_DLL_link_succeeded':False,'full_build_proof':'source-workspace/full-build-verification.json','link_proof':'dll-link-gate-source/current-full-link.json'};write(D/'active-work-plan.json',plan)
ledger=read(D/'validation.json');ledger.update({'scope':'Fresh full common-build delivery PASS; production DLL whole-archive link BLOCKED by two loader hooks','full_build_or_link_run':True,'planned_counts_executed':True,'full_common_build':{'path':'source-workspace/full-build-verification.json','sha256':sha(S/'full-build-verification.json'),'result':'PASS','objects':296,'normal_groups':356,'framework_terminal_cases':19},'full_production_link':{'path':'dll-link-gate-source/current-full-link.json','sha256':sha(O/'dll-link-gate-source/current-full-link.json'),'readiness':'BLOCKED','dll_created':False},'comparison_baseline_snapshot':'source-workspace/history/B54/source-snapshot.json','packaging':{'updated_at':now,'mode':'fresh full build outputs only; no historical compiled output overlay','full_common_build_run':True,'production_DLL_link_run':True,'compiled_production_DLL':False}});ledger['artifacts']['active-work-plan.json']=sha(D/'active-work-plan.json');write(D/'validation.json',ledger)

with ZipFile(archive) as old:
 old_manifest=json.loads(old.read('package-manifest.json'));old_files={r['path']:r for r in old_manifest['files']}
 assert len(old.namelist())==len(set(old.namelist()))==len(old_files)+1
 # Keep historical TEXT/source evidence; no prior compiled file/build tree or
 # old focused-artifact tree can silently survive under canonical names.
 retained={n for n in old_files if not build_path(n) and PurePosixPath(n).suffix.lower() not in binary and not n.startswith(('validation-work/','delivery/'))}
 removed=sorted(set(old_files)-retained);overlays={}
 def add(p):overlays[safe(p.relative_to(O).as_posix())]=p
 for folder in O.iterdir():
  if folder.is_dir() and folder.name.endswith('-source'):
   for p in folder.rglob('*'):
    name=p.relative_to(O).as_posix()
    if build_path(name) or p.suffix.lower() in binary or any(x in {'.git','__pycache__'} for x in p.parts):continue
    if p.is_file():add(p)
 for folder in [S,D]:
  for p in folder.rglob('*'):
   name=p.relative_to(O).as_posix()
   if build_path(name) or p.suffix.lower() in binary or '__pycache__' in p.parts:continue
   if p.is_file():add(p)
 for row in snapshot['sources']:add(O/row['path'])
 # These are the ONLY permitted compiled-artifact locations, all regenerated.
 fresh_roots=[S/'build',O/'hook-engine-source/testing/build',O/'dll-link-gate-source/build-current-full']
 fresh=set()
 for folder in fresh_roots:
  for p in folder.rglob('*'):
   if p.is_file():add(p);fresh.add(p.relative_to(O).as_posix())
 for p in [O/'KAYNAK-DURUMU.md',O/'TAMAMLANMA-TAHMINI.md',O/'KOD-REHBERI.md',O/'YOL-HARITASI.md',O/'DLL-ANALIZ-RAPORU.md']:add(p)
 names=(retained|set(overlays))-{'package-manifest.json'}
 assert {n for n in names if build_path(n)}==fresh
 assert all(n in fresh for n in names if PurePosixPath(n).suffix.lower() in binary)
 assert not any(n.startswith('validation-work/') for n in names)
 docs=[{'path':n,'sha256':sha(overlays[n]) if n in overlays else old_files[n]['sha256']} for n in sorted(names) if n.lower().endswith('.md')]
 previous_doc=S/'documentation-snapshot.json';history_doc=S/'history/B54/documentation-snapshot.json'
 if previous_doc.exists() and not history_doc.exists():history_doc.parent.mkdir(parents=True,exist_ok=True);history_doc.write_bytes(previous_doc.read_bytes());add(history_doc)
 write(previous_doc,{'recorded_at':now,'scope':'Documentation accompanying fresh full common build; prior B54 documentation snapshot retained in history','build_record_sha256':sha(S/'build-record.json'),'documents':docs});add(previous_doc)
 current_sources={'schema':'current-source-delivery.v2','recorded_at':now,'scope':'1017 current sources accompanying completed clean full common build','current_source_count':1017,'sources':[{'path':r['path'],'bytes':(O/r['path']).stat().st_size,'sha256':r['sha256']} for r in snapshot['sources']],'baseline_snapshot':{'path':'source-workspace/history/B54/source-snapshot.json','sha256':sha(S/'history/B54/source-snapshot.json'),'historical_common_build':True},'current_common_source_snapshot':{'path':'source-workspace/source-snapshot.json','sha256':sha(S/'source-snapshot.json')},'full_common_build_run':True,'objects':296,'normal_groups':356,'framework_terminal_cases':19,'production_DLL_link_run':True,'production_DLL_created':False}
 status={'schema':'fresh-full-build-delivery.v1','updated_at':now,'scope':'Fresh clean full common-build results and source delivery','full_common_build_run':True,'current_sources':1017,'objects':296,'cpp_objects':292,'c_objects':4,'normal_groups':356,'framework_terminal_cases':19,'full_build_proof':{'path':'source-workspace/full-build-verification.json','sha256':sha(S/'full-build-verification.json')},'source_snapshot':{'path':'source-workspace/source-snapshot.json','sha256':sha(S/'source-snapshot.json')},'production_DLL_link':{'path':'dll-link-gate-source/current-full-link.json','sha256':sha(O/'dll-link-gate-source/current-full-link.json'),'whole_archive':True,'readiness':'BLOCKED','dll_created':False,'undefined_symbols':link['undefined_symbols']},'compiled_artifact_policy':'Only freshly regenerated canonical common build, hook fixture and full-link diagnostic trees are included. Previous per-module build binaries and focused-artifact tree omitted. Historical textual evidence remains dated.','fresh_artifact_roots':[p.relative_to(O).as_posix() for p in fresh_roots],'removed_old_entries_count':len(removed),'historical_B54_records':'source-workspace/history/B54/','original_inputs_rechecked':False,'original_targets_executed':False,'blocked_version_files_accessed':False,'toolchain_included':False,'synthetic_DLL_note':'owned_reference_fixture.dll is our own TEST fixture, not a production replacement DLL'}
 status_text=f'''GUNCEL TEMIZ TAM ORTAK DERLEME — {now}

PASS: 296 nesne (292 C++ / 4 C), 356 ortak test grubu,
19 framework terminal senaryosu. 1017 kaynak dosyasi hash ile dogrulandi.
Eski derleme klasoru onceden yedege tasindi; bu build bos klasorde yapildi.

PRODUCTION DLL LINK: BLOCKED. Tam anadius+hook arşivleri linke secildi.
Eksik uretim tanimlari: require_loader_attach_runtime,
require_loader_detach_quiescence. Calisan production DLL uretilmedi.
ZIP'teki owned_reference_fixture.dll yalnizca kendi sentetik test DLL'imizdir.

Guncel build: source-workspace/build/
Tam kayit: source-workspace/build-record.json
Tam dogrulama: source-workspace/full-build-verification.json
Kaynak snapshot: source-workspace/source-snapshot.json
Link sonucu: dll-link-gate-source/current-full-link.json
Paket ayrintisi: delivery/latest-source-status.json

Eski per-module derleme dosyalari ve onceki focused binary agaci yeni ZIP'e
alinmadi. B54 build/source/document snapshot kayitlari yalniz history/B54
altinda tarihseldir; eski raporlardaki sayilar o asamalara aittir.
Orijinaller okunmadi/calistirilmadi/degistirilmedi; engelli version girdisi
veya ciktisi yeniden denenmedi. Derleyici/toolchain pakete dahil degildir.
'''
 verifier='''from pathlib import Path
import hashlib,json
root=Path(__file__).resolve().parents[1]
m=json.loads((root/'package-manifest.json').read_text(encoding='utf-8-sig'))
for row in m['files']:
 p=(root/row['path']).resolve();assert p.is_relative_to(root),row['path']
 assert p.stat().st_size==row['bytes'],row['path']
 with p.open('rb') as f:assert hashlib.file_digest(f,'sha256').hexdigest()==row['sha256'],row['path']
print('PASS:',len(m['files']),'manifested files; no target execution')
'''
 generated={'delivery/current-sources.json':enc(current_sources),'delivery/latest-source-status.json':enc(status),'delivery/removed-old-entry-paths.json':enc({'scope':'Old entries intentionally not carried over; newly regenerated canonical paths may reuse their names','paths':removed}),'SON-DURUM.txt':status_text.encode('utf-8'),'delivery/verify-package.py':verifier.encode('utf-8')}
 for n,data in generated.items():(O/n).parent.mkdir(parents=True,exist_ok=True);(O/n).write_bytes(data)
 # Include exact new build log and reproducibility/check scripts, not old EXEs.
 overlays['delivery/full-common-build-current.log']=W/'full-common-build-current.log'
 for name in ['verify-fresh-common-build.py','check-current-full-dll-link.py','package-fresh-full-build.py']:overlays['delivery/tools/'+name]=W/name
 names=sorted(names|set(overlays)|set(generated));names=[n for n in names if n!='package-manifest.json']
 assert all(n in names for n in ['source-workspace/build-record.json','source-workspace/source-snapshot.json','source-workspace/full-build-verification.json','source-workspace/build.ps1','hook-engine-source/build-fixture-tests.ps1','dll-link-gate-source/current-full-link.json'])
 expected={};frozen={n:sha(p) for n,p in overlays.items()};temp=O/'dll-reverse-engineering.full-build.tmp.zip';assert not temp.exists()
 print(f'Creating fresh ZIP: {len(names)+1} entries; old stale trees excluded.',flush=True)
 with ZipFile(temp,'w',ZIP_DEFLATED,compresslevel=6,allowZip64=True) as new:
  for i,n in enumerate(names):
   safe(n)
   if n in generated:data=generated[n]
   elif n in overlays:data=overlays[n].read_bytes();assert digest(data)==frozen[n],n
   else:data=old.read(n);assert len(data)==old_files[n]['bytes'] and digest(data)==old_files[n]['sha256'],n
   new.writestr(n,data);expected[n]={'path':n,'bytes':len(data),'sha256':digest(data)}
   if i%1000==0:print(f'Packed {i+1}/{len(names)}',flush=True)
  manifest={'format':'deliverable-manifest-v1','created_at':now,'scope':'Fresh full common-build/source/test delivery; production DLL link BLOCKED, not a working replacement DLL','exclusions':old_manifest['exclusions']+['Old compiled per-module outputs and previous validation-work binary tree','Compiler/toolchain binaries'],'omitted_generated_artifacts':old_manifest['omitted_generated_artifacts'],'delivery_status':{'path':'delivery/latest-source-status.json','sha256':digest(generated['delivery/latest-source-status.json'])},'files':[expected[n] for n in names]};manifest_bytes=enc(manifest);new.writestr('package-manifest.json',manifest_bytes)
print('Verifying every fresh ZIP member and exact canonical build membership.',flush=True)
with ZipFile(temp) as new:
 assert len(new.namelist())==len(set(new.namelist()))==len(expected)+1
 assert {n for n in new.namelist() if n.startswith('source-workspace/build/')}=={p.relative_to(O).as_posix() for p in (S/'build').rglob('*') if p.is_file()}
 assert {n for n in new.namelist() if build_path(n)}==fresh
 for i,(n,row) in enumerate(expected.items()):
  data=new.read(n);assert len(data)==row['bytes'] and digest(data)==row['sha256'],n
  if i%1500==0:print(f'Verified {i+1}/{len(expected)}',flush=True)
 assert new.read('package-manifest.json')==manifest_bytes
 assert not any(n.startswith('validation-work/') for n in new.namelist())
 assert [n for n in new.namelist() if n.endswith('.dll')]==['source-workspace/build/owned_reference_fixture.dll']
for n,p in overlays.items():assert sha(p)==frozen[n],n
final_sha=sha(temp);final_size=temp.stat().st_size;assert sha(archive)==prior_sha and sha(backup)==prior_sha
os.replace(temp,archive);assert sha(archive)==final_sha
(O/'package-manifest.json').write_bytes(manifest_bytes)
receipt={'path':str(archive),'updated_at':now,'bytes':final_size,'sha256':final_sha,'entries':len(expected)+1,'all_manifested_entries_hash_verified':True,'canonical_build_membership_verified':True,'all_packaged_compiled_artifacts_fresh':True,'full_common_build_run':True,'objects':296,'normal_groups':356,'framework_terminal_cases':19,'production_DLL_link_attempted':True,'production_DLL_created':False,'production_DLL_readiness':'BLOCKED','previous_delivery_backup':{'path':str(backup),'sha256':prior_sha},'input_integrity':'Historical original-input records retained; original inputs were not accessed in this full-build/repack','unavailable_inputs':prior['unavailable_inputs'],'omitted_generated_artifacts':prior['omitted_generated_artifacts'],'toolchain_bundled':False}
write(O/'package-receipt.json',receipt);write(W/'fresh-full-build-package-results.json',{'result':'PASS','seconds':round(time.monotonic()-started,3),'archive':receipt,'full_build_proof_sha256':sha(S/'full-build-verification.json'),'whole_archive_link_proof_sha256':sha(O/'dll-link-gate-source/current-full-link.json'),'stale_old_entries_not_carried_over':len(removed),'original_access':False,'target_execution':False})
print(json.dumps({'result':'PASS','seconds':round(time.monotonic()-started,3),'bytes':final_size,'sha256':final_sha,'entries':len(expected)+1,'fresh_artifacts':len(fresh),'removed_old_entries':len(removed)},indent=2),flush=True)
