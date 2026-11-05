"""Record source/evidence/status only. No project execution or packaging."""
import hashlib, json, shutil
from datetime import datetime, timezone
from pathlib import Path
stage=Path(__file__).resolve().parent;root=stage.parents[2];out=root/'outputs';critical=out/'dll-critical-path'
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def sha(p):
    with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def write(p,v):p.write_text(json.dumps(v,indent=2,ensure_ascii=True)+'\n',encoding='utf-8')
def proof(p):return {'path':p.relative_to(root).as_posix(),'sha256':sha(p)}
assert not (stage/'evidence.json').exists()
prior=read(stage/'pending-source-status.before.json');snapshot=read(out/'source-workspace/source-snapshot.json');rows=snapshot['sources']+snapshot.get('additional_dependency_headers',[])
changes=[]
for row in rows:
    current=sha(out/row['path'])
    if current!=row['sha256']:changes.append({'path':row['path'],'build_sha256':row['sha256'],'current_sha256':current})
paths=['anadius-application-graph-source/graph.cpp','anadius-application-graph-source/graph.hpp','anadius-application-graph-source/graph_tests.cpp']
assert len(changes)==13 and {r['path'] for r in changes}==set(prior['changed_sources_since_last_full_build'])|set(paths),changes
by_path={r['path']:r for r in rows};review=(stage/'independent-review.md').read_text(encoding='utf-8-sig')
for path in paths:
    assert sha(out/path) in review
    assert sha(stage/(Path(path).name+'.before.txt'))==by_path[path]['sha256']
assert sha(out/'anadius-application-graph-source/graph_integration_tests.cpp')==sha(stage/'graph_integration_tests.cpp.before.txt')
frozen_paths=[out/'source-workspace'/n for n in ['build-record.json','source-snapshot.json','full-build-verification.json']]+[out/'dll-link-gate-source/current-full-link.json',out/'package-receipt.json',out/'dll-link-gate-source/build-current-full/anadius64.reconstruction.dll',out/'dll-reverse-engineering.zip']
frozen=[proof(p) for p in frozen_paths]
assert frozen[-2]['sha256']=='44bc309127e7cec7db7ed5c62ff6664a22e257b4edf3ed977a0fa8fa109b290b'
assert frozen[-1]['sha256']=='790b71bbf92d02ff05a391d2985fdab4eaad3ab72f34de29ea51e79be13b267c'
assert sha(Path('C:/Users/Skysect/Downloads/anadius64.dll'))=='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
native=[]
for p in sorted(stage.glob('*.json')):
    if p.name.startswith(('pseudo-','instructions-')):
        reply=read(p);e=reply.get('structuredContent') or json.loads(reply['content'][0]['text'])
        assert e['provider']['id']=='ghidra' and e['provider']['version']=='12.1.4'
        native.append({**proof(p),'evidence_id':e['evidence_id'],'operation':e['operation'],'parameters':e['parameters']})
assert len(native)==7
now=datetime.now(timezone.utc).isoformat();status='SOURCE_IMPLEMENTED_STATIC_REVIEWED_NOT_COMPILED';scope='Actual Graph event queue grows at native one-spare-slot threshold; preserves wrapped FIFO and Parent payload ownership. Two regressions added but not run.'
result={'schema':'application-event-queue-growth.v1','recorded_at':now,'client_date':'2026-11-04','status':status,'scope':scope,'fresh_REA_evidence':native,'report':proof(stage/'report.md'),'native_findings':proof(stage/'native-findings.md'),'independent_review':proof(stage/'independent-review.md'),'implementation':proof(stage/'graph-implementation.md'),'test_source_notes':proof(stage/'test-source-notes.md'),'source_changes':[{**proof(out/p),'before_sha256':sha(stage/(Path(p).name+'.before.txt'))} for p in paths],'production_files_changed':2,'test_files_changed':1,'pending_source_count':len(changes),'pending_sources':changes,'compile':False,'tests_run':False,'link':False,'DLL_execution':False,'ZIP_updated':False,'original_unchanged':True,'native_session_closed':True,'frozen_build_and_delivery':frozen,'overall_completion_percentage':None,'new_normal_native_body_recoveries':0,'closed_source_behavior_gap':'Artificial64-event limit in actual production Graph removed; selected projection limits remain.','continuous_goal_active':True}
write(stage/'evidence.json',result);current=proof(stage/'evidence.json')
p=read(critical/'pending-source-status.json');p.setdefault('source_evidence_history',[]).append(p['source_evidence']);p.update(recorded_at=now,status=status,source_evidence=current,changed_sources_since_last_full_build=[r['path'] for r in changes],current_source_matches_last_full_build=False,compilation_tests_link_execution_performed=False,latest_source_review=current);p['last_verified_common_build']['scope']='Historical successful full build;13 pending source/header changes have not been consistently rebuilt.';write(critical/'pending-source-status.json',p)
plan=read(critical/'active-work-plan.json');plan.update(current_source_matches_last_full_build=False,last_code_result={**current,'status':status,'scope':scope},current_production_sources_changed=True,pending_source_status_proof=proof(critical/'pending-source-status.json'));plan['completed_now'].append('SOURCE_ONLY actual production Graph event queue now grows with native034660 spare-slot/min8doubling rule. Fresh7-record REA evidence, independent static review;70plainIGO and wrapped encryptedFIFO regression sources NOT RUN.2 production+1test files changed;13pending canonical sources total, no compile/link/package.')
plan['next_native_code_target']={'scope':'Fresh native check of Config2 duplicate child assignment012bf0/012cf0 and actual assignment helpers against parser::assign -> config_hash::from_parser. Determine whether native move steals fresh child table or retains old hash capacity/history before making a production change. Candidate only, not a confirmed missing algorithm.','validation':'13 pending source/header changes. Continuous REA/Ghidra/subagent goal active; no compile/project tests/link/DLL execution/ZIP update without explicit instruction.'};plan['user_required_REA_workflow']['current_fresh_request']={'status':'EVIDENCE_EXPORTED_SESSION_CLOSED','path':stage.relative_to(root).as_posix(),'evidence':[r['evidence_id'] for r in native]};write(critical/'active-work-plan.json',plan)
for name in ['blockers.json','validation.json']:shutil.copyfile(critical/name,stage/(name.removesuffix('.json')+'.before.json'))
b=read(critical/'blockers.json');b.update(pending_source_delta={'status':status,'count':len(changes),'new_files':[],'changed_sources':[r['path'] for r in changes]},latest_source_stage=current);write(critical/'blockers.json',b)
v=read(critical/'validation.json');v.update(result=status,scope=scope,current_change_checks_executed=False,current_source_matches_last_full_build=False,pending_source_changes=[r['path'] for r in changes],latest_source_stage=current,current_stage_full_build_or_link_run=False);write(critical/'validation.json',v)
assert frozen==[proof(p) for p in frozen_paths]
write(stage/'metadata-result.json',{'pending_source_count':len(changes),'frozen_build_and_delivery_unchanged':True,'updated_records':[proof(critical/n) for n in ['pending-source-status.json','active-work-plan.json','blockers.json','validation.json']]});print(json.dumps({'status':status,'pending_sources':len(changes),'production_files_changed':2,'test_files_changed':1,'DLL_and_ZIP_unchanged':True,'continuous_goal_active':True}))
