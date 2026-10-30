"""Produce a per-TU symbol/test/source-API usage matrix from built COFF objects.

Uses LLVM inspection tools and the generated CMake target graph. Object lookup
supports the Ninja CMake layout; unsupported layouts fail rather than guessing.
This report is static evidence, not a claim of complete game-path coverage.
"""
import argparse
import collections
import csv
import hashlib
import json
from pathlib import Path
import re
import subprocess
from check_build_partition import load_generated_graph


def run(tool, *args):
    return subprocess.check_output([str(tool), *map(str, args)], text=True, errors='replace')


def symbols(nm, path):
    defined, undefined = set(), set()
    for line in run(nm, '--extern-only', '--format=posix', path).splitlines():
        cells = line.split()
        if len(cells) < 2 or 'preloader_l' not in cells[0]:
            continue
        name = cells[0].removeprefix('__imp_')
        (undefined if cells[1] == 'U' else defined).add(name)
    return defined, undefined


def object_path(root, build, target, source):
    source_path = Path(source)
    if source_path.is_absolute():
        source = source_path.relative_to(root).as_posix()
    result = build / ('CMakeFiles/' + target + '.dir/' + source + '.obj')
    if not result.is_file():
        raise ValueError('Built object missing or unsupported object layout: ' + str(result))
    return result


def build_report(root, build, config, nm, objdump, readobj):
    policy = json.loads((root / 'metadata/BUILD_PARTITION_POLICY.json').read_text(encoding='utf-8'))
    targets = load_generated_graph(build, config)
    ref = set(policy['reference_sources'])
    production = policy['production_object_sources'] + policy['vdf_sources'] + policy['dll_entry_sources']
    records = {}
    for source in production + policy['reference_sources']:
        target = ('preloader_l_reference_objects' if source in ref else
                  'preloader_l_vdf_objects' if source in policy['vdf_sources'] else
                  'preloader_l' if source in policy['dll_entry_sources'] else 'preloader_l_objects')
        path = object_path(root, build, target, source)
        defined, undefined = symbols(nm, path)
        sections = run(objdump, '-h', path)
        startup = [m.group(1) for m in re.finditer(r'^\s*\d+\s+(\.CRT\S*|\.tls\S*)\s+([0-9a-fA-F]+)', sections, re.M) if int(m.group(2), 16)]
        records[source] = {'defined': sorted(defined), 'undefined': sorted(undefined),
                           'initializer_sections': startup,
                           'source_sha256': hashlib.sha256((root / source).read_bytes()).hexdigest()}
    prod_defs = set(s for n in production for s in records[n]['defined'])
    ref_providers = collections.defaultdict(set)
    for name in ref:
        for symbol in records[name]['defined']:
            if symbol not in prod_defs:
                ref_providers[symbol].add(name)
    incoming = []
    for name in production:
        for symbol in set(records[name]['undefined']) & ref_providers.keys():
            incoming.append({'caller': name, 'symbol': symbol, 'providers': sorted(ref_providers[symbol])})
    test_targets = (policy['production_test_targets'] + policy['reference_test_targets'] +
                    [n for n in policy['source_api_targets'] if n.endswith('_host')] + policy['vendor_comparison_targets'])
    test_refs = {}
    for name in test_targets:
        if name not in targets:
            continue
        refs = set()
        for source in targets[name]['sources']:
            path = source['path'].replace('\\', '/')
            if path.endswith(('.cpp', '.c', '.cxx')):
                _, undefined = symbols(nm, object_path(root, build, name, path))
                refs.update(undefined & ref_providers.keys())
        test_refs[name] = refs
    exports = set()
    source_api = build / 'preloader.source.dll'
    if source_api.is_file():
        exports = set(re.findall(r'^\s+Name: (.*)$', run(readobj, '--coff-exports', source_api), re.M))
    adjacency = {}
    for name in ref:
        adjacency[name] = set(p for s in records[name]['undefined'] for p in ref_providers.get(s, ())) - {name}
    callers = collections.defaultdict(set)
    direct_tests = collections.defaultdict(set)
    for name, syms in test_refs.items():
        todo = [p for s in syms for p in ref_providers[s]]
        for p in todo:
            direct_tests[p].add(name)
        seen = set()
        while todo:
            node = todo.pop()
            if node in seen:
                continue
            seen.add(node); callers[node].add(name); todo.extend(adjacency[node])
    rows = []
    for name in policy['reference_sources']:
        rec = records[name]
        unique = sorted(s for s in rec['defined'] if s in ref_providers)
        vt = [s for s in unique if s.startswith(('_ZTV', '??_7'))]
        rows.append({'source': name, 'source_sha256': rec['source_sha256'],
                     'decision': 'PRESERVED_REFERENCE_TEST_AND_SOURCE_API; EXCLUDED_FROM_PRODUCTION_INPUTS',
                     'reference_only_defined_symbols': unique, 'vtable_symbols': vt,
                     'initializer_sections': rec['initializer_sections'],
                     'production_callers': [v['caller'] for v in incoming if name in v['providers']],
                     'reference_callers': sorted(n for n in ref if name in adjacency[n]),
                     'direct_test_targets': sorted(direct_tests[name]),
                     'transitive_test_targets': sorted(callers[name]),
                     'source_api_exported_symbols': sorted(set(unique) & exports)})
    bindings = []
    for name in production:
        for index, line in enumerate((root / name).read_text(encoding='utf-8').splitlines(), 1):
            if re.search(r'GetProcAddress|CreateThread|std::thread|rebuilt_address|GetModuleHandleEx', line):
                bindings.append({'source': name, 'line': index, 'text': line.strip()})
    return {'schema': 'surge.translation-unit-usage.v1', 'classification': 'NOT_RELEASE',
            'analysis': 'EXTERNAL_COFF_SYMBOL_REFERENCES_PLUS_TARGET_AND_BINDING_REVIEW; NOT_EXHAUSTIVE_DYNAMIC_PROOF',
            'production_tu_count': len(production), 'reference_tu_count': len(ref),
            'reference_only_symbol_count': len(ref_providers),
            'production_undefined_reference_only': incoming,
            'reference_initializer_sections': {n: records[n]['initializer_sections'] for n in ref if records[n]['initializer_sections']},
            'production_initializer_sections': {n: records[n]['initializer_sections'] for n in production if records[n]['initializer_sections']},
            'production_binding_sites_for_review': bindings,
            'root_contract': ['DllMain@1', 'SURGE@2', 'DllShutdown@3', 'DLL worker entry',
                              'registered native SDK callbacks and concrete vtables', 'CRT/TLS initializer sections'],
            'native_msvc_and_game_acceptance': 'SEPARATE_REQUIRED_EVIDENCE',
            'all_runtime_and_reference_source_sha256': {n: r['source_sha256'] for n, r in records.items()},
            'source_api_export_count': len(exports), 'rows': rows}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    ap.add_argument('--build-dir', type=Path, required=True)
    ap.add_argument('--config', default='Release')
    ap.add_argument('--nm', type=Path, required=True)
    ap.add_argument('--objdump', type=Path, required=True)
    ap.add_argument('--readobj', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--csv', type=Path)
    args = ap.parse_args()
    report = build_report(args.root.resolve(), args.build_dir.resolve(), args.config, args.nm, args.objdump, args.readobj)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    if args.csv:
        args.csv.parent.mkdir(parents=True, exist_ok=True)
        with args.csv.open('w', newline='', encoding='utf-8') as stream:
            writer = csv.writer(stream)
            writer.writerow(['source', 'reference_symbols', 'production_callers', 'reference_callers', 'direct_tests', 'transitive_tests', 'source_api_exports', 'initializer_sections', 'decision'])
            for r in report['rows']:
                writer.writerow([r['source'], len(r['reference_only_defined_symbols']), len(r['production_callers']),
                                 ';'.join(r['reference_callers']), ';'.join(r['direct_test_targets']),
                                 ';'.join(r['transitive_test_targets']), len(r['source_api_exported_symbols']),
                                 ';'.join(r['initializer_sections']), r['decision']])
    print('TRANSLATION_UNIT_USAGE:', report['reference_tu_count'], 'retained reference TUs;',
          len(report['production_undefined_reference_only']), 'production dependencies on reference-only symbols')
    return int(bool(report['production_undefined_reference_only'] or report['reference_initializer_sections']))


if __name__ == '__main__':
    raise SystemExit(main())
