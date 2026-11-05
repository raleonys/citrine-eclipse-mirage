"""One-time source bookkeeping only. No project execution or packaging."""
import hashlib, json, shutil
from pathlib import Path
from datetime import datetime, timezone
stage=Path(__file__).resolve().parent; root=stage.parents[2]; out=root/'outputs'; critical=out/'dll-critical-path'
def read(p): return json.loads(p.read_text(encoding='utf-8-sig'))
def sha(p):
    with p.open('rb') as f: return hashlib.file_digest(f,'sha256').hexdigest()
def write(p,v): p.write_text(json.dumps(v,indent=2,ensure_ascii=True)+'\n',encoding='utf-8')
def proof(p): return {'path':p.relative_to(root).as_posix(),'sha256':sha(p)}
assert not (stage/'evidence.json').exists()
prior=read(stage/'pending-source-status.before.json'); snapshot=read(out/'source-workspace/source-snapshot.json')
rows=snapshot['sources']+snapshot.get('additional_dependency_headers',[]); by_path={r['path']:r for r in rows}
paths=['anadius-session-profile-source/profile.cpp','anadius-application-graph-source/graph.cpp','anadius-session-profile-source/profile.hpp','anadius-application-graph-source/graph.hpp','anadius-session-flow-source/coherent_io.hpp','anadius-socket-source/environment_constructor.hpp','anadius-session-profile-source/profile_tests.cpp','anadius-session-profile-source/profile_integration_tests.cpp','anadius-application-graph-source/graph_integration_tests.cpp']
changes=[{'path':r['path'],'build_sha256':r['sha256'],'current_sha256':sha(out/r['path'])} for r in rows if sha(out/r['path'])!=r['sha256']]
assert len(changes)==22 and {r['path'] for r in changes}==set(prior['changed_sources_since_last_full_build'])|set(paths)
old=read(root/prior['source_evidence']['path']); old_pending={r['path']:r['current_sha256'] for r in old['pending_sources']}
for p in paths: assert sha(stage/(Path(p).name+'.before.txt'))==old_pending.get(p,by_path[p]['sha256']),p
prod=(stage/'independent-review.md').read_text(encoding='utf-8-sig'); test=(stage/'test-review.md').read_text(encoding='utf-8-sig')
assert all(sha(out/p) in prod for p in paths[:6]); assert all(sha(out/p) in test for p in paths[6:])
frozen_expected={'source-workspace/build-record.json':'b9b5bd714c6b5744f379961b6ad60e7eb037a00c1677723bf7bcea6a664d4573','source-workspace/source-snapshot.json':'b7362cf518727f6fc997bfc1377bc4d83e2751903ead78cf376251cb0bf87e6d','source-workspace/full-build-verification.json':'6cbded372c77eeb2a35b332dbe75fbc9bc6cfbd24da3b967bfaa03b124087e12','dll-link-gate-source/current-full-link.json':'70d9d9dcbaed784530bee67928fe0ebbe27dbe931b07219545cd6988f50e4755','dll-link-gate-source/build-current-full/anadius64.reconstruction.dll':'44bc309127e7cec7db7ed5c62ff6664a22e257b4edf3ed977a0fa8fa109b290b','dll-reverse-engineering.zip':'790b71bbf92d02ff05a391d2985fdab4eaad3ab72f34de29ea51e79be13b267c'}
for p,h in frozen_expected.items(): assert sha(out/p)==h,p
frozen=[proof(out/p) for p in frozen_expected]+[proof(out/'package-receipt.json')]
assert sha(Path('C:/Users/Skysect/Downloads/anadius64.dll'))=='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
native=[]
for p in sorted(stage.glob('*.json')):
    if p.name.startswith(('pseudo-','instructions-')):
        reply=read(p); e=reply.get('structuredContent') or json.loads(reply['content'][0]['text'])
        assert e['provider']['id']=='ghidra' and e['provider']['version']=='12.1.4'
        native.append({**proof(p),'evidence_id':e['evidence_id'],'operation':e['operation'],'parameters':e['parameters']})
assert len(native)==6
now=datetime.now(timezone.utc).isoformat(); status='SOURCE_IMPLEMENTED_STATIC_REVIEWED_NOT_COMPILED'
scope='Selected Profile session tables and Graph Logic rows grow beyond initial capacity while preserving old session object addresses and republishing internal spans/queued aliases. Twenty-owner core and two-peer actual pipeline regression sources added, not run.'
result={'schema':'profile-session-registry-growth.v1','recorded_at':now,'client_date':'2026-11-04','status':status,'scope':scope,'fresh_REA_evidence':native,'report':proof(stage/'report.md'),'native_findings':proof(stage/'native-findings.md'),'independent_review':proof(stage/'independent-review.md'),'test_review':proof(stage/'test-review.md'),'test_source_notes':proof(stage/'test-source-notes.md'),'source_changes':[{**proof(out/p),'before_sha256':sha(stage/(Path(p).name+'.before.txt'))} for p in paths],'production_files_changed':6,'test_files_changed':3,'pending_source_count':len(changes),'pending_sources':changes,'compile':False,'tests_run':False,'link':False,'DLL_execution':False,'ZIP_updated':False,'original_unchanged':True,'native_session_closed':True,'frozen_build_and_delivery':frozen,'overall_completion_percentage':None,'continuous_goal_active':True}
write(stage/'evidence.json',result); current=proof(stage/'evidence.json')
for name in ['blockers.json','validation.json']: shutil.copyfile(critical/name,stage/(name.removesuffix('.json')+'.before.json'))
p=read(critical/'pending-source-status.json'); p.setdefault('source_evidence_history',[]).append(p['source_evidence']); p.update(recorded_at=now,status=status,source_evidence=current,changed_sources_since_last_full_build=[r['path'] for r in changes],current_source_matches_last_full_build=False,compilation_tests_link_execution_performed=False,latest_source_review=current); p['last_verified_common_build']['scope']='Historical successful full build;22 pending source/header changes require a consistent explicitly authorized rebuild.'; write(critical/'pending-source-status.json',p)
plan=read(critical/'active-work-plan.json'); plan.update(current_source_matches_last_full_build=False,last_code_result={**current,'status':status,'scope':scope},current_production_sources_changed=True,pending_source_status_proof=proof(critical/'pending-source-status.json')); plan['completed_now'].append('SOURCE_ONLY Profile session and Graph Logic registries grow beyond initial16 with stable old objects and coherent internal views/queued aliases.6fresh REA records, independent production/test static reviews;20core owners vs2network peers explicitly distinguished.6production/header+3test sources changed;22pending, no build/link/package.'); plan['next_native_code_target']={'scope':'Bounded source triage of selected Jobs/IO/environment capacities, then fresh native evidence for a concrete behavior discrepancy before source changes.','validation':'22 pending source/header changes. Continuous REA/Ghidra/subagent goal active; no compile/project tests/link/DLL execution/ZIP update without explicit instruction.'}; plan['user_required_REA_workflow']['current_fresh_request']={'status':'EVIDENCE_EXPORTED_SESSION_CLOSED','path':stage.relative_to(root).as_posix(),'evidence':[r['evidence_id'] for r in native]}; write(critical/'active-work-plan.json',plan)
b=read(critical/'blockers.json'); b.update(pending_source_delta={'status':status,'count':len(changes),'new_files':[],'changed_sources':[r['path'] for r in changes]},latest_source_stage=current); write(critical/'blockers.json',b)
v=read(critical/'validation.json'); v.update(result=status,scope=scope,current_change_checks_executed=False,current_source_matches_last_full_build=False,pending_source_changes=[r['path'] for r in changes],latest_source_stage=current,current_stage_full_build_or_link_run=False); write(critical/'validation.json',v)
assert frozen==[proof(out/p) for p in frozen_expected]+[proof(out/'package-receipt.json')]
write(stage/'metadata-result.json',{'pending_source_count':len(changes),'frozen_build_and_delivery_unchanged':True,'updated_records':[proof(critical/n) for n in ['pending-source-status.json','active-work-plan.json','blockers.json','validation.json']]})
print(json.dumps({'status':status,'pending_sources':len(changes),'production_files_changed':6,'test_files_changed':3,'DLL_and_ZIP_unchanged':True,'continuous_goal_active':True}))
