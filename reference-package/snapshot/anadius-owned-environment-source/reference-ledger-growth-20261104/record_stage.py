"""One-time source/hash bookkeeping only; never execute project code."""
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
paths=['anadius-owned-environment-source/environment.cpp','anadius-owned-environment-source/environment.hpp','anadius-owned-environment-source/environment_tests.cpp','anadius-owned-environment-source/environment_integration_tests.cpp']
prior=read(stage/'pending-source-status.before.json');old=read(root/prior['source_evidence']['path']);snapshot=read(out/'source-workspace/source-snapshot.json');rows=snapshot['sources']+snapshot.get('additional_dependency_headers',[]);by_path={r['path']:r for r in rows};old_pending={r['path']:r['current_sha256'] for r in old['pending_sources']}
changes=[]
for r in rows:
    h=sha(out/r['path'])
    if h!=r['sha256']:changes.append({'path':r['path'],'build_sha256':r['sha256'],'current_sha256':h})
assert len(changes)==44 and {r['path'] for r in changes}==set(prior['changed_sources_since_last_full_build'])|set(paths)
for p in paths:assert sha(stage/(Path(p).name+'.before.txt'))==old_pending.get(p,by_path[p]['sha256']),p
review_names=['production-independent-review.md','test-review.md'];reviews=[(stage/n).read_text(encoding='utf-8-sig') for n in review_names]
for i,p in enumerate(paths):assert sha(out/p) in reviews[i//2],p
frozen_paths=['source-workspace/build-record.json','source-workspace/source-snapshot.json','source-workspace/full-build-verification.json','dll-link-gate-source/current-full-link.json','dll-link-gate-source/build-current-full/anadius64.reconstruction.dll','dll-reverse-engineering.zip','package-receipt.json'];previous_frozen={r['path']:r['sha256'] for r in old['frozen_build_and_delivery']};frozen=[proof(out/p) for p in frozen_paths]
for r in frozen:assert r['sha256']==previous_frozen[r['path']],r
assert sha(Path('C:/Users/Skysect/Downloads/anadius64.dll'))=='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
native=[]
for p in sorted(stage.glob('*.json')):
    if p.name.startswith(('pseudo-','instructions-')):
        reply=read(p);e=reply.get('structuredContent') or json.loads(reply['content'][0]['text']);assert e['provider']['id']=='ghidra' and e['provider']['version']=='12.1.4';native.append({**proof(p),'evidence_id':e['evidence_id'],'operation':e['operation'],'parameters':e['parameters'],'supporting_ledger_path':e['parameters'].get('procedure')!='0x180021770'})
assert len(native)==8 and sum(r['supporting_ledger_path'] for r in native)==7
now=datetime.now(timezone.utc).isoformat();status='SOURCE_IMPLEMENTED_STATIC_REVIEWED_NOT_COMPILED';scope='OwnedEnvironment private reference ledger grows beyond initial4096 with exact source/job authority revalidation and outside-mutex metadata allocation/retirement.4100metadata projections andinitial1two-peer source regressions reviewed, NOT RUN. Native nonempty virtual callees remain unknown.'
result={'schema':'owned-environment-reference-ledger-growth.v1','recorded_at':now,'client_date':'2026-11-04','status':status,'scope':scope,'fresh_REA_evidence':native,'supporting_native_records':7,'incidental_native_records':1,'report':proof(stage/'report.md'),'native_findings':proof(stage/'native-findings.md'),'independent_reviews':[proof(stage/n) for n in review_names],'implementation':proof(stage/'implementation.md'),'design_review':proof(stage/'design-review.md'),'test_source_notes':proof(stage/'test-source-notes.md'),'source_changes':[{**proof(out/p),'before_sha256':sha(stage/(Path(p).name+'.before.txt'))} for p in paths],'production_files_changed':2,'test_files_changed':2,'pending_source_count':len(changes),'pending_sources':changes,'compile':False,'tests_run':False,'link':False,'DLL_execution':False,'ZIP_updated':False,'original_unchanged':True,'native_session_closed':True,'frozen_build_and_delivery':frozen,'overall_completion_percentage':None,'continuous_goal_active':True}
write(stage/'evidence.json',result);current=proof(stage/'evidence.json')
for n in ['blockers.json','validation.json']:shutil.copyfile(critical/n,stage/(n.removesuffix('.json')+'.before.json'))
p=read(critical/'pending-source-status.json');p.setdefault('source_evidence_history',[]).append(p['source_evidence']);p.update(recorded_at=now,status=status,source_evidence=current,changed_sources_since_last_full_build=[r['path'] for r in changes],current_source_matches_last_full_build=False,compilation_tests_link_execution_performed=False,latest_source_review=current);p['last_verified_common_build']['scope']='Historical successful full build;44 pending source/header changes require a consistent explicitly authorized rebuild.';write(critical/'pending-source-status.json',p)
plan=read(critical/'active-work-plan.json');plan.update(current_source_matches_last_full_build=False,last_code_result={**current,'status':status,'scope':scope},current_production_sources_changed=True,pending_source_status_proof=proof(critical/'pending-source-status.json'));plan['completed_now'].append('SOURCE_ONLY OwnedEnvironment private4096ledgercap removed with outsidegateallocation/retirement andsource/protectedjob/capability revalidation.7supporting+1incidentalREA records;independentproduction/testreviews.4100metadata refs/3reuses andinitial1two-peer source regressionsNOTRUN.2production+2tests;44pending.No build/link/package.Nativevirtualcalleesstillunknown.');plan['next_native_code_target']={'scope':'Recover concrete native environment operations/executor origins and virtual copy/prepare targets reached by023dd0/028d30 using focused owner/socket constructor data flow and vtable Evidence. Preserve unresolved-target limits; avoid further generic capacity/CRT framework work without a concrete behavior gap.','validation':'44 pending source/header changes. Continuous REA/Ghidra/subagent goal active. No compile/project tests/link/DLL execution/ZIP update without explicit instruction.'};plan['user_required_REA_workflow']['current_fresh_request']={'status':'EVIDENCE_EXPORTED_SESSION_CLOSED','path':stage.relative_to(root).as_posix(),'evidence':[r['evidence_id'] for r in native]};write(critical/'active-work-plan.json',plan)
b=read(critical/'blockers.json');b.update(pending_source_delta={'status':status,'count':len(changes),'new_files':[],'changed_sources':[r['path'] for r in changes]},latest_source_stage=current);write(critical/'blockers.json',b)
v=read(critical/'validation.json');v.update(result=status,scope=scope,current_change_checks_executed=False,current_source_matches_last_full_build=False,pending_source_changes=[r['path'] for r in changes],latest_source_stage=current,current_stage_full_build_or_link_run=False);write(critical/'validation.json',v)
assert frozen==[proof(out/p) for p in frozen_paths]
write(stage/'metadata-result.json',{'pending_source_count':len(changes),'frozen_build_and_delivery_unchanged':True,'updated_records':[proof(critical/n) for n in ['pending-source-status.json','active-work-plan.json','blockers.json','validation.json']]})
print(json.dumps({'status':status,'pending_sources':len(changes),'production_files_changed':2,'test_files_changed':2,'DLL_and_ZIP_unchanged':True,'continuous_goal_active':True}))
