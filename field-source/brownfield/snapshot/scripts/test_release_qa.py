#!/usr/bin/env python3
"""Version-independent eight-shard release QA runner. G0 mandatory; fail closed.

Historical test migrations MUST be reviewed before any release shard executes.
No test is silently dropped from the inherited authoritative universe.
"""
import hashlib,json,os,subprocess,sys
from pathlib import Path
os.environ['PYTHONDONTWRITEBYTECODE']='1';sys.dont_write_bytecode=True
r=Path(__file__).resolve().parents[1]
# G0 has already run once in an independent, externally pinned context.
# Each shard verifies only the pinned G0 receipt + same source/catalog bytes.
from check_release_qa_authority import verify as verify_authority
_authority = verify_authority()
if _authority['status'] != 'PASS_PREPUBLICATION_AUTH_ONLY':
    print('RELEASE_QA: BLOCKED missing/mismatched independently pinned G0 receipt', ','.join(_authority['blockers']))
    sys.exit(3)
catalog_path=r/'metadata/RELEASE_QA_CATALOG.json'
plan=json.loads(catalog_path.read_bytes())
# The catalog remains permanently SOURCE_ONLY / NOT_AUTHORIZED; only the
# externally trusted G0 receipt can grant prepublication QA access.
if plan.get('schema')!='universal-release-qa-catalog.v1' or plan.get('scope')!='IDS_QA_ONLY_NOT_AUTHORIZED':
    print('RELEASE_QA: BLOCKED mutated source-only QA catalog');sys.exit(3)
if plan.get('review_blockers') or any(item['migration']!='GENERAL_CHECK' for item in plan.get('steps',[])):
    print('RELEASE_QA: BLOCKED historical regressions require explicit review');sys.exit(3)
steps=[x['script'] for x in plan['steps']]
if len(steps)!=len(set(steps)) or 'test_unit_syntax.py' not in steps or steps.count('test_regressions.py')!=1:
    print('RELEASE_QA: FAIL_CLOSED missing mandatory gates');sys.exit(3)
AUTHORITATIVE_SHARD_COUNT=8
qa_shard_index=None
qa_shard_count=None
has_index='--shard-index' in sys.argv
has_count='--shard-count' in sys.argv
if not has_index and not has_count:
    print('UNIVERSAL_RELEASE_SOURCE_QA: FAIL - authoritative release QA requires exactly 8 deterministic shards; invoke --shard-count 8 --shard-index 0..7');sys.exit(2)
if has_index != has_count:print('UNIVERSAL_RELEASE_SOURCE_QA: FAIL incomplete shard args');sys.exit(2)
try:
    qa_shard_index=int(sys.argv[sys.argv.index('--shard-index')+1]);qa_shard_count=int(sys.argv[sys.argv.index('--shard-count')+1])
except (ValueError,IndexError):print('UNIVERSAL_RELEASE_SOURCE_QA: FAIL invalid shard args');sys.exit(2)
if qa_shard_count != AUTHORITATIVE_SHARD_COUNT:
    print('UNIVERSAL_RELEASE_SOURCE_QA: FAIL shard count != required 8');sys.exit(2)
if qa_shard_index<0 or qa_shard_index>=qa_shard_count:
    print('UNIVERSAL_RELEASE_SOURCE_QA: FAIL shard range');sys.exit(2)
behavior='test_regressions.py'
normal_steps=[s for s in steps if s!=behavior]
selected=[s for i,s in enumerate(normal_steps) if i % AUTHORITATIVE_SHARD_COUNT == qa_shard_index]
if '--authority-preflight' in sys.argv:
    print('RELEASE_QA: PASS_PREPUBLICATION_AUTH_ONLY; NO_SHARD_EXECUTED; NOT_G4')
    sys.exit(0)
for s in selected:
    args=[sys.executable,'-B',str(r/'scripts'/s)]
    if s=='check_crt_transition.py':args.append('--source')
    subprocess.run(args,check=True)
subprocess.run([sys.executable,str(r/'scripts'/behavior),'--shard-index',str(qa_shard_index),'--shard-count',str(AUTHORITATIVE_SHARD_COUNT)],check=True)
print(f'UNIVERSAL_RELEASE_SOURCE_QA_SHARD: PASS ({qa_shard_index+1}/{AUTHORITATIVE_SHARD_COUNT})')
