"""Record this source-only stage; no compiler, project test or target execution."""
import hashlib
import json
import shutil
from datetime import datetime, timezone
from pathlib import Path

root = Path(__file__).resolve().parents[3]
stage = Path(__file__).resolve().parent
out = root / 'outputs'
critical = out / 'dll-critical-path'

def read(p):
    return json.loads(p.read_text(encoding='utf-8-sig'))

def sha(p):
    return hashlib.file_digest(p.open('rb'), 'sha256').hexdigest()

def write(p, value):
    p.write_text(json.dumps(value, indent=2, ensure_ascii=True) + '\n', encoding='utf-8')

def proof(p):
    return {'path': p.relative_to(root).as_posix(), 'sha256': sha(p)}

assert not (stage / 'evidence.json').exists(), 'Stage already recorded'
snapshot = read(out / 'source-workspace/source-snapshot.json')
changes = []
for row in snapshot['sources'] + snapshot.get('additional_dependency_headers', []):
    path = out / row['path']
    current = sha(path)
    if current != row['sha256']:
        changes.append({'path': row['path'], 'build_sha256': row['sha256'], 'current_sha256': current})
assert len(changes) == 1 and changes[0]['path'] == 'anadius-registry-source/registry.cpp', changes
assert sha(stage / 'registry.cpp.before.txt') == changes[0]['build_sha256']
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
assert (stage / 'review.md').exists()
evidence = {'schema': 'registry-language-lifetime.v1', 'client_date': '2026-11-04', 'recorded_at': datetime.now(timezone.utc).isoformat(), 'status': 'SOURCE_IMPLEMENTED_STATIC_REVIEWED_NOT_COMPILED', 'scope': 'C++ narrow language log temporary retired before A/W replacement writer; no CRT ABI or runtime equivalence claim', 'source_changes': changes, 'pending_source_count': len(changes), 'fresh_REA_evidence': replies, 'independent_static_review': proof(stage / 'review.md'), 'report': proof(stage / 'report.md'), 'native_session_closed': True, 'compile': False, 'project_tests': False, 'link': False, 'DLL_execution': False, 'ZIP_updated': False, 'original_unchanged': True, 'frozen_proofs_and_delivery': frozen, 'overall_completion_percentage': None, 'new_native_body_recoveries': 0}
write(stage / 'evidence.json', evidence)
current = proof(stage / 'evidence.json')
p = read(critical / 'pending-source-status.json')
p.setdefault('source_evidence_history', []).append(p['source_evidence'])
p.update(recorded_at=evidence['recorded_at'], status=evidence['status'], changed_sources_since_last_full_build=[x['path'] for x in changes], source_evidence=current, current_source_matches_last_full_build=False, compilation_tests_link_execution_performed=False, latest_source_review=current)
p['last_verified_common_build']['scope'] = 'Historical successful full build; current registry.cpp differs in the source-only stage recorded here.'
write(critical / 'pending-source-status.json', p)
plan = read(critical / 'active-work-plan.json')
plan.update(current_source_matches_last_full_build=False, last_code_result={**current, 'status': evidence['status'], 'scope': evidence['scope']}, current_production_sources_changed=True, pending_source_status_proof=proof(critical / 'pending-source-status.json'))
plan['completed_now'].append('SOURCE_ONLY fresh REA/Ghidra009520/009850 instruction review: retire narrow language logging temporary before replacement writer, independent static review; one source differs from last successful full build, no compile/test/link/package.')
plan['next_native_code_target'] = {'scope': 'Review early general registry admission during per-hook publication against synchronous native startup. Close/query and nonnull general bodies remain exclusive. Establish a concrete discrepancy before source changes; preserve original startup/detach and no added unload requirement.', 'validation': 'One canonical production source pending since successful full build/link. Static source continuation only; explicit rebuild required.'}
plan['user_required_REA_workflow']['current_fresh_request'] = {'status': 'EVIDENCE_EXPORTED_SESSION_CLOSED', 'path': stage.relative_to(root).as_posix(), 'evidence': [r['evidence_id'] for r in replies]}
write(critical / 'active-work-plan.json', plan)
for name in ['blockers.json', 'validation.json']:
    shutil.copyfile(critical / name, stage / (name.removesuffix('.json') + '.before.json'))
b = read(critical / 'blockers.json')
b.update(pending_source_delta={'status': evidence['status'], 'count': 1, 'new_files': [], 'changed_sources': [x['path'] for x in changes]}, latest_source_stage=current)
write(critical / 'blockers.json', b)
v = read(critical / 'validation.json')
v.update(result=evidence['status'], scope=evidence['scope'], current_change_checks_executed=False, current_source_matches_last_full_build=False, pending_source_changes=[x['path'] for x in changes], latest_source_stage=current, current_stage_full_build_or_link_run=False)
write(critical / 'validation.json', v)
assert frozen == [proof(p) for p in frozen_paths]
write(stage / 'metadata-result.json', {'pending_source_count': 1, 'frozen_proofs_and_delivery_unchanged': True, 'updated_records': [proof(critical / n) for n in ['pending-source-status.json', 'active-work-plan.json', 'blockers.json', 'validation.json']]})
print(json.dumps({'status': evidence['status'], 'pending_sources': 1, 'frozen_delivery_unchanged': True}))
