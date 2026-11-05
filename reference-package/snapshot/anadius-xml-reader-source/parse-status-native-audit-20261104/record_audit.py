"""Record a closed native candidate; no project code execution."""
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
native=[]
for p in sorted(stage.glob('*.json')):
    if p.name.startswith(('pseudo-','instructions-')):
        r=read(p);e=r.get('structuredContent') or json.loads(r['content'][0]['text']);assert e['provider']['id']=='ghidra' and e['provider']['version']=='12.1.4';assert e['subject']['digest']['sha256']=='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';native.append({**proof(p),'evidence_id':e['evidence_id'],'parameters':e['parameters']})
assert len(native)==12 and len(list(stage.glob('error-pseudo-*.json')))==4
paths=['anadius-xml-reader-source/reader.cpp','anadius-xml-reader-source/reader.hpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.h','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp','anadius-native-profile-source/native.cpp','anadius-socket-source/native_message.cpp','anadius-messages-source/messages.cpp'];report=(stage/'report.md').read_text(encoding='utf-8-sig')
assert all(sha(out/p) in report for p in paths)
now=datetime.now(timezone.utc).isoformat();status='CANDIDATE_CLOSED_NO_PRODUCTION_CHANGE'
write(stage/'evidence.json',{'schema':'xml-parse-status-native-audit.v1','recorded_at':now,'status':status,'scope':'Fresh native numeric7attribute and8text producers and033d70 protocol consumers match current pinned parser/source; malformed-input universal equivalence and embedded release remain unproven.','fresh_REA_evidence':native,'report':proof(stage/'report.md'),'reviewed_unchanged_sources':[proof(out/p) for p in paths],'production_changes_for_this_audit':0,'initial_isolation_errors_preserved':4,'native_session_closed':True,'compile':False,'tests_run':False,'link':False,'DLL_execution':False,'ZIP_updated':False,'other_work_note':'Root IO registry source implementation is a separate concurrent stage; this audit does not freeze or validate those changed files.','overall_completion_percentage':None})
current=proof(stage/'evidence.json');p=out/'dll-critical-path/active-work-plan.json';plan=read(p);plan.setdefault('audit_history',[]).append({**current,'status':status,'scope':'XML7/8 producer and protocol routing candidate closed without source change;12 fresh native records.'});plan['latest_native_audit']=current;write(p,plan);print(json.dumps({'status':status,'native_records':12,'production_changes':0}))
