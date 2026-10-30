"""Verify retained reference sources cannot enter the production link/dependency graph."""
import argparse
import hashlib
import json
from pathlib import Path
import re

POLICY = 'metadata/BUILD_PARTITION_POLICY.json'


def declared_sources(root, variable):
    text = (root / 'cmake/Sources.cmake').read_text(encoding='utf-8')
    match = re.search(r'\bset\(\s*' + re.escape(variable) + r'\b([^)]*)\)', text, re.S)
    if not match:
        raise ValueError('Missing CMake source list: ' + variable)
    return re.findall(r'\bsrc/[A-Za-z0-9_/.-]+\.cpp\b', match.group(1), re.I)


def source_errors(root, policy):
    root = Path(root)
    runtime = declared_sources(root, 'PRELOADER_L_APPLICATION_SOURCES') + ['src/platform/crypt_query_host.cpp']
    vdf = declared_sources(root, 'PRELOADER_L_VDF_SOURCES')
    reference = declared_sources(root, 'PRELOADER_L_REFERENCE_SOURCES')
    errors = []
    for name, actual in [('production_object_sources', runtime), ('vdf_sources', vdf), ('reference_sources', reference)]:
        expected = policy[name]
        if len(actual) != len(set(actual)) or len(expected) != len(set(expected)):
            errors.append('Duplicate source in ' + name)
        if set(actual) != set(expected):
            errors.append('Declared CMake source set differs: ' + name)
    prod = set(runtime + vdf + policy['dll_entry_sources'])
    ref = set(reference)
    if prod & ref:
        errors.append('Production/reference source overlap: ' + ', '.join(sorted(prod & ref)))
    actual = {p.relative_to(root).as_posix()
              for p in (root / 'src').rglob('*')
              if p.is_file() and p.suffix.lower() == '.cpp'}
    if actual != prod | ref:
        errors.append('Partition does not own every application/reference source')
    return errors


def object_pattern(stem):
    # Match both CMake/LLVM foo.cpp.obj and MSVC foo.obj, including archive members.
    return re.compile(r'(?:^|[\s/\\(:])' + re.escape(stem) + r'(?:\.cpp)?\.(?:obj|o)(?=$|[\s):])', re.I | re.M)


def map_errors(text, policy):
    lower = text.lower()
    if re.search(r'address\s+size\s+align\s+out\s+in\s+symbol', lower):
        format_name = 'LLVM_MINGW'
    elif 'preferred load address is' in lower and 'publics by value' in lower:
        format_name = 'MSVC'
    else:
        return ['Unrecognized or empty production linker map'], 'UNKNOWN'
    errors = []
    # Scan large Debug maps once rather than once per reference object.
    stems = {Path(name).stem.lower() for name in policy['reference_sources']} | {'dll_entry', 'crypt_query_host'}
    names = '|'.join(re.escape(stem) for stem in sorted(stems))
    pattern = re.compile(r'(?:^|[\s/\\(:])(' + names + r')(?:\.cpp)?\.(?:obj|o)(?=$|[\s):])', re.I | re.M)
    present = {match.group(1).lower() for match in pattern.finditer(text)}
    for name in policy['reference_sources']:
        if Path(name).stem.lower() in present:
            errors.append('Reference object present in production map: ' + name)
    for name in policy['reference_targets']:
        if name.lower() in lower:
            errors.append('Reference target/archive present in production map: ' + name)
    for stem in ['dll_entry', 'crypt_query_host']:
        if stem not in present:
            errors.append('Production object evidence missing: ' + stem)
    return errors, format_name


def load_generated_graph(build, config):
    path = Path(build) / ('partition.' + config + '.targets')
    if not path.is_file():
        raise ValueError('CMake generated target graph is missing: ' + str(path))
    targets, current = {}, None
    for line in path.read_text(encoding='utf-8').splitlines():
        if line.startswith('TARGET\t'):
            name = line.split('\t', 1)[1]
            if name in targets:
                raise ValueError('Duplicate generated target: ' + name)
            current = {'name': name, 'id': name, 'sources': [], 'dependencies': []}
            targets[name] = current
        elif line.startswith('TYPE\t') and current is not None:
            current['type'] = line.split('\t', 1)[1]
        elif line.startswith('DEPS\t') and current is not None:
            current['dependencies'] = [{'id': v} for v in line.split('\t', 1)[1].split(';') if v]
        elif line.startswith('SOURCES\t') and current is not None:
            current['sources'] = [{'path': v} for v in line.split('\t', 1)[1].split(';') if v]
        elif line == 'END':
            current = None
        elif line:
            raise ValueError('Malformed generated target graph')
    if not targets or current is not None:
        raise ValueError('Empty or incomplete generated target graph')
    return targets


def target_closure(targets, root):
    by_id = {v['id']: k for k, v in targets.items()}
    todo, found = [root], set()
    while todo:
        name = todo.pop()
        if name in found:
            continue
        found.add(name)
        todo.extend(by_id[d['id']] for d in targets[name].get('dependencies', []) if d['id'] in by_id)
    return found


def codemodel_errors(targets, policy):
    errors = []
    forbidden = set(policy['reference_targets'] + policy['source_api_targets'])
    classified = set(policy['production_test_targets'] + policy['reference_test_targets'] +
                     policy['source_api_targets'] + policy.get('vendor_comparison_targets', []))
    for name, data in targets.items():
        if data.get('type') == 'EXECUTABLE' and name not in classified:
            errors.append('Unclassified executable test/tool target: ' + name)
    for name in policy['production_targets']:
        if name not in targets:
            errors.append('Required production target missing: ' + name)
    for name in policy['production_targets'] + policy['production_test_targets']:
        if name not in targets:
            continue  # BUILD_TESTING=OFF intentionally has no test targets.
        closure = target_closure(targets, name)
        # A generation-time CMake graph may omit imported or out-of-directory
        # targets. Still reject a *named* forbidden dependency even when its
        # target record is absent; do not confuse it with a harmless external.
        referenced_ids = {edge['id'] for node in closure
                          for edge in targets[node].get('dependencies', [])}
        overlap = (closure | referenced_ids) & forbidden
        if overlap:
            errors.append('Production target depends on nonproduction target: ' + name + ' -> ' + ', '.join(sorted(overlap)))
        for dependency in closure:
            for source in targets[dependency].get('sources', []):
                path = source['path'].replace('\\', '/')
                for reference in policy['reference_sources']:
                    if (path == reference or path.endswith('/' + reference) or
                            object_pattern(Path(reference).stem).search(path)):
                        errors.append('Reference source/object in production closure: ' + name + ' -> ' + path)
    for name, key in [('preloader_l_objects', 'production_object_sources'), ('preloader_l_vdf_objects', 'vdf_sources'), ('preloader_l', 'dll_entry_sources'), ('preloader_l_reference_objects', 'reference_sources')]:
        if name not in targets:
            continue
        actual = {s['path'].replace('\\', '/') for s in targets[name].get('sources', [])
                  if s['path'].lower().endswith('.cpp') and not s.get('isGenerated')}
        if actual != set(policy[key]):
            errors.append('Generated target source set differs: ' + name)
    if 'preloader_l_source_api' in targets and 'preloader_l_reference_objects' not in target_closure(targets, 'preloader_l_source_api'):
        errors.append('Source API lost the retained reference object payload')
    for name in policy['reference_test_targets']:
        if name in targets and 'preloader_l_reference_models' not in target_closure(targets, name):
            errors.append('Reference test lost its model library: ' + name)
    return errors


def read_map(path):
    data = Path(path).read_bytes()
    return data.decode('utf-16') if data.startswith((b'\xff\xfe', b'\xfe\xff')) else data.decode('utf-8', errors='replace')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    ap.add_argument('--source-only', action='store_true')
    ap.add_argument('--build-dir', type=Path)
    ap.add_argument('--config', default='')
    ap.add_argument('--map', type=Path)
    ap.add_argument('--binary', type=Path)
    ap.add_argument('--report', type=Path)
    args = ap.parse_args()
    errors, format_name = [], None
    try:
        root = args.root.resolve()
        policy = json.loads((root / POLICY).read_text(encoding='utf-8'))
        errors += source_errors(root, policy)
        if not args.source_only:
            if not args.build_dir or not args.map or not args.binary:
                raise ValueError('Build proof requires --build-dir, --map and --binary')
            errors += codemodel_errors(load_generated_graph(args.build_dir, args.config), policy)
            findings, format_name = map_errors(read_map(args.map), policy)
            errors += findings
            if not args.binary.is_file():
                errors.append('Linked production binary missing')
    except (OSError, ValueError, KeyError, TypeError) as error:
        errors.append(str(error))
    result = {'schema': 'surge.build-partition-check.v1', 'classification': 'NOT_RELEASE',
              'result': 'FAIL' if errors else 'PASS_SOURCE_PARTITION' if args.source_only else 'PASS_PRODUCTION_BOUNDARY',
              'map_format': format_name, 'errors': errors,
              'native_msvc_and_game_acceptance': 'SEPARATE_REQUIRED_EVIDENCE'}
    if not errors and not args.source_only:
        result['binary_sha256'] = hashlib.sha256(args.binary.read_bytes()).hexdigest()
        result['map_sha256'] = hashlib.sha256(args.map.read_bytes()).hexdigest()
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    for error in errors:
        print(' -', error)
    print('BUILD_PARTITION:', result['result'])
    return int(bool(errors))


if __name__ == '__main__':
    raise SystemExit(main())
