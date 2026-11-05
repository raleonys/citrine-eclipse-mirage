"""One-time source/evidence bookkeeping. Does not build or run project code."""
import hashlib,json,shutil
from pathlib import Path
from datetime import datetime,timezone
stage=Path(__file__).resolve().parent
root=stage.parents[2];out=root/'outputs';critical=out/'dll-critical-path'
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def sha(p):
    with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def write(p,v):p.write_text(json.dumps(v,indent=2,ensure_ascii=True)+'\n',encoding='utf-8')
def proof(p):return {'path':p.relative_to(root).as_posix(),'sha256':sha(p)}
assert not (stage/'evidence.json').exists()
production=['anadius-owned-environment-source/environment.cpp','anadius-owned-environment-source/environment.hpp','anadius-server-runtime-source/runtime.hpp','anadius-server-runtime-source/runtime.cpp','anadius-session-profile-source/profile.cpp','anadius-session-policy-source/policy.hpp','anadius-session-policy-source/policy.cpp','anadius-socket-source/worker_run.hpp','anadius-socket-source/worker_run.cpp','anadius-socket-source/handler_dispatch.hpp','anadius-socket-source/posted_operation.hpp','anadius-socket-source/posted_operation.cpp']
tests=['anadius-owned-environment-source/environment_tests.cpp','anadius-session-policy-source/policy_tests.cpp','anadius-socket-source/posted_operation_tests.cpp'];paths=production+tests
prior=read(stage/'before/pending-source-status.json');old=read(root/prior['source_evidence']['path'])
snap=read(out/'source-workspace/source-snapshot.json');rows=snap['sources']+snap.get('additional_dependency_headers',[])
canonical={r['path']:r for r in rows};old_pending={r['path']:r['current_sha256'] for r in old['pending_sources']}
bridge=set(production[2:7]);before={}
for p in paths:
    if p in bridge:before[p]=stage.parent/'tracked-executor-design-20261104/before'/(Path(p).name+'.before.txt')
    elif p==tests[0]:before[p]=stage/'before/environment_tests.cpp.before.txt'
    else:before[p]=stage/'before'/Path(p).name
    assert sha(before[p])==old_pending.get(p,canonical[p]['sha256']),p
    assert sha(out/p)!=sha(before[p]),p
changes=[{'path':r['path'],'build_sha256':r['sha256'],'current_sha256':sha(out/r['path'])} for r in rows if sha(out/r['path'])!=r['sha256']]
assert {r['path'] for r in changes}==set(prior['changed_sources_since_last_full_build'])|set(paths)
review_names=['root-production-review.md','environment-tests-independent-review.md','socket-and-policy-independent-review.md','independent-bridge-review.md']
reviews='\n'.join((stage/n).read_text(encoding='utf-8-sig') for n in review_names)
for p in paths:assert sha(out/p) in reviews,p
frozen_paths=['source-workspace/build-record.json','source-workspace/source-snapshot.json','source-workspace/full-build-verification.json','dll-link-gate-source/current-full-link.json','dll-link-gate-source/build-current-full/anadius64.reconstruction.dll','dll-reverse-engineering.zip','package-receipt.json']
previous_frozen={r['path']:r['sha256'] for r in old['frozen_build_and_delivery']};frozen=[proof(out/p) for p in frozen_paths]
for p in frozen:assert p['sha256']==previous_frozen[p['path']],p
original=Path('C:/Users/Skysect/Downloads/anadius64.dll')
assert sha(original)=='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
native=[]
for p in sorted(stage.glob('*.json')):
    if p.name.startswith(('pseudo-','instructions-')):
        reply=read(p);e=reply.get('structuredContent') or json.loads(reply['content'][0]['text'])
        assert e['provider']['id']=='ghidra' and e['provider']['version']=='12.1.4'
        native.append({**proof(p),'evidence_id':e['evidence_id'],'operation':e['operation'],'parameters':e['parameters']})
assert len(native)==8
bundle=read(stage/'rea-evidence-bundle.json')
now=datetime.now(timezone.utc).isoformat();status='SOURCE_IMPLEMENTED_STATIC_REVIEWED_NOT_COMPILED'
scope='Connected Initial/Tracked executor families, exact work retain/transfer/drop and immediate original/child stop capability; native handler actual022e40 dispatch with real caller slot. Source cases and independent review only.'
limits=['No current source compile, tests or link; actual DLL remains historical LINKED_NOT_EXECUTED.','Prepared-backed actual Runtime/Policy child last-release integration and Runtime failed-post latch integration remain unexecuted/uncovered.','Bound source provider accepts exact nonzero context roots; native null-context/ABI inputs and native EH are not established.','Full original application session/retirement, loader/runtime behavior and whole-DLL native equivalence remain unproven.']
result={'schema':'tracked-executor-source-implementation.v1','recorded_at':now,'client_date':'2026-11-04','status':status,'scope':scope,'fresh_REA_evidence':native,'native_findings':proof(stage/'native-findings.md'),'native_session_records':[proof(stage/n) for n in ['session.json','open.json','rea-evidence-bundle.json','close.json']],'native_session_closed':True,'supplementary_prior_evidence':['outputs/anadius-owned-environment-source/native-environment-targets-20261104/evidence.json','outputs/anadius-owned-environment-source/response-executor-native-20261104/evidence.json'],'report':proof(stage/'report.md'),'implementation_reports':[proof(stage/'environment-implementation.md'),proof(stage.parent/'tracked-executor-design-20261104/bridge-implementation.md'),proof(stage/'environment-test-source-notes.md')],'independent_reviews':[proof(stage/n) for n in review_names],'source_changes':[{**proof(out/p),'before':proof(before[p])} for p in paths],'production_files_changed':len(production),'test_files_changed':len(tests),'pending_source_count':len(changes),'pending_sources':changes,'remaining_limits':limits,'compile':False,'tests_run':False,'link':False,'DLL_execution':False,'ZIP_updated':False,'original_unchanged':True,'frozen_build_and_delivery':frozen,'overall_completion_percentage':None,'continuous_goal_active':True}
write(stage/'evidence.json',result);current=proof(stage/'evidence.json')
for n in ['blockers.json','validation.json','static-source-milestone-20261104.json']:shutil.copyfile(critical/n,stage/'before'/n)
p=read(critical/'pending-source-status.json');p.setdefault('source_evidence_history',[]).append(p['source_evidence']);p.update(recorded_at=now,status=status,source_evidence=current,changed_sources_since_last_full_build=[r['path'] for r in changes],current_source_matches_last_full_build=False,compilation_tests_link_execution_performed=False,latest_source_review=current);p['last_verified_common_build']['scope']=f'Historical successful full build;{len(changes)} pending source/header changes require a consistent explicitly authorized rebuild.';write(critical/'pending-source-status.json',p)
plan=read(critical/'active-work-plan.json');plan.update(current_source_matches_last_full_build=False,last_code_result={**current,'status':status,'scope':scope},current_production_sources_changed=True,pending_source_status_proof=proof(critical/'pending-source-status.json'),latest_native_audit=current)
plan['completed_now'].append(f'SOURCE_ONLY full concrete Initial/Tracked family now connected through actual factories/copy/move/prepare/response/typedhandler dispatch and Runtime/Profile/Policy immediate original/child lastrelease.8freshREA records exported/closed;12production+3test sources independently reviewed,NOTRUN.{len(changes)}pending.No build/link/package. Previous default-only deferral superseded.')
plan['next_native_code_target']={'scope':'Preserve full reconstruction objective. Next source validation task is prepared-backed actual child last-release and failed-post Runtime latch fixture, then refresh the full native-function/source coverage map to select remaining concrete behavior gaps. Source presence/default-route success does not prove whole DLL equivalence.','validation':f'{len(changes)} pending source/header changes. No compile/project tests/link/DLL execution/ZIP update without explicit instruction.'}
plan['user_required_REA_workflow']['current_fresh_request']={'status':'EVIDENCE_EXPORTED_SESSION_CLOSED','path':stage.relative_to(root).as_posix(),'evidence':[r['evidence_id'] for r in native]}
plan['user_required_REA_workflow']['delegation_policy']['current_tasks']=['tracked-family-source-and-cross-review','runtime-policy-work-finish-bridge-and-cross-review','tracked-family-test-source']
plan['user_required_REA_workflow']['latest_observation']='Fresh0229a0 atomicdrop/old1stop,02a190 orderedflags/immediatepost/errorcategory,023580/023360retains and0231b0move; source families and actor-bound stop connected. Static only.'
write(critical/'active-work-plan.json',plan)
b=read(critical/'blockers.json');b.update(pending_source_delta={'status':status,'count':len(changes),'new_files':sorted(set(paths)-set(prior['changed_sources_since_last_full_build'])),'changed_sources':[r['path'] for r in changes]},latest_source_stage=current);write(critical/'blockers.json',b)
v=read(critical/'validation.json');v.update(result=status,scope=scope,current_change_checks_executed=False,current_source_matches_last_full_build=False,pending_source_changes=[r['path'] for r in changes],latest_source_stage=current,current_stage_full_build_or_link_run=False);write(critical/'validation.json',v)
m=read(critical/'static-source-milestone-20261104.json');m.update(recorded_at=now,pending_source_count=len(changes),latest_source_milestone=current,tracked_family_decision='Previous default-only deferral superseded: concrete Initial/Tracked family source and original/child immediate work-stop route implemented and statically reviewed. Full objective active; native ABI/null/EH and real child integration remain unvalidated.',independent_static_work_in_progress='Prepared-backed actual child and Runtime error-latch integration source coverage, followed by full coverage-map gap selection.',remaining_validation_limits=limits,current_validation_gate={'reason':'Explicit rebuild required for compiler/test/link evidence. Further full-objective source integration/coverage work remains useful and active.','consecutive_goal_turns_at_impasse':0,'no_repeat_question':True});write(critical/'static-source-milestone-20261104.json',m)
plan=read(critical/'active-work-plan.json');plan['current_static_milestone']=proof(critical/'static-source-milestone-20261104.json');write(critical/'active-work-plan.json',plan)
assert frozen==[proof(out/p) for p in frozen_paths]
write(stage/'metadata-result.json',{'pending_source_count':len(changes),'frozen_build_and_delivery_unchanged':True,'updated_records':[proof(critical/n) for n in ['pending-source-status.json','active-work-plan.json','blockers.json','validation.json','static-source-milestone-20261104.json']]})
print(json.dumps({'status':status,'fresh_REA_records':len(native),'production_changes':len(production),'test_changes':len(tests),'pending_sources':len(changes),'frozen_DLL_ZIP_unchanged':True,'continuous_goal_active':True}))
