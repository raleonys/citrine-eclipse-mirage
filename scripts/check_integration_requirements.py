# SPDX-License-Identifier: GPL-3.0-only

"""Preserve the owner's broad Infrastructure mandate and pending runtime boundaries."""
import hashlib
import json
from pathlib import Path


def check(root):
    root = Path(root)
    req = json.loads((root / 'metadata/INTEGRATION_REQUIREMENTS.json').read_text())
    errors = []
    mandatory = {'build_orchestration': 'MANDATORY_ADAPT', 'documentation': 'MANDATORY_CLEAN_ADAPT',
                 'golden_tools': 'MANDATORY_ADAPT_AND_INTEGRATE', 'metadata': 'MANDATORY_CLEAN_ADAPT',
                 'resources': 'MANDATORY_ADAPT', 'root_layout': 'MANDATORY_CLEAN_ADAPT',
                 'scripts': 'MANDATORY_ADAPT', 'tooling': 'MANDATORY_EVALUATE_AND_ADAPT',
                 'tests': 'SELECT_COMPATIBLE_AND_PRESERVE_SURGE_OWN_TESTS',
                 'preloader_l_application_code': 'NOT_INHERITED'}
    if req.get('infrastructure') != mandatory:
        errors.append('INFRASTRUCTURE_SCOPE_CHANGED')
    if req.get('third_party') != {name: 'REQUIRED_ACTIVE' for name in ['asio','minhook','pegtl','pugixml']}:
        errors.append('MANDATORY_VENDOR_POLICY_WEAKENED')
    for role in ['redfield', 'greenfield']:
        if req.get(role) != 'COMPARISON_AND_CODE_REVIEW_ONLY':
            errors.append('FROZEN_COMPARISON_ROLE_CHANGED ' + role)
    if req.get('library_synchronization') != 'FORBIDDEN_UNLESS_NEW_EXPLICIT_USER_REQUEST':
        errors.append('LIBRARY_SCOPE_CHANGED')
    order = ['MSVC Build','URSYNEX Golden Finalizer','ARYRA Golden Layout',
             'MUSTAVEX Golden Metadata','CORVEXON Golden Trail','FALCION Golden Cleaning']
    if req.get('golden_pipeline_order') != order:
        errors.append('GOLDEN_ORDER_CHANGED')
    identity = json.loads((root / 'metadata/BRANDING_AND_IDENTITY.json').read_text())
    if identity.get('cpp_root_namespace') != 'preloader_l' or identity.get('display_name') != 'SURGE':
        errors.append('SURGE_IDENTITY_CHANGED')
    if hashlib.sha256((root / identity['branding_asset']).read_bytes()).hexdigest() != identity['asset_sha256']:
        errors.append('OWNER_AVATAR_CHANGED')
    local = json.loads((root / 'metadata/LOCAL_DATA_POLICY.json').read_text())
    if local.get('release_file_logging') != 'FORBIDDEN':
        errors.append('RELEASE_LOG_POLICY_WEAKENED')
    runtime_path = root / 'metadata/RUNTIME_CURRENT_STATUS.json'
    if not runtime_path.is_file():
        errors.append('CURRENT_RUNTIME_STATE_MISSING')
    else:
        runtime=json.loads(runtime_path.read_text())
        abi=json.loads((root/'metadata/PUBLIC_DLL_ABI.json').read_text())
        if runtime.get('role')!='CURRENT_RUNTIME_STATE':
            errors.append('CURRENT_RUNTIME_SNAPSHOT_STALE')
        if runtime.get('exports')!={name:entry['ordinal'] for name,entry in abi['exports'].items()}:
            errors.append('CURRENT_RUNTIME_ABI_MISMATCH')
        if (set(runtime.get('active_vendors',{}))!=set(req['third_party'])
                or runtime.get('active_vendor_count')!=len(req['third_party'])):
            errors.append('CURRENT_RUNTIME_VENDOR_MISMATCH')
        if runtime.get('logging',{}).get('release_file_logging')!=local['release_file_logging']:
            errors.append('CURRENT_RUNTIME_LOG_POLICY_MISMATCH')
        if runtime.get('windows_filesystem_backend')!='STANDARD_COMPILER_FILESYSTEM_AND_FSTREAM':
            errors.append('CURRENT_RUNTIME_FILESYSTEM_STALE')
    matrix = json.loads((root / 'metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json').read_text())
    expected_ids = {'INFRA-DOC','INFRA-GOLD','INFRA-ROOT','INFRA-META','INFRA-RES','INFRA-SCR','INFRA-ORCH','INFRA-TEST','INFRA-TOOLS','INFRA-UX'}
    records = matrix['requirements']
    if {r['id'] for r in records} != expected_ids or len(records) != len(expected_ids):
        errors.append('INFRASTRUCTURE_TRANSFER_SCOPE_LOST')
    return errors


if __name__ == '__main__':
    try:
        errors = check(Path(__file__).resolve().parents[1])
    except (OSError, ValueError, KeyError) as error:
        errors = [str(error)]
    for error in errors:
        print(' -', error)
    print('INTEGRATION_REQUIREMENTS:', 'FAIL' if errors else 'PASS_TARGET_CONTRACT_ONLY')
    raise SystemExit(1 if errors else 0)
