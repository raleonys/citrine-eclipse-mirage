"""Source/evidence bookkeeping only. Never compile, test, load or package."""
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

assert not (stage / 'evidence.json').exists(), 'Stage already recorded'
snapshot = read(out / 'source-workspace/source-snapshot.json')
changes = []
for row in snapshot['sources'] + snapshot.get('additional_dependency_headers', []):
    current = sha(out / row['path'])
    if current != row['sha256']:
        changes.append({'path': row['path'], 'build_sha256': row['sha256'], 'current_sha256': current})
expected = {'anadius-registry-source/registry.cpp', 'anadius-resources-factory-source/factory_tests.cpp'}
assert {row['path'] for row in changes} == expected, changes
assert (stage / 'review.md').exists()
frozen_paths = [out / 'source-workspace' / name for name in ['build-record.json', 'source-snapshot.json', 'full-build-verification.json']]
frozen_paths += [out / 'dll-link-gate-source/current-full-link.json', out / 'package-receipt.json', out / 'dll-link-gate-source/build-current-full/anadius64.reconstruction.dll', out / 'dll-reverse-engineering.zip']
frozen = [proof(path) for path in frozen_paths]
assert frozen[-2]['sha256'] == '44bc309127e7cec7db7ed5c62ff6664a22e257b4edf3ed977a0fa8fa109b290b'
assert frozen[-1]['sha256'] == '790b71bbf92d02ff05a391d2985fdab4eaad3ab72f34de29ea51e79be13b267c'
assert sha(Path('C:/Users/Skysect/Downloads/anadius64.dll')) == '9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
replies = []
for path in sorted(stage.glob('*.json')):
    if path.name.startswith(('pseudo-', 'instructions-')):
        envelope = read(path)
        evidence = envelope.get('structuredContent') or json.loads(envelope['content'][0]['text'])
        assert evidence['provider']['id'] == 'ghidra' and evidence['provider']['version'] == '12.1.4'
        replies.append({**proof(path), 'evidence_id': evidence['evidence_id'], 'operation': evidence['operation'], 'parameters': evidence['parameters']})
assert len(replies) == 7
now = datetime.now(timezone.utc).isoformat()
result = {'schema': 'early-registry-publication.v1', 'recorded_at': now, 'client_date': '2026-11-04', 'status': 'TEST_SOURCE_ADDED_STATIC_REVIEWED_NOT_RUN', 'scope': 'Fresh native/source audit confirms existing per-hook early general registry admission. Synthetic actual-host-enable typed callback integration source added; no production readiness change.', 'fresh_REA_evidence': replies, 'report': proof(stage / 'native-audit.md'), 'independent_static_review': proof(stage / 'review.md'), 'pending_source_count': len(changes), 'pending_sources': changes, 'current_stage_changed_sources': ['anadius-resources-factory-source/factory_tests.cpp'], 'production_cpp_changed_this_stage': False, 'compiled': False, 'tests_executed': False, 'link_attempted': False, 'DLL_executed': False, 'ZIP_updated': False, 'native_session_closed': True, 'original_unchanged': True, 'frozen_build_and_delivery': frozen, 'overall_completion_percentage': None, 'new_normal_native_body_recoveries': 0, 'native_ABI_closures': 0}
write(stage / 'evidence.json', result)
current = proof(stage / 'evidence.json')
p = read(critical / 'pending-source-status.json')
p.setdefault('source_evidence_history', []).append(p['source_evidence'])
p.update(recorded_at=now, status=result['status'], source_evidence=current, changed_sources_since_last_full_build=[x['path'] for x in changes], current_source_matches_last_full_build=False, compilation_tests_link_execution_performed=False, latest_source_review=current)
p['last_verified_common_build']['scope'] = 'Historical successful build; current pending source list records all differences. New typed early-registry integration scenarios have not run.'
write(critical / 'pending-source-status.json', p)
plan = read(critical / 'active-work-plan.json')
plan.update(current_source_matches_last_full_build=False, last_code_result={**current, 'status': result['status'], 'scope': result['scope']}, pending_source_status_proof=proof(critical / 'pending-source-status.json'))
plan['completed_now'].append('TEST_SOURCE_ONLY fresh7-record REA/Ghidra pointer-before-enable audit confirms existing early registry admission; actual OriginalCrt install_host typed nonnull RegOpenA enable-time controller/foreign scenarios added and static reviewed, NOT RUN. No production change or percentage credit.')
plan['next_native_code_target'] = {'scope': 'Investigate bounded same-thread nested registry calls while the outer callback is inside its original SDK call, before its shared handle-set body. General overlap remains a separate limit. Establish exact native phase boundaries and actual source permission conflict before changing production; do not add global readiness or unload gates.', 'validation': 'Two canonical source differences pending (one earlier production lifetime fix, one test source). Explicit instruction required before compile/tests/link/execution/ZIP update.'}
plan['user_required_REA_workflow']['current_fresh_request'] = {'status': 'EVIDENCE_EXPORTED_SESSION_CLOSED', 'path': stage.relative_to(root).as_posix(), 'evidence': [x['evidence_id'] for x in replies]}
write(critical / 'active-work-plan.json', plan)
for name in ['blockers.json', 'validation.json']:
    shutil.copyfile(critical / name, stage / (name.removesuffix('.json') + '.before.json'))
b = read(critical / 'blockers.json')
b.update(pending_source_delta={'status': result['status'], 'count': len(changes), 'new_files': [], 'changed_sources': [x['path'] for x in changes]}, latest_source_stage=current)
write(critical / 'blockers.json', b)
v = read(critical / 'validation.json')
v.update(result=result['status'], scope=result['scope'], current_change_checks_executed=False, current_source_matches_last_full_build=False, pending_source_changes=[x['path'] for x in changes], latest_source_stage=current, current_stage_full_build_or_link_run=False)
write(critical / 'validation.json', v)
assert frozen == [proof(path) for path in frozen_paths]
write(stage / 'metadata-result.json', {'pending_sources': len(changes), 'frozen_build_and_delivery_unchanged': True, 'updated_records': [proof(critical / name) for name in ['pending-source-status.json', 'active-work-plan.json', 'blockers.json', 'validation.json']]})
print(json.dumps({'status': result['status'], 'pending_sources': len(changes), 'production_changes_this_stage': 0, 'DLL_and_ZIP_unchanged': True}))
