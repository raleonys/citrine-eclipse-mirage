#!/usr/bin/env python3
"""Generic externally pinned G0 prepublication checker.

All authority and ZIP pins must be operator-supplied outside the candidate tree.
A PASS here is a bounded PREPUBLICATION gate, never a G4/pointer commit.
"""
import hashlib
import json
import os
import re
import sys
import zipfile
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]
SHA = re.compile(r'[0-9a-f]{64}\Z')
HOTFIX = re.compile(r'^preloader_l (\d+\.\d+\.\d+) ([A-Z][A-Z0-9]+) HOTFIX (\d+)$')
INPUTS = {
    'master_md': ('DLL_RD_GOVERNANCE_MASTER_MD', 'DLL_RD_TRUSTED_MASTER_MD_SHA256'),
    'master_json': ('DLL_RD_GOVERNANCE_MASTER_JSON', 'DLL_RD_TRUSTED_MASTER_JSON_SHA256'),
    'intent': ('DLL_RD_INDEPENDENT_RELEASE_INTENT', 'DLL_RD_TRUSTED_RELEASE_INTENT_SHA256'),
    'pointer': ('DLL_RD_INDEPENDENT_CURRENT_ARTIFACT', 'DLL_RD_TRUSTED_CURRENT_ARTIFACT_SHA256'),
    'parent_zip': ('DLL_RD_G0_PARENT_ZIP', 'DLL_RD_TRUSTED_G0_PARENT_ZIP_SHA256'),
    'source_zip': ('DLL_RD_G0_CANDIDATE_ZIP', 'DLL_RD_TRUSTED_G0_CANDIDATE_ZIP_SHA256'),
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def outside(path, root):
    return path != root and root not in path.parents


def evaluate(root=ROOT, environ=None):
    root = Path(root).resolve()
    env = os.environ if environ is None else environ
    checks, locations, pins = {}, {}, {}
    def put(name, value):
        checks[name] = bool(value)
    for key, (path_env, sha_env) in INPUTS.items():
        raw = env.get(path_env, '')
        loc = Path(raw).resolve() if raw else None
        pin = env.get(sha_env, '')
        put(key + '_outside_source', loc is not None and outside(loc, root) and loc.is_file())
        put(key + '_trusted_sha', SHA.fullmatch(pin) is not None)
        put(key + '_verified', checks[key + '_outside_source'] and checks[key + '_trusted_sha'] and digest(loc.read_bytes()) == pin)
        locations[key], pins[key] = loc, pin
    details = {'authority_mode': 'independently_pinned_prepublication', 'publication_committed': False}
    if not all(checks.values()):
        return finish(checks, details)
    try:
        master = json.loads(locations['master_json'].read_bytes())
        intent = json.loads(locations['intent'].read_bytes())
        pointer = json.loads(locations['pointer'].read_bytes())
        state = json.loads((root/'RELEASE_STATE.json').read_bytes())
        authorization = json.loads((root/'metadata/RELEASE_AUTHORIZATION.json').read_bytes())
        transaction = json.loads((root/'metadata/PUBLICATION_TRANSACTION.json').read_bytes())
        catalog_raw = (root/'metadata/RELEASE_QA_CATALOG.json').read_bytes()
        catalog = json.loads(catalog_raw)
        parent = master.get('current_release_line', {})
        target = state.get('official_release_identity', '')
        match = HOTFIX.fullmatch(target)
        put('governance_master_live', master.get('schema') == 'dll-rd-football.project-rules-master.v1' and master.get('status') == 'ACTIVE_FAIL_CLOSED' and master.get('governance_first_protocol', {}).get('status') == 'ACTIVE_FAIL_CLOSED')
        put('master_current_parent', parent.get('artifact_sha256') == pins['parent_zip'] and pointer.get('schema') == 'dll-rd-football.current-artifact.v1' and pointer.get('status') == 'CURRENT_ACCEPTED_OFFICIAL_RELEASE' and pointer.get('current_artifact', {}).get('sha256') == pins['parent_zip'] and pointer.get('release_identity', {}).get('display') == parent.get('display'))
        put('hotfix_monotonic_identity', match is not None and parent.get('release_type') == 'HOTFIX_RELEASE' and state.get('release_type') == 'HOTFIX_RELEASE' and int(match.group(3)) == parent.get('hotfix_ordinal', -2) + 1 and state.get('hotfix_ordinal') == int(match.group(3)) and match.group(1) == parent.get('semantic_version') and match.group(2) == parent.get('codename'))
        put('exact_parent_binding', state.get('immediate_hotfix_parent', {}).get('sha256') == pins['parent_zip'] and transaction.get('canonical_before', {}).get('sha256') == pins['parent_zip'] and transaction.get('canonical_before', {}).get('identity') == parent.get('display'))
        put('source_and_authorization_target', authorization.get('official_release_identity') == target and authorization.get('publication_authorized') is True and transaction.get('official_release_identity') == target and transaction.get('publication_authorized') is True)
        put('independent_release_intent', intent.get('release_identity') == target and intent.get('release_type') == state.get('release_type') and intent.get('source_development_authorized') is True and intent.get('publication_authorized') is True and intent.get('publication_requires_gate_pass') is True and intent.get('publication_committed') is False and bool(intent.get('user_instruction')))
        put('intent_parent', intent.get('immediate_official_parent', {}).get('sha256') == pins['parent_zip'])
        im = intent.get('master', {})
        put('governance_binding', state.get('governance_master_revision') == master.get('revision') == im.get('revision') and state.get('governance_master_json_sha256') == pins['master_json'] == im.get('json_sha256') and state.get('governance_master_md_sha256') == pins['master_md'] == im.get('md_sha256'))
        if 'candidate_source_zip_sha256' in intent:
            put('intent_source_zip_bound', intent['candidate_source_zip_sha256'] == pins['source_zip'])
        put('not_already_published', state.get('canonical_artifact_pointer_commit') is False and state.get('publication_date') is None and transaction.get('canonical_pointer_committed') is False and transaction.get('post_commit_verified') is False)
        put('catalog_shape_without_self_authorization', catalog.get('schema') == 'universal-release-qa-catalog.v1' and catalog.get('scope') == 'IDS_QA_ONLY_NOT_AUTHORIZED' and catalog.get('release_identity') is None and catalog.get('review_blockers') == [] and len(catalog.get('steps', [])) == catalog.get('inherited_step_count') == 64)
        manifest = json.loads((root/'metadata/ACTIVE_SOURCE_MANIFEST.json').read_bytes())['files']
        expected = set(manifest) | {'metadata/ACTIVE_SOURCE_MANIFEST.json'}
        put('source_manifest_format', isinstance(manifest, dict) and len(manifest) > 100 and all(isinstance(name, str) and SHA.fullmatch(value) for name, value in manifest.items()))
        if checks['source_manifest_format']:
            with zipfile.ZipFile(locations['source_zip']) as z:
                entries = z.infolist()
                names = [item.filename for item in entries]
                safe = (len(names) == len(set(names)) and set(names) == expected and z.testzip() is None and all(name and name == str(PurePosixPath(name)) and not name.startswith('/') and '\\' not in name and '..' not in PurePosixPath(name).parts and not name.endswith('/') for name in names) and not any(((i.external_attr >> 16) & 0o170000) == 0o120000 for i in entries))
                put('source_zip_flat_exact', safe)
                put('source_zip_tree_manifest', safe and z.read('metadata/ACTIVE_SOURCE_MANIFEST.json') == (root/'metadata/ACTIVE_SOURCE_MANIFEST.json').read_bytes() and all((root/name).is_file() and not (root/name).is_symlink() and digest(z.read(name)) == value and digest((root/name).read_bytes()) == value for name, value in manifest.items()))
        else:
            put('source_zip_flat_exact', False)
            put('source_zip_tree_manifest', False)
        details.update({'release_identity': target, 'release_type': state.get('release_type'), 'source_sha256': pins['source_zip'], 'parent_sha256': pins['parent_zip'], 'catalog_sha256': digest(catalog_raw), 'master_json_sha256': pins['master_json'], 'master_md_sha256': pins['master_md'], 'governance_revision': master.get('revision')})
    except Exception as exc:
        put('parse_and_verify', False)
        details['error'] = type(exc).__name__ + ': ' + str(exc)[:160]
    return finish(checks, details)


def finish(checks, details):
    failures = [name for name, ok in checks.items() if not ok]
    return {'schema': 'universal-independent-g0-receipt.v1', 'status': 'PASS_G0_PREPUBLICATION' if checks and not failures else 'BLOCKED', 'g0_authorized': bool(checks and not failures), 'g1_authorized': False, 'g2_authorized': False, 'g4_publication_authorized': False, 'checks': checks, 'blockers': failures, 'details': details}


def main():
    outcome = evaluate()
    output = os.environ.get('DLL_RD_G0_REPORT_PATH')
    if output:
        dest = Path(output).resolve()
        if not outside(dest, ROOT):
            print('UNIVERSAL_EXTERNAL_G0: BLOCKED receipt must be outside source')
            return 3
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_text(json.dumps(outcome, indent=2, sort_keys=True) + '\n')
    print('UNIVERSAL_EXTERNAL_G0:', outcome['status'], 'checks', sum(outcome['checks'].values()), '/', len(outcome['checks']))
    if outcome['blockers']:
        print('BLOCKERS:', ', '.join(outcome['blockers']))
    return 0 if outcome['g0_authorized'] else 3


if __name__ == '__main__':
    sys.exit(main())
