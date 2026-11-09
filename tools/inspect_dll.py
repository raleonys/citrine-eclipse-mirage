# SPDX-License-Identifier: GPL-3.0-only

"""Read-only DLL imports/exports, VERSIONINFO and exact read-only avatar data."""
import argparse
import hashlib
import json
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from verify_corvexon_golden_trail import PE, PEError, u16, u32, cstr, compute_pe_checksum
from pe_resources import resource_leaves
from verify_mustavex_golden_metadata import validate_resource_leaves, inspect as inspect_metadata
from project_version import resolve_policy


def inspect(path, root, final=False):
    path, root = Path(path), Path(root)
    before = hashlib.sha256(path.read_bytes()).hexdigest()
    pe = PE(path)
    exports = pe.exports()
    resources = [{'path': p, 'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
                 for p, data, cp in resource_leaves(pe)]
    avatar_contract_path = root/'metadata/AVATAR_DATA_CONTRACT.json'
    avatar_contract = json.loads(avatar_contract_path.read_text(encoding='utf-8'))
    avatars = {}
    for role,spec in avatar_contract['assets'].items():
        data=(root/spec['source']).read_bytes()
        storage=[]
        for name,_va,_span,rp,rs,chars in pe.sections:
            if data in pe.b[rp:rp+rs]:
                storage.append({'section':name,'read_only':bool(chars & 0x40000000) and not bool(chars & 0xA0000000)})
        avatars[role]={'sha256':hashlib.sha256(data).hexdigest(),
                       'exact_input':hashlib.sha256(data).hexdigest()==spec['sha256'],
                       'storage':storage,
                       'present_as_data':any(s['read_only'] and s['section']!='.rsrc' for s in storage)}
    matches=all(v['exact_input'] and v['present_as_data'] for v in avatars.values())
    resource_policy_path = root / 'metadata/RESOURCE_METADATA_CONTRACT.json'
    metadata_errors = []
    if resource_policy_path.is_file():
        resource_policy = resolve_policy(json.loads(resource_policy_path.read_text(encoding='utf-8')), root)
        metadata_errors = validate_resource_leaves(resource_leaves(pe), resource_policy['required'])
    after = hashlib.sha256(path.read_bytes()).hexdigest()
    crypt32_imports = pe.import_names('CRYPT32.DLL')
    crypt32_matches = crypt32_imports == ['CryptQueryObject']
    metadata_policy=resolve_policy(json.loads((root/'metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text(encoding='utf-8')), root)
    header_errors=inspect_metadata(path,metadata_policy)
    export_rva,_=pe.directory(0)
    export_offset=pe.rva2off(export_rva)
    header={'timestamp':f'0x{u32(pe.b,pe.coff+4):08X}',
            'linker_version':[pe.b[pe.opt+2],pe.b[pe.opt+3]],
            'rich_header':'PRESENT' if b'Rich' in pe.b[:pe.pe] else 'ABSENT',
            'os_version':[u16(pe.b,pe.opt+40),u16(pe.b,pe.opt+42)],
            'subsystem_version':[u16(pe.b,pe.opt+48),u16(pe.b,pe.opt+50)],
            'subsystem':u16(pe.b,pe.opt+68),
            'export_internal_name':cstr(pe.b,pe.rva2off(u32(pe.b,export_offset+12))),
            'debug_directory':list(pe.directory(6)),
            'security_directory':list(pe.directory(4)),
            'coff_symbol_table':[u32(pe.b,pe.coff+8),u32(pe.b,pe.coff+12)],
            'pe_checksum':f'0x{u32(pe.b,pe.opt+64):08X}',
            'checksum_valid':u32(pe.b,pe.opt+64)==compute_pe_checksum(pe.b,pe.opt+64)}
    passed=matches and before==after and not metadata_errors and crypt32_matches
    import_order=json.loads((root/'metadata/CORVEXON_GOLDEN_TRAIL.json').read_text(encoding='utf-8'))['import_policy'].get('module_order')
    import_order_matches=import_order is None or pe.imports()==[x.upper() for x in import_order]
    if final:passed=passed and not header_errors and header['checksum_valid'] and import_order_matches
    return {'execution_level': 'READ_ONLY_BINARY_INSPECTION',
            'result': ('PASS_FINAL_METADATA_CHECK' if final else 'PASS_DEVELOPMENT_RESOURCE_CHECK') if passed else 'FAIL',
            'header_metadata':header,
            'infrastructure_metadata_validation':{'stage':'FINAL_DLL' if final else 'RAW_LINK_OUTPUT',
                'result':'PASS' if not header_errors else 'FAIL' if final else 'NOT_FINALIZED',
                'differences':header_errors},
            'dll_sha256': before, 'unchanged': before == after,
            'machine': hex(u16(pe.b, pe.coff)), 'entrypoint_rva': hex(u32(pe.b, pe.opt + 16)),
            'sections': [s[0] for s in pe.sections], 'export_count': len(exports),
            'imports': pe.imports(), 'resources': resources, 'exact_owner_avatar': matches,
            'expected_import_module_order':import_order,'import_module_order_matches':import_order_matches,
            'crypt32_static_import':crypt32_matches, 'crypt32_named_imports':crypt32_imports,
            'avatar_storage':'READ_ONLY_CPP_DATA_NOT_PE_RESOURCE', 'avatar_data':avatars,
            'versioninfo_contract': 'metadata/RESOURCE_METADATA_CONTRACT.json' if resource_policy_path.is_file() else 'NOT_SELECTED',
            'resource_metadata_errors': metadata_errors,
            'golden_shipping_acceptance': 'NOT_RUN', 'game_acceptance': 'NOT_RUN'}


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('dll', type=Path)
    ap.add_argument('--report', type=Path)
    ap.add_argument('--final',action='store_true',help='Require the finalized Infrastructure metadata profile and checksum')
    args = ap.parse_args()
    try:
        report = inspect(args.dll, Path(__file__).resolve().parents[1], final=args.final)
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(report, indent=2))
        raise SystemExit(0 if report['result'].startswith('PASS') else 1)
    except (OSError, ValueError, PEError, KeyError) as error:
        print('DLL_INSPECTION: FAIL:', error, file=sys.stderr)
        raise SystemExit(1)
