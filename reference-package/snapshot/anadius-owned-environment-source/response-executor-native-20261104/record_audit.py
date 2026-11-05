"""Record a bounded native/source agreement; no project execution."""
import hashlib,json
from pathlib import Path
from datetime import datetime,timezone
stage=Path(__file__).resolve().parent;root=stage.parents[2];out=root/'outputs'
def sha(p):
    with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,v):p.write_text(json.dumps(v,indent=2,ensure_ascii=True)+'\n',encoding='utf-8')
def proof(p):return {'path':p.relative_to(root).as_posix(),'sha256':sha(p)}
assert not (stage/'evidence.json').exists()
native=[];errors=[]
for p in sorted(stage.glob('*.json')):
    if p.name.startswith(('pseudo-','bytes-','instructions-')):
        r=read(p);e=r.get('structuredContent') or json.loads(r['content'][0]['text'])
        if e.get('evidence_id'):
            assert e['provider']['id']=='ghidra' and e['provider']['version']=='12.1.4';assert e['subject']['digest']['sha256']=='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';native.append({**proof(p),'evidence_id':e['evidence_id'],'parameters':e['parameters']})
        else:errors.append(proof(p))
assert len(native)==11 and len(errors)==1
paths=['anadius-owned-environment-source/environment.cpp','anadius-socket-source/response_dispatch.cpp','anadius-socket-source/owned_response.cpp','anadius-socket-source/posted_operation.cpp','anadius-socket-source/completion_dispatcher.cpp']
now=datetime.now(timezone.utc).isoformat();status='CANDIDATE_CLOSED_NO_PRODUCTION_CHANGE'
write(stage/'evidence.json',{'schema':'response-executor-native-audit.v1','recorded_at':now,'status':status,'scope':'Initial native response factories produce(flags0|1)&~2=1, useowned022f90→022e40→028ca0 with one postedwork pin, matching selected hostforcepost1. Arbitrary tracked-family inputs/work retains remain outside equivalence claim.','fresh_REA_evidence':native,'tiny_stub_pseudo_errors':errors,'native_findings':proof(stage/'native-findings.md'),'source_flow_audit':proof(stage/'source-flow-audit.md'),'reviewed_unchanged_sources':[proof(out/p) for p in paths],'production_changes_for_this_audit':0,'native_session_closed':True,'compile':False,'tests_run':False,'link':False,'DLL_execution':False,'ZIP_updated':False,'overall_completion_percentage':None})
current=proof(stage/'evidence.json');p=out/'dll-critical-path/active-work-plan.json';plan=read(p);plan.setdefault('audit_history',[]).append({**current,'status':status,'scope':'Initial response factories20/88 and actualforcepost/oneenqueueworkpin matchsource;11freshnative records, no sourcepatch.'});plan['latest_native_audit']=current;write(p,plan);print(json.dumps({'status':status,'native_records':11,'production_changes':0}))
