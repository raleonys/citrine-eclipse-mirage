#!/usr/bin/env python3
"""Validate one external G0 receipt for all release QA shards.

The source QA catalog is permanently *not* an authority object. A trustworthy
receipt must be outside the source and independently SHA-pinned by the caller.
This validates prepublication authorization, NOT source-shard PASS or G4.
"""
import hashlib
import json
import os
import re
import sys
from pathlib import Path
from check_release_qa_catalog import verify as verify_catalog

ROOT = Path(__file__).resolve().parents[1]
SHA = re.compile(r'[0-9a-f]{64}\Z')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def verify(root=ROOT, env=None):
    root = Path(root).resolve()
    env = os.environ if env is None else env
    checks = {}
    def put(name, ok): checks[name] = bool(ok)
    receipt_raw = env.get('DLL_RD_G0_RECEIPT_PATH', '')
    zip_raw = env.get('DLL_RD_G0_CANDIDATE_ZIP', '')
    receipt = Path(receipt_raw).resolve() if receipt_raw else None
    source_zip = Path(zip_raw).resolve() if zip_raw else None
    receipt_sha = env.get('DLL_RD_TRUSTED_G0_RECEIPT_SHA256', '')
    source_sha = env.get('DLL_RD_TRUSTED_G0_CANDIDATE_ZIP_SHA256', '')
    catalog_sha = env.get('DLL_RD_TRUSTED_RELEASE_QA_CATALOG_SHA256', '')
    for label, path, sha in [('receipt', receipt, receipt_sha), ('source_zip', source_zip, source_sha)]:
        put(label + '_outside_source', bool(path and path.is_file() and root != path and root not in path.parents))
        put(label + '_pin_valid', SHA.fullmatch(sha) is not None)
        put(label + '_pin_matches', checks[label+'_outside_source'] and checks[label+'_pin_valid'] and digest(path) == sha)
    put('catalog_independently_pinned', SHA.fullmatch(catalog_sha) is not None)
    if all(checks.values()):
        try:
            record = json.loads(receipt.read_bytes())
            state = json.loads((root/'RELEASE_STATE.json').read_bytes())
            catalog = json.loads((root/'metadata/RELEASE_QA_CATALOG.json').read_bytes())
            put('receipt_is_prepublication_g0', record.get('schema') == 'universal-independent-g0-receipt.v1' and record.get('status') == 'PASS_G0_PREPUBLICATION' and record.get('g0_authorized') is True and record.get('g4_publication_authorized') is False and record.get('blockers') == [] and bool(record.get('checks')) and all(record['checks'].values()))
            detail = record.get('details', {})
            put('receipt_binds_source_and_catalog', detail.get('source_sha256') == source_sha and detail.get('catalog_sha256') == catalog_sha == digest(root/'metadata/RELEASE_QA_CATALOG.json'))
            put('receipt_binds_target_and_parent', detail.get('release_identity') == state.get('official_release_identity') and detail.get('release_type') == state.get('release_type') and detail.get('parent_sha256') == state.get('immediate_hotfix_parent', {}).get('sha256'))
            put('receipt_binds_governance', detail.get('governance_revision') == state.get('governance_master_revision') and detail.get('master_md_sha256') == state.get('governance_master_md_sha256') and detail.get('master_json_sha256') == state.get('governance_master_json_sha256'))
            put('catalog_structure_reviewed', verify_catalog(path=root/'metadata/RELEASE_QA_CATALOG.json', pin=catalog_sha)['status'] == 'PASS_STRUCTURE_ONLY' and not catalog.get('review_blockers'))
            # Exact source bytes must be used at every shard; this cheap check
            # does NOT rerun G0 or full ZIP/authority verification.
            manifest = json.loads((root/'metadata/ACTIVE_SOURCE_MANIFEST.json').read_bytes())['files']
            put('source_tree_manifest', bool(manifest) and all((root/name).is_file() and not (root/name).is_symlink() and digest(root/name) == expected for name, expected in manifest.items()))
        except Exception as exc:
            put('receipt_or_tree_parse', False)
    errors = [name for name, passed in checks.items() if not passed]
    return {'schema': 'release-qa-receipt-preflight.v1', 'status': 'PASS_PREPUBLICATION_AUTH_ONLY' if not errors else 'BLOCKED', 'checks': checks, 'blockers': errors, 'g4_publication_authorized': False}


def main():
    result = verify()
    print('RELEASE_QA_RECEIPT:', result['status'], sum(result['checks'].values()), '/', len(result['checks']))
    if result['blockers']: print('BLOCKERS:', ', '.join(result['blockers']))
    return 0 if result['status'] == 'PASS_PREPUBLICATION_AUTH_ONLY' else 3

if __name__ == '__main__':
    sys.exit(main())
