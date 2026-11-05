"""One-time metadata/hash recording only; no project execution."""
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
paths=['anadius-socket-source/posted_operation.hpp','anadius-socket-source/posted_operation.cpp','anadius-socket-source/completion_dispatcher.hpp','anadius-socket-source/completion_dispatcher.cpp','anadius-socket-source/handler_dispatch.cpp','anadius-server-runtime-source/runtime.cpp','anadius-owned-environment-source/environment.cpp','anadius-owned-jobs-source/jobs.cpp','anadius-session-policy-source/policy.cpp','anadius-session-profile-source/profile.cpp','anadius-io-memory-source/io_memory.cpp','anadius-io-memory-source/io_memory.hpp','anadius-accept-source/owned.cpp','anadius-accept-source/owned.hpp','anadius-io-memory-source/io_memory_tests.cpp','anadius-application-graph-source/graph_integration_tests.cpp']
prior=read(stage/'pending-source-status.before.json');old=read(root/prior['source_evidence']['path']);snapshot=read(out/'source-workspace/source-snapshot.json');rows=snapshot['sources']+snapshot.get('additional_dependency_headers',[]);by_path={r['path']:r for r in rows};old_pending={r['path']:r['current_sha256'] for r in old['pending_sources']}
changes=[]
for r in rows:
    h=sha(out/r['path'])
    if h!=r['sha256']:changes.append({'path':r['path'],'build_sha256':r['sha256'],'current_sha256':h})
assert len(changes)==41 and {r['path'] for r in changes}==set(prior['changed_sources_since_last_full_build'])|set(paths)
for p in paths:assert sha(stage/(Path(p).name+'.before.txt'))==old_pending.get(p,by_path[p]['sha256']),p
review_names=['consumer-independent-review.md','production-review.md','root-test-review.md'];reviews=[(stage/n).read_text(encoding='utf-8-sig') for n in review_names]
for i,p in enumerate(paths):assert sha(out/p) in reviews[0 if i<10 else 1 if i<14 else 2],p
frozen_paths=['source-workspace/build-record.json','source-workspace/source-snapshot.json','source-workspace/full-build-verification.json','dll-link-gate-source/current-full-link.json','dll-link-gate-source/build-current-full/anadius64.reconstruction.dll','dll-reverse-engineering.zip','package-receipt.json'];previous_frozen={r['path']:r['sha256'] for r in old['frozen_build_and_delivery']};frozen=[proof(out/p) for p in frozen_paths]
for r in frozen:assert r['sha256']==previous_frozen[r['path']],r
assert sha(Path('C:/Users/Skysect/Downloads/anadius64.dll'))=='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
native=[]
for p in sorted(stage.glob('*.json')):
    if p.name.startswith(('pseudo-','instructions-')):
        reply=read(p);e=reply.get('structuredContent') or json.loads(reply['content'][0]['text']);assert e['provider']['id']=='ghidra' and e['provider']['version']=='12.1.4';native.append({**proof(p),'evidence_id':e['evidence_id'],'operation':e['operation'],'parameters':e['parameters']})
assert len(native)==8
now=datetime.now(timezone.utc).isoformat();status='SOURCE_IMPLEMENTED_STATIC_REVIEWED_NOT_COMPILED';scope='Selected IO/accept registries grow beyond initial64 with stable projections/OVERLAPPEDs, repaired live packet fields and borrowed owner views across copied environments.70records with2SDKloans and initial1 two-peer Runtime regression sources reviewed, NOT RUN.'
result={'schema':'io-accept-registry-growth.v1','recorded_at':now,'client_date':'2026-11-04','status':status,'scope':scope,'fresh_REA_evidence':native,'report':proof(stage/'report.md'),'native_findings':proof(stage/'native-findings.md'),'independent_reviews':[proof(stage/n) for n in review_names],'implementation_notes':[proof(stage/n) for n in ['io-implementation.md','accept-implementation.md','io-consumer-triage.md','io-storage-triage.md']],'source_changes':[{**proof(out/p),'before_sha256':sha(stage/(Path(p).name+'.before.txt'))} for p in paths],'production_files_changed':14,'test_files_changed':2,'pending_source_count':len(changes),'pending_sources':changes,'compile':False,'tests_run':False,'link':False,'DLL_execution':False,'ZIP_updated':False,'original_unchanged':True,'native_session_closed':True,'frozen_build_and_delivery':frozen,'overall_completion_percentage':None,'continuous_goal_active':True}
write(stage/'evidence.json',result);current=proof(stage/'evidence.json')
for n in ['blockers.json','validation.json']:shutil.copyfile(critical/n,stage/(n.removesuffix('.json')+'.before.json'))
p=read(critical/'pending-source-status.json');p.setdefault('source_evidence_history',[]).append(p['source_evidence']);p.update(recorded_at=now,status=status,source_evidence=current,changed_sources_since_last_full_build=[r['path'] for r in changes],current_source_matches_last_full_build=False,compilation_tests_link_execution_performed=False,latest_source_review=current);p['last_verified_common_build']['scope']='Historical successful full build;41 pending source/header changes require a consistent explicitly authorized rebuild.';write(critical/'pending-source-status.json',p)
plan=read(critical/'active-work-plan.json');plan.update(current_source_matches_last_full_build=False,last_code_result={**current,'status':status,'scope':scope},current_production_sources_changed=True,pending_source_status_proof=proof(critical/'pending-source-status.json'));plan['completed_now'].append('SOURCE_ONLY IO/accept registries dynamicallygrow with stable oldrecords/OVERLAPPED/SDKpins,exactpacketaliases andliveownerviews acrosscopiedPorts/transientenvs.8freshREA records,14production+2testdiffs independentlyreviewed;70records/2SDKloans andinitial1Runtime sourcesNOTRUN.41pending.No build/link/package.');plan['next_native_code_target']={'scope':'Bounded fresh native comparison of selected OwnedEnvironment4096 reference-ledger ceiling versus native023dd0 copy/construct paths; design allocation outside its nonrecursive metadata mutex with exact active job lineage and no added readiness/unload framework.','validation':'41 pending source/header changes. Continuous REA/Ghidra/subagent goal active. No compile/project tests/link/DLL execution/ZIP update without explicit instruction.'};plan['user_required_REA_workflow']['current_fresh_request']={'status':'EVIDENCE_EXPORTED_SESSION_CLOSED','path':stage.relative_to(root).as_posix(),'evidence':[r['evidence_id'] for r in native]};write(critical/'active-work-plan.json',plan)
b=read(critical/'blockers.json');b.update(pending_source_delta={'status':status,'count':len(changes),'new_files':[],'changed_sources':[r['path'] for r in changes]},latest_source_stage=current);write(critical/'blockers.json',b)
v=read(critical/'validation.json');v.update(result=status,scope=scope,current_change_checks_executed=False,current_source_matches_last_full_build=False,pending_source_changes=[r['path'] for r in changes],latest_source_stage=current,current_stage_full_build_or_link_run=False);write(critical/'validation.json',v)
assert frozen==[proof(out/p) for p in frozen_paths]
write(stage/'metadata-result.json',{'pending_source_count':len(changes),'frozen_build_and_delivery_unchanged':True,'updated_records':[proof(critical/n) for n in ['pending-source-status.json','active-work-plan.json','blockers.json','validation.json']]})
print(json.dumps({'status':status,'pending_sources':len(changes),'production_files_changed':14,'test_files_changed':2,'DLL_and_ZIP_unchanged':True,'continuous_goal_active':True}))
