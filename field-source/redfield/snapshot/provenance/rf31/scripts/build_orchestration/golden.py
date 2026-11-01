# SPDX-License-Identifier: GPL-3.0-only
"""Destination-bound local Golden stages; no release/CURRENT operations.

HF19 mutation/layout/metadata/ABI contracts are retained through explicit policy
bindings. Source hygiene and link auditing use this project's current ownership
and BuildSession interfaces rather than a parallel historical build backend.
"""
import argparse
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import sys
import tempfile
import zipfile
from pe_image import Image, PEError, checksum, unpack, version_tree

ORDER = ('URSYNEX Golden Finalizer','ARYRA Golden Layout','MUSTAVEX Golden Metadata',
         'CORVEXON Golden Trail','FALCION Golden Cleaning')
POLICIES = ('URSYNEX_GOLDEN_FINALIZER','ARYRA_GOLDEN_LAYOUT',
            'MUSTAVEX_GOLDEN_METADATA','CORVEXON_GOLDEN_TRAIL')
CRT_PREFIXES = ('MSVCP','MSVCR','VCRUNTIME','UCRTBASE','API-MS-WIN-CRT-')
FIXED_KEYS = ('dwSignature','dwStrucVersion','dwFileVersionMS','dwFileVersionLS',
              'dwProductVersionMS','dwProductVersionLS','dwFileFlagsMask','dwFileFlags',
              'dwFileOS','dwFileType','dwFileSubtype','dwFileDateMS','dwFileDateLS')


def sha(data):
    return hashlib.sha256(data).hexdigest()


def hx(value):
    return int(value, 16) if isinstance(value, str) else value


def load_contract(root):
    binding = json.loads((root/'metadata/GOLDEN_BINDING.json').read_text(encoding='utf-8'))
    if (binding.get('schema') != 1 or binding.get('status') != 'BOUND_LOCAL_M20_3' or
            binding.get('classification') != 'LOCAL_BUILD/NOT_RELEASE' or
            binding.get('shipping_runtime') != '/MT' or binding.get('shipping_gs') != '/GS' or
            binding.get('resource_source') != 'resources/preloader_version.rc' or
            binding.get('data_virtual_size_limit') != 0x1800 or
            binding.get('require_static_fptable_for_production') is not True or
            binding.get('hygiene_profile') != 'LOCAL_REFACTOR_SOURCE_PACKAGE' or
            binding.get('additional_forbidden_identity_tokens') != [json.loads((root/'metadata/ARCHITECTURE.json').read_text(encoding='utf-8'))['project_name']] or
            binding.get('retained_policies') != list(POLICIES)):
        raise ValueError('GOLDEN_DESTINATION_BINDING_DRIFT')
    records = binding['sources']; paths = [row['path'] for row in records]
    origins = {'scripts/'+name+'.py' for name in ('ursynex_golden_finalizer','verify_aryra_golden_layout',
               'verify_mustavex_golden_metadata','verify_corvexon_golden_trail','check_falcion_golden_cleaning',
               'check_data_footprint','check_msvc_shipping_link_command','test_pe_metadata_tools')}
    origins |= {'metadata/'+name+'.json' for name in (*POLICIES,'CRT_STATIC_CONTRACT')}
    origins |= {'resources/preloader_version.rc','exports/preloader_l.def'}
    if (len(paths) != len(set(paths)) or set(paths) != {'provenance/hf19_golden/'+p for p in origins} or
            any(row['path'] != 'provenance/hf19_golden/'+row['origin'] for row in records) or
            binding.get('canonical_hf19_zip_sha256') != 'c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab'):
        raise ValueError('GOLDEN_REFERENCE_INVENTORY')
    for row in records:
        relative = row['path']; path = root/relative
        if (not relative.startswith('provenance/hf19_golden/') or '..' in Path(relative).parts or
                '\\' in relative or path.is_symlink() or not path.resolve().is_relative_to(root.resolve()) or
                sha(path.read_bytes()) != row['sha256']):
            raise ValueError('GOLDEN_REFERENCE_HASH_OR_PATH')
    policies = {}
    for name in POLICIES:
        path = root/'metadata'/(name+'.json')
        reference = root/'provenance/hf19_golden/metadata'/(name+'.json')
        if path.read_bytes() != reference.read_bytes():
            raise ValueError('GOLDEN_RETAINED_POLICY_DRIFT:' + name)
        policies[name] = json.loads(path.read_text(encoding='utf-8'))
    finalizer = policies[POLICIES[0]]; metadata = policies[POLICIES[2]]['required']
    if (finalizer['mutation_values']['linker_version'] != metadata['linker_version'] or
            finalizer['mutation_values']['timestamp'] != metadata['timestamp'] or
            finalizer['mutation_values']['rich_header'] != metadata['rich_header'] or
            finalizer['integrity_finalization']['pe_checksum'] != 'RECOMPUTE_AFTER_APPROVED_MUTATIONS'):
        raise ValueError('GOLDEN_MUTATOR_METADATA_BINDING')
    if (root/'exports/preloader_l.def').read_text(encoding='utf-8').splitlines()[0] != 'LIBRARY "preloader.unsigned.dll"':
        raise ValueError('GOLDEN_EXPORT_INTERNAL_NAME_BINDING')
    return binding, policies


def metadata_core(image, req, *, finalized):
    if image.u16(68) != req['subsystem']:
        raise PEError('SUBSYSTEM_WINDOWS_GUI_REQUIRED')
    for offset, key in [(40,'os_version'),(48,'subsystem_version')]:
        if (image.u16(offset), image.u16(offset+2)) != tuple(req[key]):
            raise PEError('VERSION_POLICY:' + key)
    if image.directory(6) != (0,0):
        raise PEError('DEBUG_DIRECTORY_FORBIDDEN')
    if image.directory(4) != (0,0):
        raise PEError('CERTIFICATE_DIRECTORY_FORBIDDEN')
    if finalized and (unpack(image.data,'<I',image.coff+4)[0] != hx(req['timestamp']) or
                      image.data[image.optional+2:image.optional+4] != bytes(req['linker_version'])):
        raise PEError('FINAL_METADATA_TIMESTAMP_OR_LINKER')


def finalized_bytes(data, policies):
    image = Image(data); req = policies[POLICIES[2]]['required']
    metadata_core(image, req, finalized=False)
    result = bytearray(data)
    allowed = set(range(image.coff+4,image.coff+8)) | set(range(image.optional+2,image.optional+4)) | set(range(image.optional+64,image.optional+68))
    rich = data.find(b'Rich',0,image.pe)
    if rich != -1:
        if rich+8 > image.pe or rich % 4 or data.find(b'Rich',rich+4,image.pe) != -1:
            raise PEError('RICH_HEADER_RANGE_OR_DUPLICATE')
        key, = unpack(data,'<I',rich+4)
        start = next((o for o in range(rich-4,63,-4) if unpack(data,'<I',o)[0] ^ key == 0x536e6144),None)
        if start is None or rich-start < 16:
            raise PEError('RICH_DANS_NOT_FOUND')
        allowed.update(range(start,rich+8)); result[start:rich+8] = bytes(rich+8-start)
    struct.pack_into('<I',result,image.coff+4,hx(req['timestamp']))
    result[image.optional+2:image.optional+4] = bytes(req['linker_version'])
    struct.pack_into('<I',result,image.optional+64,checksum(result,image.optional+64))
    if len(result) != len(data) or any(a != b and i not in allowed for i,(a,b) in enumerate(zip(data,result))):
        raise PEError('MUTATION_ALLOWLIST_VIOLATION')
    metadata_core(Image(result),req,finalized=True)
    return bytes(result), len([1 for a,b in zip(data,result) if a != b])


def atomic_finalize(path, data, expected):
    if path.is_symlink() or sha(path.read_bytes()) != expected:
        raise ValueError('FINALIZER_INPUT_CHANGED_OR_SYMLINK')
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(prefix='.'+path.name+'.',suffix='.tmp',dir=path.parent,delete=False) as stream:
            temporary = Path(stream.name)
            if stream.write(data) != len(data):
                raise OSError('FINALIZER_SHORT_WRITE')
            stream.flush(); os.fsync(stream.fileno())
        if sha(temporary.read_bytes()) != sha(data) or sha(path.read_bytes()) != expected:
            raise ValueError('FINALIZER_STAGED_OR_INPUT_CHANGED')
        os.replace(temporary,path); temporary = None
        if path.read_bytes() != data:
            raise ValueError('FINALIZER_READBACK_MISMATCH')
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def layout(image, policy):
    names = [s['name'] for s in image.sections]; fp = policy['static_crt_fptable_profile']
    expected = fp['sections_in_order'] if len(names) == fp['section_count'] else policy['sections_in_order']
    if names != expected:
        raise PEError('SECTION_PROFILE:' + repr(names))
    file_alignment = image.u32(36); alignment = image.u32(32)
    if not 0x200 <= file_alignment <= 0x10000 or file_alignment & (file_alignment-1):
        raise PEError('FILE_ALIGNMENT')
    if not alignment or alignment & (alignment-1) or alignment < file_alignment or alignment < 0x1000 and alignment != file_alignment:
        raise PEError('SECTION_ALIGNMENT')
    headers = image.u32(60)
    if headers < image.section_table_end or headers % file_alignment or headers > len(image.data):
        raise PEError('SIZE_OF_HEADERS')
    virtual, raw = [], []
    for s in image.sections:
        if s['va'] < headers or s['va'] % alignment:
            raise PEError('SECTION_VIRTUAL_ALIGNMENT_OR_HEADERS')
        if s['size']:
            if s['raw'] < headers or s['raw'] % file_alignment or s['size'] % file_alignment:
                raise PEError('SECTION_RAW_ALIGNMENT_OR_HEADERS')
            raw.append((s['raw'],s['raw']+s['size']))
        virtual.append((s['va'],s['va']+max(s['vs'],s['size'])))
        if s['flags'] & 0xa0000000 == 0xa0000000:
            raise PEError('WRITABLE_EXECUTABLE_SECTION')
    for ranges in [virtual,raw]:
        ordered = sorted(ranges)
        if any(b[0] < a[1] for a,b in zip(ordered,ordered[1:])):
            raise PEError('SECTION_OVERLAP')
    backed = [s for s in image.sections if s['size']]
    if [s['raw'] for s in sorted(backed,key=lambda s:s['va'])] != sorted(s['raw'] for s in backed):
        raise PEError('RAW_RVA_ORDER')
    end = max(x[1] for x in virtual)
    if image.u32(56) % alignment or image.u32(56) < (end+alignment-1)//alignment*alignment:
        raise PEError('SIZE_OF_IMAGE')
    if len(names) == fp['section_count']:
        s = image.sections[fp['fptable']['index']]; spec = fp['fptable']
        if (s['name'] != spec['name'] or s['vs'] != hx(spec['virtual_size']) or
                s['size'] != hx(spec['raw_size']) or s['flags'] != hx(spec['characteristics']) or
                any(image.data[s['raw']:s['raw']+s['size']])):
            raise PEError('FPTABLE_PROFILE')
    return {'sections':names,'exact_donor_rva_size_parity':False}


def validate_version(data, req):
    tree = version_tree(data); spec = req['versioninfo']
    if tree['key'] != 'VS_VERSION_INFO' or tree['kind'] != 0 or len(tree['value']) != 52:
        raise PEError('VERSIONINFO_FIXED_ROOT')
    expected = tuple(hx(spec['fixed'][k]) for k in FIXED_KEYS)
    if unpack(tree['value'],'<13I',0) != expected:
        raise PEError('VERSIONINFO_FIXED_VALUE')
    children = tree['children']; keys = [n['key'] for n in children]
    if sorted(keys) != ['StringFileInfo','VarFileInfo'] or any(n['value'] for n in children):
        raise PEError('VERSIONINFO_BLOCK_ALLOWLIST')
    strings = next(n for n in children if n['key'] == 'StringFileInfo')['children']
    if len(strings) != 1 or strings[0]['key'] != spec['string_table'] or strings[0]['value']:
        raise PEError('VERSIONINFO_STRING_TABLE')
    fields = strings[0]['children']; values = {}
    for field in fields:
        if field['key'] in values or field['kind'] != 1 or field['children'] or not field['value'].endswith(b'\0\0'):
            raise PEError('VERSIONINFO_STRING_SHAPE')
        values[field['key']] = field['value'][:-2].decode('utf-16le')
    if values != spec['strings']:
        raise PEError('VERSIONINFO_STRINGS')
    var = next(n for n in children if n['key'] == 'VarFileInfo')['children']
    if (len(var) != 1 or var[0]['key'] != 'Translation' or var[0]['kind'] != 0 or var[0]['children'] or
            var[0]['value'] != struct.pack('<HH',hx(spec['translation_language']),spec['translation_codepage'])):
        raise PEError('VERSIONINFO_TRANSLATION')


def metadata(image, policy, binding):
    req = policy['required']; metadata_core(image,req,finalized=True)
    if unpack(image.data,'<II',image.coff+8) != (0,0) or b'Rich' in image.data[:image.pe]:
        raise PEError('COFF_SYMBOLS_OR_RICH_FORBIDDEN')
    _, internal = image.exports()
    if internal != req['export_internal_name']:
        raise PEError('EXPORT_INTERNAL_NAME')
    leaves = image.resources(); expected = [16,1,hx(req['versioninfo']['translation_language'])]
    if len(leaves) != 1 or leaves[0][0] != expected or leaves[0][2] not in {0,1200}:
        raise PEError('RESOURCE_ALLOWLIST')
    validate_version(leaves[0][1],req)
    tokens = policy['project_identity_forbidden_in_shipping_dll'] + policy['forbidden_project_identity_tokens'] + binding['additional_forbidden_identity_tokens']
    folded = image.data.lower()
    wide = [image.data[p:len(image.data)-((len(image.data)-p)%2)].decode('utf-16le','ignore').casefold() for p in (0,1)]
    if any(t.casefold().encode('ascii') in folded or any(t.casefold() in v for v in wide) for t in tokens):
        raise PEError('PROJECT_RELEASE_IDENTITY_IN_BINARY')
    return {'internal_export_name':internal,'versioninfo':'PINNED_COMPATIBILITY_ALLOWLIST', 'certificate_debug_rich_symbols':'ABSENT'}


def binary_acceptance(image, *, native=False, policy=None):
    exports, _ = image.exports(native=native); imports = image.imports()
    if any(name.startswith(CRT_PREFIXES) for name in imports):
        raise PEError('DYNAMIC_CRT_IMPORT')
    if image.directory(13) != (0,0):
        raise PEError('DELAY_IMPORTS_NOT_APPROVED')
    entry = image.u32(16)
    if not entry:
        raise PEError('ENTRY_OR_ASLR_NX_HIGH_ENTROPY')
    section = image.section_for(entry)
    if not entry or not section['flags'] & 0x20000000 or image.u16(70) & 0x160 != 0x160:
        raise PEError('ENTRY_OR_ASLR_NX_HIGH_ENTROPY')
    info = {'exports':{k:v[0] for k,v in exports.items()},'imports':sorted(imports),
            'entry_rva':entry,'machine':'x64','static_crt_import_check':'PASS'}
    if policy is None:
        return info
    if section['name'] != '.text' or entry == exports['DllMain'][1]:
        raise PEError('DEFAULT_CRT_ENTRY_RELATION')
    if image.u16(68) != policy['required_core']['subsystem']:
        raise PEError('BINARY_SUBSYSTEM')
    ip = policy['import_policy']; modules = set(imports)
    if not {n.upper() for n in ip['required_modules']}.issubset(modules) or not modules.issubset({n.upper() for n in ip['allowed_modules']}):
        raise PEError('IMPORT_MODULE_ALLOWLIST')
    for module, functions in ip['restricted_named_imports'].items():
        if module.upper() in imports and sorted(imports[module.upper()]) != sorted(functions):
            raise PEError('RESTRICTED_IMPORT_NAMES:' + module)
    load_rva, load_size = image.directory(10)
    if not load_rva or load_size < 96:
        raise PEError('GS_LOAD_CONFIG_REQUIRED')
    image.mapped(load_rva,load_size)
    declared, = image.read_rva(load_rva,'<I')
    cookie, = image.read_rva(load_rva+88,'<Q')
    base = image.u64(24)
    if declared < 96 or declared > load_size or not base <= cookie < base+image.u32(56):
        raise PEError('GS_COOKIE_RANGE')
    cookie_section = image.section_for(cookie-base,8)
    if not cookie_section['flags'] & 0x80000000:
        raise PEError('GS_COOKIE_WRITABLE_SECTION')
    if image.u32(64) != checksum(image.data,image.optional+64):
        raise PEError('PE_CHECKSUM')
    info.update(gs_cookie_rva=cookie-base,checksum=image.u32(64),crt_entry_distinct_from_DllMain=True)
    return info


def hygiene(root, image, binding, policies, package=None):
    from build_common import source_preflight
    qualification = source_preflight(root)
    expected = policies[POLICIES[1]]['static_crt_fptable_profile']['sections_in_order']
    if [s['name'] for s in image.sections] != expected:
        raise PEError('STATIC_CRT_DATA_PROFILE')
    data = next(s for s in image.sections if s['name'] == '.data')
    if data['vs'] > binding['data_virtual_size_limit']:
        raise PEError('DATA_VIRTUAL_SIZE_BUDGET')
    roots = {'src','include','tests','tools','debug','scripts','resources','exports','third_party','provenance','evidence','metadata','docs','examples'}
    files = {'README.md','LICENSE','CMakeLists.txt','CHANGELOG.md','SOURCE_REVISION.md'}
    for path in root.iterdir():
        if path.name in {'build','dist'}:
            if path.is_symlink():
                raise ValueError('HYGIENE_GENERATED_ROOT_SYMLINK')
            continue
        if path.name not in (roots if path.is_dir() else files):
            raise ValueError('HYGIENE_UNOWNED_ROOT:' + path.name)
    patterns = [re.compile(r'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----'),
                re.compile(r'(?<![A-Z0-9])AKIA[0-9A-Z]{16}(?![A-Z0-9])'),
                re.compile(r'(?<![A-Za-z0-9_])gh[pousr]_[A-Za-z0-9_]{24,}(?![A-Za-z0-9_])'),
                re.compile(r'(?i)\bBearer\s+[A-Za-z0-9._~+/-]{24,}={0,2}'),
                re.compile(r'\beyJ[A-Za-z0-9_-]{12,}\.[A-Za-z0-9_-]{12,}\.[A-Za-z0-9_-]{12,}\b'),
                re.compile(r'(?i)\b[A-Z]:\\Users\\(?!USER(?:NAME)?\\|Public\\|Default\\|example\\|test\\|dummy\\)[^\\\r\n\t ]+\\'),
                re.compile(r'(?<![A-Za-z0-9_])/(?:home|Users)/(?!USER(?:NAME)?/|Public/|Default/|example/|test/|dummy/)[^/\r\n\t ]+/')]
    manifest = json.loads((root/'metadata/PACKAGE_MANIFEST.json').read_text(encoding='utf-8'))['files']
    for relative in manifest:
        path = root/relative
        if path.suffix.lower() in {'.zip','.7z','.rar','.dll','.exe','.obj','.o','.pdb','.pyc','.pyo'}:
            raise ValueError('HYGIENE_GENERATED_OR_EMBEDDED_PAYLOAD:' + relative)
        if not relative.startswith('third_party/'):
            text = path.read_bytes().decode('utf-8','ignore')
            if any(p.search(text) for p in patterns):
                raise ValueError('HYGIENE_SENSITIVE_MATERIAL:' + relative)
    if package is not None:
        prefix = root.name+'/'
        with zipfile.ZipFile(package) as z:
            seen, folded = set(), set()
            for info in z.infolist():
                name = info.filename
                if (not name.startswith(prefix) or '\\' in name or any(ord(c)<32 or ord(c)==127 for c in name) or
                        any(p in {'','.','..'} for p in name.split('/')) or info.is_dir() or
                        name in seen or name.casefold() in folded or (info.external_attr >> 16) & 0o170000 not in {0,0o100000}):
                    raise ValueError('HYGIENE_PACKAGE_PATH_OR_DUPLICATE')
                seen.add(name); folded.add(name.casefold())
                relative = name[len(prefix):]
                if relative != 'metadata/PACKAGE_MANIFEST.json':
                    if relative not in manifest or z.read(info) != (root/relative).read_bytes():
                        raise ValueError('HYGIENE_PACKAGE_BYTES_OR_MEMBERSHIP')
                elif z.read(info) != (root/relative).read_bytes():
                    raise ValueError('HYGIENE_PACKAGE_MANIFEST')
            if seen != {prefix+p for p in manifest} | {prefix+'metadata/PACKAGE_MANIFEST.json'}:
                raise ValueError('HYGIENE_PACKAGE_FILE_SET')
    return {'source_manifest_sha256':sha((root/'metadata/PACKAGE_MANIFEST.json').read_bytes()),
            'data_virtual_size':data['vs'],'data_budget':binding['data_virtual_size_limit'],
            'scope':'LOCAL_REFACTOR_SOURCE_PACKAGE;not official flat release acceptance',
            'import_refactor':qualification,'package_checked':package is not None}


@dataclass(frozen=True)
class Stage:
    name: str
    before: str
    after: str


@dataclass(frozen=True)
class Outcome:
    candidate: Path
    original: str
    final: str
    stages: tuple


def validate_outcome(outcome, candidate, expected):
    if (not isinstance(outcome,Outcome) or outcome.candidate != candidate or outcome.original != expected or
            len(outcome.stages) != 5 or tuple(x.name for x in outcome.stages) != ORDER or
            outcome.stages[0].before != expected or outcome.stages[0].after != outcome.final or
            any(s.before != outcome.final or s.after != outcome.final for s in outcome.stages[1:]) or
            sha(candidate.read_bytes()) != outcome.final):
        raise ValueError('GOLDEN_OUTCOME_OR_IMMUTABILITY')


def execute(root, candidate, *, stage_sink=None):
    binding, policies = load_contract(root)
    original = sha(candidate.read_bytes()); stages = []
    for index, name in enumerate(ORDER):
        before = sha(candidate.read_bytes())
        print('[GOLDEN] ' + name,flush=True)
        try:
            if index == 0:
                changed, count = finalized_bytes(candidate.read_bytes(),policies)
                atomic_finalize(candidate,changed,before); info = {'changed_bytes':count}
            else:
                image = Image(candidate.read_bytes())
                if index == 1: info = layout(image,policies[POLICIES[1]])
                elif index == 2: info = metadata(image,policies[POLICIES[2]],binding)
                elif index == 3: info = binary_acceptance(image,policy=policies[POLICIES[3]])
                else: info = hygiene(root,image,binding,policies)
            after = sha(candidate.read_bytes())
            if index and (before != stages[0].after or after != before):
                raise ValueError('GOLDEN_READ_ONLY_STAGE_MUTATED_CANDIDATE:' + name)
        except BaseException as error:
            if stage_sink is not None:
                stage_sink.append({'name':name,'status':'FAIL','before':before,'error':type(error).__name__+':'+str(error)})
            raise
        stage = Stage(name,before,after); stages.append(stage)
        if stage_sink is not None:
            stage_sink.append({'name':name,'status':'PASS','before':before,'after':after,'info':info})
        print('[GOLDEN PASS] ' + name,flush=True)
    outcome = Outcome(candidate,original,sha(candidate.read_bytes()),tuple(stages))
    validate_outcome(outcome,candidate,original)
    return outcome


def cli(index):
    parser = argparse.ArgumentParser(description=ORDER[index]+';scoped local gate,not whole-chain orrelease acceptance')
    parser.add_argument('dll',type=Path)
    if index == 4: parser.add_argument('--package',type=Path)
    args = parser.parse_args(); root = Path(__file__).resolve().parents[2]
    try:
        binding, policies = load_contract(root); before = sha(args.dll.read_bytes())
        if index == 0:
            data, _ = finalized_bytes(args.dll.read_bytes(),policies); atomic_finalize(args.dll,data,before)
        else:
            image = Image(args.dll.read_bytes())
            if index == 1: layout(image,policies[POLICIES[1]])
            elif index == 2: metadata(image,policies[POLICIES[2]],binding)
            elif index == 3: binary_acceptance(image,policy=policies[POLICIES[3]])
            else: hygiene(root,image,binding,policies,args.package)
            if sha(args.dll.read_bytes()) != before:
                raise ValueError('READ_ONLY_CLI_MUTATED_CANDIDATE')
    except (ValueError,OSError,KeyError,TypeError,UnicodeError,struct.error,zipfile.BadZipFile) as error:
        print(ORDER[index]+': FAIL: '+str(error),file=sys.stderr); return 1
    print(ORDER[index]+': SCOPED_PASS'); return 0
