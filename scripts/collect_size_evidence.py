# SPDX-License-Identifier: GPL-3.0-only

"""Collect read-only DLL/MAP/COFF evidence from an existing local build.

Object section sizes are pre-link contributions, never final function sizes.
No build, binary execution, finalization, publication or source repair is performed.
"""
import argparse
import csv
import hashlib
import io
import json
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import zipfile
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
from project_files import verify_manifest
from check_build_partition import load_generated_graph
from verify_corvexon_golden_trail import PE, PEError, u32


def sha(data):
    return hashlib.sha256(data).hexdigest()


def coff_sections(data):
    """Read ordinary/BigObj COFF headers; reject unknown or truncated objects."""
    if len(data) < 20:
        raise ValueError('Truncated COFF header')
    if data[:4] == b'\0\0\xff\xff':
        if len(data) < 56 or struct.unpack_from('<H', data, 4)[0] < 2:
            raise ValueError('Import/LTCG object is not supported COFF BigObj')
        if data[12:28] != bytes.fromhex('c7a1bad1eebaa94baf20faf66aa4dcb8'):
            raise ValueError('Unknown COFF BigObj class')
        count, symbols, symbol_count = struct.unpack_from('<III', data, 44)
        offset, symbol_size = 56, 20
        machine = struct.unpack_from('<H', data, 6)[0]
    else:
        machine, count = struct.unpack_from('<HH', data)
        symbols, symbol_count = struct.unpack_from('<II', data, 8)
        optional_size = struct.unpack_from('<H', data, 16)[0]
        if optional_size:
            raise ValueError('Expected an object, not a PE optional header')
        offset, symbol_size = 20, 18
    if machine not in (0x8664, 0x14c, 0xaa64):
        raise ValueError('Unsupported object machine')
    if not count or count > (len(data) - offset) // 40:
        raise ValueError('Truncated or empty COFF section table')
    strings = symbols + symbol_count * symbol_size
    string_end = 0
    if symbols:
        if strings > len(data) - 4:
            raise ValueError('Truncated COFF symbol/string table')
        length = struct.unpack_from('<I', data, strings)[0]
        if length < 4 or length > len(data) - strings:
            raise ValueError('Truncated COFF string data')
        string_end = strings + length
    rows = []
    for index in range(count):
        at = offset + index * 40
        name = data[at:at + 8].split(b'\0', 1)[0]
        if name.startswith(b'/'):
            try:
                start = strings + int(name[1:])
            except ValueError as error:
                raise ValueError('Invalid long COFF section name') from error
            if not symbols or start < strings + 4 or start >= string_end:
                raise ValueError('Long section name outside string table')
            end = data.find(b'\0', start, string_end)
            if end < 0:
                raise ValueError('Unterminated COFF section name')
            name = data[start:end]
        size, raw = struct.unpack_from('<II', data, at + 16)
        flags = struct.unpack_from('<I', data, at + 36)[0]
        # Uninitialized sections can have a logical size and no raw file payload.
        if raw and (raw > len(data) or size > len(data) - raw):
            raise ValueError('COFF section payload outside object')
        rows.append({'name': name.decode('ascii', 'replace'), 'raw_bytes': size if raw else 0,
                     'flags': flags, 'comdat': bool(flags & 0x1000)})
    return {'machine': hex(machine), 'sections': rows}


def object_owner(path, build):
    return next((p[:-4] for p in path.relative_to(build).parts if p.endswith('.dir')), None)


def vs_object_paths(project, config, sources):
    """Resolve CMake-generated VS IntDir/ObjectFileName; reject unknown macros."""
    tree = ET.parse(project)
    target = project.stem
    def applies(condition):
        return not condition or ("'" + config.lower() + "|x64'") in condition.lower()
    def expand(value, intdir=''):
        for key, text in [('$(Configuration)', config), ('$(Platform)', 'x64'),
                          ('$(ProjectName)', target), ('$(ProjectDir)', str(project.parent) + '/'),
                          ('$(IntDir)', intdir)]:
            value = value.replace(key, text)
        if '$(' in value:
            raise ValueError('Unresolved VS object path macro: ' + value)
        return value.replace('\\', '/')
    intdirs = []
    for group in tree.getroot():
        if group.tag.rsplit('}', 1)[-1] == 'PropertyGroup' and applies(group.get('Condition')):
            intdirs.extend(c.text for c in group if c.tag.rsplit('}', 1)[-1] == 'IntDir' and c.text and applies(c.get('Condition')))
    if not intdirs:
        raise ValueError('VS IntDir is absent for ' + target)
    intdir = expand(intdirs[-1])
    result = {}
    for item in tree.iter():
        if item.tag.rsplit('}', 1)[-1] != 'ClCompile' or not item.get('Include'):
            continue
        source = (project.parent / expand(item.get('Include'))).resolve()
        if source not in sources:
            continue
        names = [c.text for c in item if c.tag.rsplit('}', 1)[-1] == 'ObjectFileName' and c.text and applies(c.get('Condition'))]
        name = expand(names[-1] if names else '$(IntDir)', intdir)
        path = project.parent / name
        if name.endswith('/'):
            path /= source.stem + '.obj'
        result[source] = path.resolve()
    return result


def current_objects(root, build, allowed, config):
    """Use generated object inputs/commands, not every stale .obj on disk."""
    graph = load_generated_graph(build, config)
    objects = set()
    for target in allowed:
        for source in graph.get(target, {}).get('sources', []):
            path = Path(source['path'])
            if path.suffix.lower() == '.obj':
                objects.add((path if path.is_absolute() else build / path).resolve())
    commands = {}
    database = build / 'compile_commands.json'
    if database.is_file():
        for item in json.loads(database.read_text(encoding='utf-8')):
            if not item.get('output'):
                continue
            base = Path(item['directory'])
            output = (base / item['output']).resolve()
            owner = object_owner(output, build)
            if owner in allowed:
                commands[owner, (base / item['file']).resolve()] = output
    for target in allowed:
        node = graph.get(target)
        if not node:
            continue
        sources = {(Path(s['path']) if Path(s['path']).is_absolute() else root / s['path']).resolve()
                   for s in node['sources'] if Path(s['path']).suffix.lower() in ('.cpp', '.c', '.cxx')}
        if node.get('type') == 'OBJECT_LIBRARY':
            selected = {p for p in objects if object_owner(p, build) == target}
            if len(selected) != len(sources):
                raise ValueError('Expanded object-library inputs do not match source count: ' + target)
            continue
        if not sources:
            continue
        resolved = {s: commands[target, s] for s in sources if (target, s) in commands}
        remaining = sources - resolved.keys()
        if remaining:
            project = build / (target + '.vcxproj')
            if not project.is_file():
                raise ValueError('No compiler output paths or supported VS project for ' + target)
            resolved.update(vs_object_paths(project, config, remaining))
        if sources - resolved.keys():
            raise ValueError('Could not resolve every current compiler object for ' + target)
        objects.update(resolved.values())
    for path in objects:
        if not path.is_relative_to(build) or not path.is_file():
            raise ValueError('Current object missing or outside selected build: ' + str(path))
    return sorted(objects)


def collect(root, build, config, dll, output, with_symbols=False):
    root, build, output = root.resolve(), build.resolve(), output.resolve()
    if root / 'build' not in output.parents:
        raise ValueError('Write evidence inside the selected project build/ directory')
    if output.exists():
        raise ValueError('Evidence output exists; choose a new path to preserve it')
    errors = verify_manifest(root)
    if errors:
        raise ValueError('Source manifest differs: ' + '; '.join(map(str, errors[:5])))
    policy = json.loads((root / 'metadata/BUILD_PARTITION_POLICY.json').read_text(encoding='utf-8'))
    payload = {}
    def add(name, path):
        data = path.read_bytes()
        if name in payload:
            raise ValueError('Duplicate evidence member')
        payload[name] = data
        return {'path': name, 'bytes': len(data), 'sha256': sha(data)}
    records = {}
    for name in ['ACTIVE_SOURCE_MANIFEST.json', 'BUILD_PARTITION_POLICY.json', 'PUBLIC_DLL_ABI.json']:
        records[name] = add('contracts/' + name, root / 'metadata' / name)
    if dll is None:
        candidate = root / 'dist/preloader_l.dll'
        dll = candidate if candidate.is_file() else build / 'preloader.unsigned.dll'
    dll = dll.resolve()
    pe = PE(dll)
    dll_record = add('binary/' + dll.name, dll)
    section_rows = []
    for index, (name, va, span, raw, size, flags) in enumerate(pe.sections):
        virtual_size = u32(pe.b, pe.opt + pe.optsz + index * 40 + 8)
        section_rows.append({'name': name, 'virtual_bytes': virtual_size, 'raw_bytes': size,
                             'rva': va, 'sha256_raw': sha(pe.b[raw:raw + size])})
    maps = sorted(build.rglob('preloader.production.' + config + '.map'))
    if len(maps) != 1:
        raise ValueError('Expected exactly one matching production MAP; build the selected configuration first')
    map_record = add('link/' + maps[0].name, maps[0])
    for name in ['production-boundary.' + config + '.json', 'partition.' + config + '.targets']:
        p = build / name
        if p.is_file():
            records[name] = add('link/' + name, p)
    receipt = root / 'dist/LOCAL_DIST_RECEIPT.json'
    if receipt.is_file():
        records['dist_receipt'] = add('link/LOCAL_DIST_RECEIPT.json', receipt)
    compilation = build / 'compile_commands.json'
    if compilation.is_file():
        commands = json.loads(compilation.read_text(encoding='utf-8'))
        selected = [x for x in commands if Path(x['file']).resolve().is_relative_to(root / 'src')]
        payload['commands/compile_commands.json'] = (json.dumps(selected, indent=2) + '\n').encode()
    targets = set(policy['production_targets'] + policy['reference_targets'] + policy['source_api_targets'])
    targets.update(['preloader_l_minhook', 'preloader_l_tinyxml2'])
    objects, failures = [], []
    dumpbin = shutil.which('dumpbin') if with_symbols else None
    if with_symbols and not dumpbin:
        raise ValueError('--with-symbols requires dumpbin in the VS developer environment')
    configurations = {'debug', 'release', 'relwithdebinfo', 'minsizerel'}
    for path in current_objects(root, build, targets, config):
        relative = path.relative_to(build)
        target = next((p[:-4] for p in relative.parts if p.endswith('.dir')), None)
        if target not in targets:
            continue
        if any(p.lower() in configurations and p.lower() != config.lower() for p in relative.parts):
            continue
        data = path.read_bytes()
        row = {'object': relative.as_posix(), 'target': target, 'file_bytes': len(data), 'sha256': sha(data)}
        try:
            parsed = coff_sections(data)
            row.update(parsed)
            row['code_raw_bytes'] = sum(s['raw_bytes'] for s in parsed['sections'] if s['flags'] & 0x20)
            row['rdata_raw_bytes'] = sum(s['raw_bytes'] for s in parsed['sections'] if s['name'].startswith('.rdata'))
            row['comdat_sections'] = sum(s['comdat'] for s in parsed['sections'])
        except ValueError as error:
            row['unsupported'] = str(error)
            failures.append({'object': relative.as_posix(), 'error': str(error)})
        objects.append(row)
        if dumpbin:
            cp = subprocess.run([dumpbin, '/nologo', '/headers', '/symbols', str(path)],
                                capture_output=True, timeout=60)
            if cp.returncode:
                raise ValueError('dumpbin failed on ' + relative.as_posix())
            payload['symbols/' + relative.as_posix().replace('/', '__') + '.txt'] = cp.stdout + cp.stderr
    if not objects:
        raise ValueError('No compiler objects found; use the original build directory, not dist alone')
    for pattern in ['**/CL.command.1.tlog', '**/link.command.1.tlog']:
        for path in sorted(build.glob(pattern)):
            relative = path.relative_to(build)
            target = next((p[:-4] for p in relative.parts if p.endswith('.dir')), None)
            # VS tlog paths can instead contain <target>.tlog without a .dir.
            if target in targets or any(p.removesuffix('.tlog') in targets for p in relative.parts):
                add('commands/' + relative.as_posix().replace('/', '__'), path)
    csv_out = io.StringIO(newline='')
    writer = csv.writer(csv_out)
    writer.writerow(['object', 'target', 'file_bytes', 'code_raw_bytes_before_link', 'rdata_raw_bytes_before_link', 'comdat_sections', 'sha256', 'unsupported'])
    for row in sorted(objects, key=lambda r: r.get('code_raw_bytes', 0), reverse=True):
        writer.writerow([row.get(k, '') for k in ['object', 'target', 'file_bytes', 'code_raw_bytes', 'rdata_raw_bytes', 'comdat_sections', 'sha256', 'unsupported']])
    payload['analysis/object_sections.csv'] = csv_out.getvalue().encode()
    report = {'schema': 'surge.size-evidence.v1', 'result': 'PARTIAL_UNSUPPORTED_OBJECTS' if failures else 'COLLECTED',
              'classification': 'READ_ONLY_LOCAL_EVIDENCE_NOT_SHIPPING_ACCEPTANCE',
              'configuration': config, 'binary': dll_record, 'binary_sections': section_rows,
              'exports': pe.exports(), 'production_map': map_record, 'contracts_and_reports': records,
              'objects': objects, 'unsupported_objects': failures,
              'limits': ['OBJ section bytes include pre-link dead/duplicate COMDATs; do not sum them as final DLL function contribution.',
                         'A public-symbol MAP interval is not an exact function size.',
                         'Collection records files as found; fresh matching builds and receipts are needed to exclude stale artifacts.',
                         'This tool does not execute Windows/game acceptance or change linker settings.']}
    payload['analysis/report.json'] = (json.dumps(report, indent=2) + '\n').encode()
    payload['SHA256SUMS'] = ''.join(sha(data) + '  ' + name + '\n' for name, data in sorted(payload.items())).encode()
    output.parent.mkdir(parents=True, exist_ok=True)
    partial = output.with_suffix(output.suffix + '.partial')
    try:
        with zipfile.ZipFile(partial, 'w', zipfile.ZIP_DEFLATED) as archive:
            for name, data in sorted(payload.items()):
                archive.writestr(name, data)
        partial.replace(output)
    finally:
        partial.unlink(missing_ok=True)
    digest = sha(output.read_bytes())
    Path(str(output) + '.sha256').write_text(digest + '  ' + output.name + '\n', encoding='utf-8')
    return {'result': report['result'], 'objects': len(objects), 'unsupported_objects': len(failures),
            'path': str(output), 'bytes': output.stat().st_size, 'sha256': digest}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--config', default='Release')
    parser.add_argument('--dll', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--with-symbols', action='store_true', help='Also capture real dumpbin headers/symbols when run in VS developer environment')
    args = parser.parse_args()
    try:
        result = collect(args.root, args.build_dir, args.config, args.dll, args.output, args.with_symbols)
        print(json.dumps(result, indent=2))
    except (OSError, ValueError, KeyError, PEError, subprocess.SubprocessError) as error:
        parser.exit(1, 'SIZE_EVIDENCE: ' + str(error) + '\n')


if __name__ == '__main__':
    main()
