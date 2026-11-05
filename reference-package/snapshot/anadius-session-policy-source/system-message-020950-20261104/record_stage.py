"""One-time source/evidence bookkeeping; never runs project code."""
import hashlib,json,shutil
from pathlib import Path
from datetime import datetime,timezone
stage=Path(__file__).resolve().parent;root=stage.parents[2];out=root/'outputs';critical=out/'dll-critical-path'
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def sha(p):
    with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def write(p,v):p.write_text(json.dumps(v,indent=2,ensure_ascii=True)+'\n',encoding='utf-8')
def proof(p):return {'path':p.relative_to(root).as_posix(),'sha256':sha(p)}
assert not (stage/'evidence.json').exists()
production=['anadius-socket-source/native.hpp','anadius-socket-source/native.cpp','anadius-session-policy-source/policy.hpp','anadius-session-policy-source/policy.cpp']
tests=['anadius-socket-source/native_tests.cpp','anadius-session-policy-source/policy_tests.cpp'];paths=production+tests
prior=read(stage/'before/pending-source-status.json');old=read(root/prior['source_evidence']['path'])
snapshot=read(out/'source-workspace/source-snapshot.json');rows=snapshot['sources']+snapshot.get('additional_dependency_headers',[])
canonical={r['path']:r for r in rows};old_pending={r['path']:r['current_sha256'] for r in old['pending_sources']}
source_changes=[]
for p in paths:
    before=stage/'before'/(Path(p).name+'.before.txt')
    assert sha(before)==old_pending.get(p,canonical[p]['sha256']),p
    assert sha(out/p)!=sha(before),p
    source_changes.append({**proof(out/p),'before':proof(before),'before_image_kind':'RECONSTRUCTED_KNOWN_BYTES_VERIFIED_AGAINST_PRIOR_SHA' if p in production else 'CAPTURED_BEFORE_EDIT'})
changes=[{'path':r['path'],'build_sha256':r['sha256'],'current_sha256':sha(out/r['path'])} for r in rows if sha(out/r['path'])!=r['sha256']]
assert {r['path'] for r in changes}==set(prior['changed_sources_since_last_full_build'])|set(paths)
review_names=['production-independent-review.md','root-independent-review.md']
reviews='\n'.join((stage/n).read_text(encoding='utf-8-sig') for n in review_names)
for p in paths:assert sha(out/p) in reviews,p
frozen_paths=['source-workspace/build-record.json','source-workspace/source-snapshot.json','source-workspace/full-build-verification.json','dll-link-gate-source/current-full-link.json','dll-link-gate-source/build-current-full/anadius64.reconstruction.dll','dll-reverse-engineering.zip','package-receipt.json']
previous_frozen={r['path']:r['sha256'] for r in old['frozen_build_and_delivery']};frozen=[proof(out/p) for p in frozen_paths]
for r in frozen:assert r['sha256']==previous_frozen[r['path']],r
assert sha(Path('C:/Users/Skysect/Downloads/anadius64.dll'))=='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
native=[]
for p in sorted(stage.glob('*.json')):
    if p.name.startswith(('pseudo-','instructions-','bytes-')):
        reply=read(p);e=reply.get('structuredContent') or json.loads(reply['content'][0]['text'])
        assert e['provider']['id']=='ghidra' and e['provider']['version']=='12.1.4'
        native.append({**proof(p),'evidence_id':e['evidence_id'],'operation':e['operation'],'parameters':e['parameters']})
assert len(native)==6
now=datetime.now(timezone.utc).isoformat();status='SOURCE_IMPLEMENTED_STATIC_REVIEWED_NOT_COMPILED'
scope='Concrete020950 system-category message values and exact FormatMessageA/count-trim/Cstrlen/LocalFree sequence replace compiler-dependent default Policy callback; explicit formatter/generic/misc and compiler exception ABI remain distinct.'
limits=['Current source is not compiled/tested/linked; actual DLL remains historical LINKED_NOT_EXECUTED.','Native string/SSO/heap/EH and compiler std::system_error category/what equivalence remain unproven.','No real OS/locale matrix, original DLL execution or whole-DLL equivalence is established.','Tracked actual prepared-child last-release and Runtime post-error latch integration coverage remain open.']
result={'schema':'system-message-020950.source.v1','recorded_at':now,'client_date':'2026-11-04','status':status,'scope':scope,'fresh_REA_evidence':native,'native_findings':proof(stage/'native-findings.md'),'native_session_records':[proof(stage/n) for n in ['session.json','open.json','export.json','rea-evidence-bundle.json','close.json','original-hash.json']],'native_session_closed':True,'seed_triage':proof(out/'anadius-owned-environment-source/tracked-executor-implementation-20261104/next-native-candidates.md'),'report':proof(stage/'report.md'),'implementation':proof(stage/'implementation.md'),'before_recovery':proof(stage/'before-recovery.md'),'test_source_notes':proof(stage/'test-source-notes.md'),'independent_reviews':[proof(stage/n) for n in review_names],'source_changes':source_changes,'production_files_changed':len(production),'test_files_changed':len(tests),'pending_source_count':len(changes),'pending_sources':changes,'remaining_limits':limits,'compile':False,'tests_run':False,'link':False,'DLL_execution':False,'ZIP_updated':False,'original_unchanged':True,'frozen_build_and_delivery':frozen,'overall_completion_percentage':None,'continuous_goal_active':True}
write(stage/'evidence.json',result);current=proof(stage/'evidence.json')
for n in ['blockers.json','validation.json','static-source-milestone-20261104.json']:shutil.copyfile(critical/n,stage/'before'/n)
p=read(critical/'pending-source-status.json');p.setdefault('source_evidence_history',[]).append(p['source_evidence']);p.update(recorded_at=now,status=status,source_evidence=current,changed_sources_since_last_full_build=[r['path'] for r in changes],current_source_matches_last_full_build=False,compilation_tests_link_execution_performed=False,latest_source_review=current);p['last_verified_common_build']['scope']=f'Historical successful full build;{len(changes)} pending source/header changes require a consistent explicitly authorized rebuild.';write(critical/'pending-source-status.json',p)
plan=read(critical/'active-work-plan.json');plan.update(current_source_matches_last_full_build=False,last_code_result={**current,'status':status,'scope':scope},current_production_sources_changed=True,pending_source_status_proof=proof(critical/'pending-source-status.json'),latest_native_audit=current)
plan['completed_now'].append(f'SOURCE_ONLY actual selected systemcategory message020950 reconstructed and bound inPolicy.6freshREA records exported/closed;4production+2test sources independentlyreviewed,NOTRUN.{len(changes)}pending. Exact normal SDK/trim/Cstr/free sequence; nativeEH/compilerexceptionABI notclosed. No build/link/package.')
plan['next_native_code_target']={'scope':'Preserve full reconstruction objective. Use the retained prepared-child fixture plan for actual Runtime/Policy capability source validation and continue selecting concrete remaining application/native behavior gaps from the full evidence/source coverage ledger. Do not replace full DLL fidelity with a default-route or message-only completion claim.','validation':f'{len(changes)} pending source/header changes. No compile/project tests/link/DLL execution/ZIP update without explicit instruction.'}
plan['user_required_REA_workflow']['current_fresh_request']={'status':'EVIDENCE_EXPORTED_SESSION_CLOSED','path':stage.relative_to(root).as_posix(),'evidence':[r['evidence_id'] for r in native]}
plan['user_required_REA_workflow']['latest_observation']='Fresh028620→A1110→84248slot10→020950;275byteFormatMessageA1300/lang400,atmostLFthenCR,countvsCstrlen,fallbackasio.system error,LocalFreeincludingNULL. Source defaultPolicy message nowconcrete. Static only.'
plan['user_required_REA_workflow']['delegation_policy']['current_tasks']=['system-message-native-evidence-and-independent-review','system-message-source-and-baseline-recovery','system-message-test-sources-and-prepared-fixture-plan']
write(critical/'active-work-plan.json',plan)
b=read(critical/'blockers.json');b.update(pending_source_delta={'status':status,'count':len(changes),'new_files':sorted(set(paths)-set(prior['changed_sources_since_last_full_build'])),'changed_sources':[r['path'] for r in changes]},latest_source_stage=current);write(critical/'blockers.json',b)
v=read(critical/'validation.json');v.update(result=status,scope=scope,current_change_checks_executed=False,current_source_matches_last_full_build=False,pending_source_changes=[r['path'] for r in changes],latest_source_stage=current,current_stage_full_build_or_link_run=False);write(critical/'validation.json',v)
m=read(critical/'static-source-milestone-20261104.json');m.update(recorded_at=now,pending_source_count=len(changes),latest_source_milestone=current,system_message_decision='Concrete020950 default system message callback implemented and statically reviewed; generic/misc/explicit override preserved; native EH/ABI remains open.',independent_static_work_in_progress='Actual prepared-child capability fixture source work and full-objective native/source gap selection remain useful.',remaining_validation_limits=limits,current_validation_gate={'reason':'Explicit rebuild required for compiler/test/link evidence. Further reconstruction source work remains useful and active.','consecutive_goal_turns_at_impasse':0,'no_repeat_question':True});write(critical/'static-source-milestone-20261104.json',m)
plan=read(critical/'active-work-plan.json');plan['current_static_milestone']=proof(critical/'static-source-milestone-20261104.json');write(critical/'active-work-plan.json',plan)
assert frozen==[proof(out/p) for p in frozen_paths]
write(stage/'metadata-result.json',{'pending_source_count':len(changes),'frozen_build_and_delivery_unchanged':True,'updated_records':[proof(critical/n) for n in ['pending-source-status.json','active-work-plan.json','blockers.json','validation.json','static-source-milestone-20261104.json']]})
print(json.dumps({'status':status,'fresh_REA_records':len(native),'production_changes':len(production),'test_changes':len(tests),'pending_sources':len(changes),'frozen_DLL_ZIP_unchanged':True,'continuous_goal_active':True}))
