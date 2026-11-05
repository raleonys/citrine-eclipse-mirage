"""Record only source hashes/evidence/status. No project execution."""
import hashlib
import json
import shutil
from datetime import datetime, timezone
from pathlib import Path

stage = Path(__file__).resolve().parent
root = stage.parents[2]
out = root / 'outputs'
critical = out / 'dll-critical-path'

def read(p):
    return json.loads(p.read_text(encoding='utf-8-sig'))

def sha(p):
    with p.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()

def write(p, value):
    p.write_text(json.dumps(value, indent=2, ensure_ascii=True) + '\n', encoding='utf-8')

def proof(p):
    return {'path': p.relative_to(root).as_posix(), 'sha256': sha(p)}

assert not (stage / 'evidence.json').exists()
prior = read(stage / 'pending-source-status.before.json')
snapshot = read(out / 'source-workspace/source-snapshot.json')
changes = []
for row in snapshot['sources'] + snapshot.get('additional_dependency_headers', []):
    current = sha(out / row['path'])
    if current != row['sha256']:
        changes.append({'path': row['path'], 'build_sha256': row['sha256'], 'current_sha256': current})
paths = ['anadius-registry-source/registry_tests.cpp', 'anadius-foreign-callback-source/foreign_integration_tests.cpp']
assert len(changes) == 10 and {x['path'] for x in changes} == set(prior['changed_sources_since_last_full_build']) | set(paths), changes
review = (stage / 'independent-review.md').read_text(encoding='utf-8-sig')
assert all(sha(out / path) in review for path in paths)
frozen_paths = [out / 'source-workspace' / n for n in ['build-record.json', 'source-snapshot.json', 'full-build-verification.json']]
frozen_paths += [out / 'dll-link-gate-source/current-full-link.json', out / 'package-receipt.json', out / 'dll-link-gate-source/build-current-full/anadius64.reconstruction.dll', out / 'dll-reverse-engineering.zip']
frozen = [proof(p) for p in frozen_paths]
assert frozen[-2]['sha256'] == '44bc309127e7cec7db7ed5c62ff6664a22e257b4edf3ed977a0fa8fa109b290b'
assert frozen[-1]['sha256'] == '790b71bbf92d02ff05a391d2985fdab4eaad3ab72f34de29ea51e79be13b267c'
assert sha(Path('C:/Users/Skysect/Downloads/anadius64.dll')) == '9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
native = []
for path in sorted(stage.glob('*.json')):
    if path.name.startswith(('pseudo-', 'instructions-')):
        reply = read(path)
        e = reply.get('structuredContent') or json.loads(reply['content'][0]['text'])
        assert e['provider']['id'] == 'ghidra' and e['provider']['version'] == '12.1.4'
        native.append({**proof(path), 'evidence_id': e['evidence_id'], 'operation': e['operation'], 'parameters': e['parameters']})
assert len(native) == 8
now = datetime.now(timezone.utc).isoformat()
status = 'TEST_SOURCE_ADDED_STATIC_REVIEWED_NOT_RUN'
scope = 'Matched OpenW/close/reopen post-SDK membership and A/W data/type/size alias test sources; no new production discrepancy established.'
result = {'schema': 'registry-matched-alias.v1', 'recorded_at': now, 'client_date': '2026-11-04', 'status': status, 'scope': scope, 'fresh_REA_evidence': native, 'report': proof(stage / 'report.md'), 'independent_review': proof(stage / 'independent-review.md'), 'source_changes': [{**proof(out / path), 'before_sha256': sha(stage / (Path(path).name + '.before.txt'))} for path in paths], 'production_changes': 0, 'test_source_changes': 2, 'pending_source_count': len(changes), 'pending_sources': changes, 'compile': False, 'project_tests_run': False, 'link': False, 'DLL_execution': False, 'ZIP_updated': False, 'original_unchanged': True, 'native_session_closed': True, 'frozen_build_and_delivery': frozen, 'whole_DLL_completion_percentage': None, 'new_native_body_recoveries': 0, 'continuous_user_goal_active': True}
write(stage / 'evidence.json', result)
current = proof(stage / 'evidence.json')
p = read(critical / 'pending-source-status.json')
p.setdefault('source_evidence_history', []).append(p['source_evidence'])
p.update(recorded_at=now, status=status, source_evidence=current, changed_sources_since_last_full_build=[x['path'] for x in changes], current_source_matches_last_full_build=False, compilation_tests_link_execution_performed=False, latest_source_review=current)
p['last_verified_common_build']['scope'] = 'Historical successful full build; ten pending source/header changes have not been consistently rebuilt.'
write(critical / 'pending-source-status.json', p)
plan = read(critical / 'active-work-plan.json')
plan.update(current_source_matches_last_full_build=False, last_code_result={**current, 'status': status, 'scope': scope}, pending_source_status_proof=proof(critical / 'pending-source-status.json'))
plan['completed_now'].append('TEST_SOURCE_ONLY fresh8-record native alias/handle audit; matched OpenW/nestedclose/reopen/capacity16 and A/W triple-output alias cases added/staticreviewed NOT RUN. No production change or percentage credit;10 canonical source changes pending.')
plan['continuous_user_goal'] = {'active': True, 'authority': 'User explicitly requested continuing REA/Ghidra with subagents until they stop the work.', 'explicit_rebuild_still_required': True, 'new_native_stage_requires_fresh_REA': True}
plan['next_native_code_target'] = {'scope': 'Triage current application/configuration/message-handler production gaps against actual selected source, discard stale documentation/library-helper candidates, then obtain fresh native Evidence before the next bounded production correction. Registry matched/alias stage is complete as source coverage; do not repeat unchanged audits.', 'validation': 'Ten pending source/header changes; no compile/project tests/link/DLL execution/ZIP update without explicit instruction. Continuous REA/Ghidra/subagent goal active until user stops or objective achieved.'}
plan['user_required_REA_workflow']['current_fresh_request'] = {'status': 'EVIDENCE_EXPORTED_SESSION_CLOSED', 'path': stage.relative_to(root).as_posix(), 'evidence': [x['evidence_id'] for x in native]}
write(critical / 'active-work-plan.json', plan)
for name in ['blockers.json', 'validation.json']:
    shutil.copyfile(critical / name, stage / (name.removesuffix('.json') + '.before.json'))
b = read(critical / 'blockers.json')
b.update(pending_source_delta={'status': status, 'count': len(changes), 'new_files': [], 'changed_sources': [x['path'] for x in changes]}, latest_source_stage=current)
write(critical / 'blockers.json', b)
v = read(critical / 'validation.json')
v.update(result=status, scope=scope, current_change_checks_executed=False, current_source_matches_last_full_build=False, pending_source_changes=[x['path'] for x in changes], latest_source_stage=current, current_stage_full_build_or_link_run=False)
write(critical / 'validation.json', v)
assert frozen == [proof(p) for p in frozen_paths]
write(stage / 'metadata-result.json', {'pending_source_count': len(changes), 'frozen_build_and_delivery_unchanged': True, 'updated_records': [proof(critical / n) for n in ['pending-source-status.json', 'active-work-plan.json', 'blockers.json', 'validation.json']]})
print(json.dumps({'status': status, 'pending_sources': len(changes), 'production_changes_this_stage': 0, 'DLL_and_ZIP_unchanged': True, 'continuous_goal_active': True}))
