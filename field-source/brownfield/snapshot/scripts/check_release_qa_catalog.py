#!/usr/bin/env python3
"""Release QA catalog structural validator; no G0 override and no skipped debt."""
from pathlib import Path
import argparse,hashlib,json,sys,re
ROOT=Path(__file__).resolve().parents[1]
PATH=ROOT/'metadata/RELEASE_QA_CATALOG.json'
SHA=re.compile(r'[0-9a-f]{64}\Z')

def verify(path=PATH, pin=None):
    errors=[]
    raw=path.read_bytes()
    digest=hashlib.sha256(raw).hexdigest()
    if pin is not None and (not SHA.fullmatch(pin) or pin!=digest):errors.append('CATALOG_EXTERNAL_PIN_MISMATCH')
    d=json.loads(raw)
    if d.get('schema')!='universal-release-qa-catalog.v1' or d.get('shard_count')!=8:errors.append('SCHEMA_OR_SHARDS')
    seq=d.get('steps',[]); names=[x.get('script','') for x in seq]
    if len(names)!=len(set(names)) or not names:errors.append('DUPLICATE_OR_EMPTY')
    if any(not x.get('retain_in_authoritative_universe') for x in seq):errors.append('DISABLED_STEP')
    ordered_hash=hashlib.sha256(('\n'.join(names)+'\n').encode()).hexdigest()
    if d.get('inherited_step_count')!=len(names) or d.get('inherited_step_order_sha256')!=ordered_hash or d.get('inherited_lineage_role')!='SOURCE_ONLY_UNREVIEWED_HISTORICAL_AUTHORITATIVE_SET':errors.append('INHERITED_UNIVERSE_ORDER_OR_COUNT')
    if d.get('behavioral_step')!='test_regressions.py' or names.count('test_regressions.py')!=1:errors.append('BEHAVIORAL_STEP')
    if 'test_unit_syntax.py' not in names:errors.append('UNIT_RUNTIME_STEP')
    if any(not (ROOT/'scripts'/name).is_file() or name.startswith('.') or '/' in name or '\\' in name for name in names):errors.append('MISSING_OR_INVALID_SCRIPT')
    # Active release contract must remain version-independent across HOTFIX ordinals.
    # Historical release-specific scripts remain on disk for provenance only.
    active_release=[x for x in names if x=='check_active_release_contract.py']
    if len(active_release)!=1 or 'check_release_contract.py' in names or any(re.fullmatch(r'check_hf[0-9]+_release_contract\.py',n) for n in names):
        errors.append('ACTIVE_RELEASE_CONTRACT_MUST_BE_VERSION_INDEPENDENT')
    pending=[x['script'] for x in seq if x.get('migration')!='GENERAL_CHECK']
    if set(pending)!=set(d.get('review_blockers',[])):errors.append('MIGRATION_REVIEW_MISMATCH')
    if d.get('release_identity') is not None or d.get('scope')!='IDS_QA_ONLY_NOT_AUTHORIZED':errors.append('UNAUTHORIZED_RELEASE_SCOPE')
    return {'schema':'universal-release-qa-catalog-check.v1','status':'PASS_STRUCTURE_ONLY' if not errors else 'FAIL','errors':errors,'catalog_sha256':digest,'step_count':len(names),'unreviewed_historical':pending,'g0_authorized':False,'release_qa_authorized':False}

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--trusted-catalog-sha');ap.add_argument('--report',type=Path);a=ap.parse_args()
    try:r=verify(pin=a.trusted_catalog_sha)
    except Exception as e:r={'status':'FAIL','errors':[type(e).__name__],'release_qa_authorized':False}
    if a.report:a.report.write_text(json.dumps(r,sort_keys=True,indent=2)+'\n')
    print('UNIVERSAL_QA_CATALOG:',r['status'],'unreviewed',len(r.get('unreviewed_historical',[])))
    return 0 if r['status']=='PASS_STRUCTURE_ONLY' else 1
if __name__=='__main__':sys.exit(main())
