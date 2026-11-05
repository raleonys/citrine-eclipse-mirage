"""Source/evidence bookkeeping only; never compile, test, load or package."""
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
rows = snapshot['sources'] + snapshot.get('additional_dependency_headers', [])
changes = [{'path': r['path'], 'build_sha256': r['sha256'], 'current_sha256': sha(out / r['path'])} for r in rows if sha(out / r['path']) != r['sha256']]
current_paths = ['anadius-native-source/native_callbacks.hpp', 'anadius-native-source/native_callbacks.cpp', 'anadius-native-context-source/context.cpp', 'anadius-foreign-callback-source/foreign.hpp', 'anadius-foreign-callback-source/foreign.cpp', 'anadius-foreign-callback-source/foreign_tests.cpp', 'anadius-foreign-callback-source/foreign_integration_tests.cpp']
expected = set(current_paths + ['anadius-registry-source/registry.cpp', 'anadius-resources-factory-source/factory_tests.cpp'])
assert {row['path'] for row in changes} == expected, changes
by_path = {r['path']: r for r in rows}
for path in current_paths:
    assert sha(stage / (Path(path).name + '.before.txt')) == by_path[path]['sha256'], path
assert (stage / 'independent-review.md').exists()
assert (stage / 'test-source-notes.md').exists()
review_text = (stage / 'independent-review.md').read_text(encoding='utf-8-sig')
assert all(sha(out / path) in review_text for path in current_paths), 'Review must identify every final source hash'
frozen_paths = [out / 'source-workspace' / n for n in ['build-record.json', 'source-snapshot.json', 'full-build-verification.json']]
frozen_paths += [out / 'dll-link-gate-source/current-full-link.json', out / 'package-receipt.json', out / 'dll-link-gate-source/build-current-full/anadius64.reconstruction.dll', out / 'dll-reverse-engineering.zip']
frozen = [proof(p) for p in frozen_paths]
assert frozen[-2]['sha256'] == '44bc309127e7cec7db7ed5c62ff6664a22e257b4edf3ed977a0fa8fa109b290b'
assert frozen[-1]['sha256'] == '790b71bbf92d02ff05a391d2985fdab4eaad3ab72f34de29ea51e79be13b267c'
assert sha(Path('C:/Users/Skysect/Downloads/anadius64.dll')) == '9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
replies = []
for path in sorted(stage.glob('*.json')):
    if path.name.startswith(('pseudo-', 'instructions-')):
        envelope = read(path)
        e = envelope.get('structuredContent') or json.loads(envelope['content'][0]['text'])
        assert e['provider']['id'] == 'ghidra' and e['provider']['version'] == '12.1.4'
        replies.append({**proof(path), 'evidence_id': e['evidence_id'], 'operation': e['operation'], 'parameters': e['parameters']})
assert len(replies) == 6
now = datetime.now(timezone.utc).isoformat()
result = {'schema': 'registry-sdk-nesting.v1', 'recorded_at': now, 'client_date': '2026-11-04', 'status': 'SOURCE_IMPLEMENTED_STATIC_REVIEWED_NOT_COMPILED', 'scope': 'Bounded OriginalCrt same-thread registry nesting during exact typed SDK calls; root PTD retained, exact actor restoration; tests added but not run.', 'fresh_REA_evidence': replies, 'report': proof(stage / 'report.md'), 'native_findings': proof(stage / 'native-findings.md'), 'independent_review': proof(stage / 'independent-review.md'), 'test_source_notes': proof(stage / 'test-source-notes.md'), 'domain_source_notes': proof(stage / 'domain-implementation.md'), 'stage_source_changes': [{**proof(out / path), 'before_sha256': sha(stage / (Path(path).name + '.before.txt'))} for path in current_paths], 'production_files_changed': 5, 'test_files_changed': 2, 'pending_source_count': len(changes), 'pending_sources': changes, 'compiled': False, 'tests_executed': False, 'link_attempted': False, 'DLL_executed': False, 'ZIP_updated': False, 'native_session_closed': True, 'original_unchanged': True, 'frozen_build_and_delivery': frozen, 'overall_completion_percentage': None, 'new_normal_native_body_recoveries': 0, 'native_ABI_closures': 0}
write(stage / 'evidence.json', result)
current = proof(stage / 'evidence.json')
p = read(critical / 'pending-source-status.json')
p.setdefault('source_evidence_history', []).append(p['source_evidence'])
p.update(recorded_at=now, status=result['status'], source_evidence=current, changed_sources_since_last_full_build=[x['path'] for x in changes], current_source_matches_last_full_build=False, compilation_tests_link_execution_performed=False, latest_source_review=current)
p['last_verified_common_build']['scope'] = 'Historical successful build; nine current source/header changes including the registry SDK frame/vtable/layout change await consistent rebuild.'
write(critical / 'pending-source-status.json', p)
plan = read(critical / 'active-work-plan.json')
plan.update(current_source_matches_last_full_build=False, last_code_result={**current, 'status': result['status'], 'scope': result['scope']}, current_production_sources_changed=True, pending_source_status_proof=proof(critical / 'pending-source-status.json'))
plan['completed_now'].append('SOURCE_ONLY exact typed-registry SDK-phase markers plus nested same-thread general frames, existing root PTD and actor restoration. Fresh6-record REA evidence, independent static review, recursive typed/test-source cases NOT RUN;5 production and2 test files changed,9 pending canonical sources total; no compiler/link/package.')
plan['next_native_code_target'] = {'scope': 'Review matched registry paths under the new bounded SDK nesting: returned-handle tracking, nested close/reopen and query capacity/output aliases against native rereads. Do not broaden all shared-set concurrency, pure-forward/connect parents or CRT scopes without concrete evidence.', 'validation': 'Nine canonical source/header differences pending; Admission vtable and Effects layout require a consistent rebuild only on explicit instruction. Static source continuation; no automatic goal.'}
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
assert frozen == [proof(p) for p in frozen_paths]
write(stage / 'metadata-result.json', {'pending_sources': len(changes), 'stage_production_changes': 5, 'stage_test_source_changes': 2, 'frozen_build_and_delivery_unchanged': True, 'updated_records': [proof(critical / n) for n in ['pending-source-status.json', 'active-work-plan.json', 'blockers.json', 'validation.json']]})
print(json.dumps({'status': result['status'], 'pending_sources': len(changes), 'production_changes_this_stage': 5, 'tests_run': False, 'DLL_and_ZIP_unchanged': True}))
