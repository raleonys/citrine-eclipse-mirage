"""2026-11-05 authorized build/link status; this script executes no project code."""
from pathlib import Path
from datetime import datetime, timezone
import hashlib,json,re

root=Path(__file__).resolve().parents[1];out=root/'outputs';critical=out/'dll-critical-path';common=out/'source-workspace'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def write(p,data):p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def proof(p):return {'path':p.relative_to(root).as_posix(),'sha256':sha(p)}

run=Path((root/'work/current-rebuild-path.txt').read_text(encoding='utf-8-sig').strip()).resolve()
assert run.parent==(root/'work').resolve()
full=read(common/'full-build-verification.json');snap=read(common/'source-snapshot.json');link=read(out/'dll-link-gate-source/current-full-link.json')
assert full['result']=='PASS' and full['run']==link['rebuild_run']==run.relative_to(root).as_posix()
assert sha(common/'build-record.json')==full['build_record']['sha256']==snap['build_record_sha256']
assert sha(common/'source-snapshot.json')==full['source_snapshot']['sha256']==link['source_snapshot_sha256']
additional=snap.get('additional_dependency_headers',[])
assert additional==full.get('additional_dependency_headers',[])
assert len(snap['sources'])==full['source_snapshot']['sources']
assert {'anadius-application-io-source/call.hpp','anadius-socket-source/memory_lookup.hpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.h'}<={r['path'] for r in additional}
source_rows=snap['sources']+additional
assert len({r['path'] for r in source_rows})==len(source_rows)
for row in source_rows:
 source_path=(out/row['path']).resolve(strict=True)
 assert source_path.is_relative_to(out.resolve()) and source_path.suffix.lower() not in {'.dll','.exe','.obj','.o','.a','.lib'},row['path']
 assert sha(source_path)==row['sha256'],row['path']
linked=link['dll_link_succeeded'];assert link['dll_link_attempted'] and not link['dll_loaded_or_executed']
if linked:
 assert link['dll_created'] and not link['undefined_symbols']
 artifact=(out/link['dll_artifact']['path']).resolve(strict=True)
 assert artifact.parent==(out/'dll-link-gate-source/build-current-full').resolve() and artifact.name=='anadius64.reconstruction.dll'
 assert sha(artifact)==link['dll_artifact']['sha256']
now=datetime.now(timezone.utc).isoformat();status='BUILD_AND_LINK_PASS_NOT_RUNTIME_VALIDATED' if linked else 'COMMON_BUILD_PASS_LINK_BLOCKED'
fullproof=proof(common/'full-build-verification.json');linkproof=proof(out/'dll-link-gate-source/current-full-link.json')
archive=run/'metadata-before-build-status';archive.mkdir(exist_ok=False)
for name in ['pending-source-status.json','active-work-plan.json','blockers.json','validation.json']:(archive/name).write_bytes((critical/name).read_bytes())
milestone_path=(root/read(critical/'active-work-plan.json')['current_static_milestone']['path']).resolve(strict=True)
assert milestone_path.parent==critical.resolve() and milestone_path.name.startswith('static-source-milestone-')
(archive/milestone_path.name).write_bytes(milestone_path.read_bytes())
milestone=read(milestone_path)
previous=read(critical/'pending-source-status.json');history=run/'historical-common'
preserved=[{'previous_path':'outputs/source-workspace/'+name,**proof(history/name)} for name in ['source-snapshot.json','build-record.json','full-build-verification.json']]
preserved.append({'previous_path':milestone_path.relative_to(root).as_posix(),**proof(archive/milestone_path.name)})
for name in ['historical-package-receipt.json','historical-full-link.json']:
 if (run/name).exists():preserved.append(proof(run/name))
evidence={'schema':'authorized-rebuild.v1','recorded_at':now,'client_date':'2026-11-05','client_timezone':'Europe/Istanbul','status':status,'run':run.relative_to(root).as_posix(),'scope':'Explicit user-authorized clean common build/tests, actual whole-archive production DLL link; no production DLL loading/execution','source_evidence_previous':previous['source_evidence'],'canonical_sources':len(snap['sources']),'additional_dependency_headers':snap.get('additional_dependency_headers',[]),'all_pending_sources_since_last_full_build':[],'current_source_matches_last_full_build':True,'compiled':True,'tests_executed':True,'link_attempted':True,'dll_link_succeeded':linked,'DLL_executed':False,'original_modified':False,'original_binary_accessed':False,'new_normal_native_body_recoveries':0,'new_native_ABI_closures':0,'overall_completion_percentage':None,'full_build':fullproof,'production_link':linkproof,'source_repairs':read(run/'source-repairs.json') if (run/'source-repairs.json').exists() else [],'previous_proofs_preserved':preserved,'frozen_build_link_receipt_hashes':[proof(common/n) for n in ['build-record.json','source-snapshot.json','full-build-verification.json']]+[linkproof],'package_receipt_note':'Package receipt is issued externally after ZIP verification; excluded here to avoid an archive/receipt hash cycle. Prior receipt retained in run history.'}
ep=critical/'current-rebuild.evidence.json'
evidence.update(additional_dependency_header_count=len(additional),total_verified_source_files=len(source_rows))
if ep.exists():(archive/ep.name).write_bytes(ep.read_bytes())
write(ep,evidence);current=proof(ep)
# This selected mutable milestone is current metadata. Its byte-for-byte pause
# record is archived above; frozen pause/stage evidence files are not changed.
milestone['source_only_limits_before_authorized_build']=milestone.get('remaining_validation_limits',[])
milestone.update(recorded_at=now,pending_source_count=0,current_source_matches_last_full_build=True,
 continuous_goal_active=False,work_status='PAUSED_AT_USER_REQUEST',awaiting_explicit_rebuild_instruction=False,
 async_question_pending=False,future_rebuild_requires_explicit_instruction=True,
 compile=True,tests_run=True,link=True,DLL_execution=False,ZIP_updated=False,
 previous_milestone_record=proof(archive/milestone_path.name),latest_authorized_build=current,
 full_common_build=fullproof,production_link=linkproof,
 builder_status={'status':status,'run':run.relative_to(root).as_posix(),'ZIP':'Authorized packaging follows verification','production_DLL_loaded_or_executed':False},
 independent_static_work_in_progress='RE source work remains paused at the user request. Only the explicitly authorized build/test/link and delivery run was performed.')
milestone['tracked_family_decision']=milestone.get('tracked_family_decision','').replace('Full objective active;','Further RE work paused at user request;')
milestone['remaining_validation_limits']=[
 'Authorized common build/tests and actual production link are recorded separately by the current build/link proofs; the production DLL was not loaded or executed.',
 'Passing reconstructed-component and injected-SDK tests is not native runtime/loader/ABI/EH equivalence or real-network cancellation proof.',
 'Returned-child-old1, child response20/88/completeIO/scheduling/shared/accept/packet/fullnative tail remain open; production Parent remains Serialized.',
 'The original binaries and historical source-only stage/pause evidence remain unchanged. No new native-body recovery, native ABI closure or completion percentage is claimed.',
 'RE source work is still paused at the user request; fresh build/delivery authorization does not resume it. Future rebuilds require an explicit instruction.']
milestone['current_validation_gate']={'reason':'Current authorized common build/tests and production link have their actual proofs; runtime/native gaps remain open and RE work remains user-paused. ZIP receipt follows atomic packaging.','consecutive_goal_turns_at_impasse':0,'no_repeat_question':True}
write(milestone_path,milestone)
p=previous;p.setdefault('source_evidence_history',[]).append(p['source_evidence'])
prior_receipt=read(run/'historical-package-receipt.json')
p['last_verified_ZIP']={'path':(run/'previous-delivery.zip').relative_to(root).as_posix(),'sha256':prior_receipt['sha256'],'scope':'Previous delivery backup. Current authorized ZIP result is issued externally in outputs/package-receipt.json after archive verification.'}
p['current_delivery_receipt']={'path':'outputs/package-receipt.json','scope':'External receipt issued after atomic ZIP verification; see its actual hash/date/run.'}
p.update(recorded_at=now,client_date='2026-11-05',status=status,changed_sources_since_last_full_build=[],last_verified_common_build={**fullproof,'scope':'All current canonical sources and extra dependency headers verified with the actual clean build/test run.'},latest_full_build=fullproof,source_evidence=current,current_source_matches_last_full_build=True,compilation_tests_link_execution_performed=True,latest_production_link=linkproof,production_DLL_created=link['dll_created'],production_DLL_loaded_or_executed=False)
write(critical/'pending-source-status.json',p)
plan=read(critical/'active-work-plan.json');plan.update(current_source_matches_last_full_build=True,last_code_result={**current,'status':status,'scope':evidence['scope']},delivery_status={'status':status,'build':fullproof,'link':linkproof,'ZIP':'Authorized update follows verification','runtime_equivalence_established':False},pending_source_status='outputs/dll-critical-path/pending-source-status.json',pending_source_status_proof=proof(critical/'pending-source-status.json'))
plan['current_static_milestone']=proof(milestone_path)
plan.setdefault('continuous_user_goal',{})['active']=False
plan['entry_implementation_status']='Both loader helpers and Image providers compiled with current source; '+('whole-archive production DLL link passed. Runtime/loader equivalence remains unverified.' if linked else 'whole-archive production DLL link blocked; inspect actual current diagnostics.')
plan['delivery_milestone_pending']='Current common build/tests complete. Authorized ZIP update follows this record; actual runtime/loader validation remains open.'
plan.setdefault('completed_now',[]).append('AUTHORIZED REBUILD: current common build/tests PASS; production link '+('PASS, DLL NOT EXECUTED.' if linked else 'BLOCKED; see actual diagnostics.'))
write(critical/'active-work-plan.json',plan)
b=read(critical/'blockers.json');b.update(full_build_run=True,link_run=True,pending_source_delta={'status':'CURRENT_COMMON_BUILD_VERIFIED','count':0,'new_files':[],'changed_sources':[]},latest_authorized_rebuild=current,production_dll_readiness=link['production_dll_readiness'])
for item in b['items']:
 if item['id'] in ['entry-lifecycle','resource-assembly'] and linked:item['status']='COMPILED_AND_LINKED_NATIVE_RUNTIME_UNVERIFIED'
write(critical/'blockers.json',b)
v=read(critical/'validation.json');v.update(result=status,scope=evidence['scope'],full_build_or_link_run=True,current_change_checks_executed=True,current_source_matches_last_full_build=True,pending_source_changes=[],full_common_build=fullproof,full_production_link=linkproof,latest_source_stage=current)
v['packaging']={'status':'AUTHORIZED_CURRENT_PACKAGE_FOLLOWS_BUILD_STATUS','receipt':'outputs/package-receipt.json','runtime_or_loader_equivalence_established':False}
for name in ['active-work-plan.json','pending-source-status.json','blockers.json']:v['artifacts'][name]=sha(critical/name)
write(critical/'validation.json',v)
notice='<!-- PENDING-SOURCE-CHANGES -->\n2026-11-05: Kullanıcının açık isteğiyle güncel tam ortak derleme ve testler geçti. '+('Üretim DLL linki başarılı; DLL yüklenmedi/çalıştırılmadı.' if linked else 'Üretim DLL linki engelli; gerçek hata güncel link kaydındadır.')+' '+str(full['native_objects'])+' nesne, '+str(full['normal_groups'])+' test grubu ve '+str(full['framework_terminal_cases'])+' framework terminal senaryosu doğrulandı. Güncel kaynakların son tam derlemeye göre bekleyen farkı 0. Bu sonuç tam davranış/loader/native ABI eşdeğerliği kanıtı değildir. Kanıt: `dll-critical-path/current-rebuild.evidence.json`. Önceki aşama metinleri tarihseldir; ZIP sonucu ayrı paket kaydındadır.\n<!-- END-PENDING-SOURCE-CHANGES -->\n\n'
docs=[out/n for n in ['KAYNAK-DURUMU.md','TAMAMLANMA-TAHMINI.md','KOD-REHBERI.md','YOL-HARITASI.md','DLL-ANALIZ-RAPORU.md']]+[common/'README.md']
docs += [d/'README.md' for d in out.iterdir() if d.is_dir() and d.name.endswith('-source')]
for p in docs:
 if not p.is_file():continue
 text=p.read_text(encoding='utf-8-sig')
 if '<!-- PENDING-SOURCE-CHANGES -->' not in text:continue
 (archive/p.relative_to(out).as_posix().replace('/','__')).write_bytes(p.read_bytes())
 text=re.sub(r'<!-- PENDING-SOURCE-CHANGES -->.*?<!-- END-PENDING-SOURCE-CHANGES -->\s*','',text,flags=re.S)
 p.write_text(notice+text,encoding='utf-8')
state=read(run/'run.json');state.update(status='VERIFIED_BUILD_LINK_COMPLETE',common_build_result='PASS',production_DLL_link_succeeded=linked,production_DLL_created=link['dll_created'])
write(run/'run.json',state)
print(json.dumps({'status':status,'pending_sources':0,'common_groups':full['normal_groups'],'DLL_link_succeeded':linked},indent=2))
