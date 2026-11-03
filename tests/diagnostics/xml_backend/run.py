# SPDX-License-Identifier: GPL-3.0-only

"""Run the isolated, pinned XML comparison; never configure production targets."""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import platform
import sys
import statistics
import subprocess
import urllib.request

HERE = Path(__file__).resolve().parent
REPOSITORY = HERE.parents[2]
REFERENCE = 'a34a0536f0980b38d77d029f11fa3720dcaedc6e'
PUGIXML = 'f0a3faa76e04f35110257ca8713363dee3c890e8'
PUGI_FILES = {
    'src/pugixml.cpp': '5a8e166d87dabaf67047d4fa124b06a9f1c9b36236fd5de5fbf054f9c0767a2c',
    'src/pugixml.hpp': '0b7c94701df3d50981ffafe57f177c77b940e347c6e5d01073e4161f61299212',
    'src/pugiconfig.hpp': 'e6c51fbc9a765608bd214ea47d229d3bbd6b650036cdffc94b96b498c202495c',
    'LICENSE.md': '90974fbb120e966b452654eaae3ba6ee55f3647db3b28918a7649733ee99da97',
}


def output_path(path):
    path = path.resolve()
    if not path.is_relative_to((REPOSITORY / 'build').resolve()):
        raise ValueError('Diagnostic outputs must be below this repository\'s ignored build directory')
    return path


def verify_inputs(reference, pugi, fetch=False):
    def git(*args):
        return subprocess.check_output(['git', '-C', str(reference), *args], text=True).strip()
    if Path(git('rev-parse', '--show-toplevel')).resolve() != reference:
        raise ValueError('Reference root must be its own exact checkout')
    if git('rev-parse', 'HEAD') != REFERENCE:
        raise ValueError('Reference checkout is not the reviewed PR #108 commit ' + REFERENCE)
    # Reject tracked source drift, including staged edits, before compiling it.
    if git('diff', '--name-only', 'HEAD', '--', 'include', 'src', 'tests', 'third_party/tinyxml2'):
        raise ValueError('Reference inputs have local tracked changes')
    if fetch:
        output_path(pugi)
    for name, expected in PUGI_FILES.items():
        path = pugi / name
        if fetch and not path.exists():
            url = f'https://raw.githubusercontent.com/zeux/pugixml/{PUGIXML}/{name}'
            with urllib.request.urlopen(url, timeout=60) as response:
                data = response.read()
            if hashlib.sha256(data).hexdigest() != expected:
                raise ValueError('Downloaded pugixml input hash mismatch: ' + name)
            path.parent.mkdir(parents=True, exist_ok=True)
            with path.open('xb') as stream:
                stream.write(data)
        if hashlib.sha256(path.read_bytes()).hexdigest() != expected:
            raise ValueError('External pugixml input hash mismatch: ' + name)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference-root', type=Path, required=True)
    parser.add_argument('--pugixml-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=REPOSITORY / 'build/xml-backend-comparison')
    parser.add_argument('--fetch-pugixml', action='store_true', help='Explicitly download hash-checked diagnostic inputs')
    parser.add_argument('--verify-only', action='store_true')
    parser.add_argument('--sanitizers', action='store_true')
    parser.add_argument('--configuration', choices=['Debug', 'Release'], default='Release')
    parser.add_argument('--mutations', type=int, default=100000)
    parser.add_argument('--holdout-mutations', type=int, default=500000)
    parser.add_argument('--timing-iterations', type=int, default=2000)
    args = parser.parse_args()
    if args.mutations < 1 or args.holdout_mutations < 1 or args.timing_iterations < 1:
        parser.error('Iteration counts must be positive')
    reference, pugi = args.reference_root.resolve(), args.pugixml_root.resolve()
    verify_inputs(reference, pugi, args.fetch_pugixml)
    if args.verify_only:
        print('PASS: pinned PR #108 reference and exact owner-upload pugixml core/license bytes')
        return
    output = output_path(args.output)
    output.mkdir(parents=True, exist_ok=True)
    build = output / 'binaries'
    report = {'reference_commit': REFERENCE, 'pugixml_commit': PUGIXML,
              'pugixml_files': PUGI_FILES, 'host': platform.platform(),
              'harness_files': {path.name: hashlib.sha256(path.read_bytes()).hexdigest()
                                for path in sorted(HERE.iterdir()) if path.is_file()},
              'configuration': args.configuration, 'sanitizers': args.sanitizers,
              'mutations': args.mutations, 'holdout_mutations': args.holdout_mutations,
              'seed': 85621504, 'holdout_seed': 202892583, 'commands': [],
              'production_migration': 'NOT_SWITCHED', 'complete': False}

    def run(name, command, allowed=(0,), timeout=300, reference_oom_measurement=False):
        environment = os.environ.copy()
        if args.sanitizers:
            environment['ASAN_OPTIONS'] = 'detect_leaks=1:halt_on_error=1'
            environment['UBSAN_OPTIONS'] = 'halt_on_error=1:print_stacktrace=1'
        result = subprocess.run([str(arg) for arg in command], stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, timeout=timeout, env=environment)
        (output / (name + '.log')).write_bytes(result.stdout)
        report['commands'].append({'name': name, 'arguments': [str(arg) for arg in command],
                                   'exit_code': result.returncode})
        # Publish partial results too: incomplete execution is never a comparison PASS.
        (output / 'summary.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        print(f'{name}: exit {result.returncode}', flush=True)
        if args.sanitizers and any(marker in result.stdout for marker in
                [b'ERROR: AddressSanitizer', b'AddressSanitizer:DEADLYSIGNAL',
                 b'runtime error:']):
            raise RuntimeError(f'{name} reported a sanitizer error; see its retained log')
        if args.sanitizers and b'LeakSanitizer' in result.stdout and not reference_oom_measurement:
            raise RuntimeError(f'{name} reported a leak; see its retained log')
        if result.returncode not in allowed:
            raise RuntimeError(f'{name} failed; see {output / (name + ".log")}')
        return result

    query = build / '.cmake/api/v1/query/codemodel-v2'
    query.parent.mkdir(parents=True, exist_ok=True)
    query.touch()
    configure = ['cmake', '-S', HERE, '-B', build, f'-DXML_REFERENCE_ROOT={reference.as_posix()}',
                 f'-DPUGIXML_ROOT={pugi.as_posix()}', f'-DCMAKE_BUILD_TYPE={args.configuration}',
                 f'-DXML_COMPARISON_SANITIZERS={"ON" if args.sanitizers else "OFF"}']
    run('configure', configure)
    # Evaluate the generated target graph, including transitive dependencies.
    # The candidate-only executables must work without the TinyXML2 oracle.
    reply = build / '.cmake/api/v1/reply'
    index = json.loads(max(reply.glob('index-*.json')).read_text(encoding='utf-8'))
    model = json.loads((reply / index['reply']['codemodel-v2']['jsonFile']).read_text(encoding='utf-8'))
    linkage = []
    for configuration in model['configurations']:
        targets = {target['id']: json.loads((reply / target['jsonFile']).read_text(encoding='utf-8'))
                   for target in configuration['targets']}
        for target in targets.values():
            if not target['name'].startswith('candidate_'):
                continue
            pending, seen = [target['id']], set()
            while pending:
                node = pending.pop()
                if node in seen:
                    continue
                seen.add(node)
                pending.extend(edge['id'] for edge in targets[node].get('dependencies', []))
            reachable = sorted(targets[node]['name'] for node in seen)
            if 'comparison_pugi' not in reachable or any(name in reachable for name in
                    ['comparison_tiny', 'comparison_reference']):
                raise RuntimeError('Candidate target depends on the oracle or omits pugixml: ' + target['name'])
            linkage.append({'configuration': configuration['name'], 'target': target['name'], 'reachable': reachable})
    if len(linkage) < 2:
        raise RuntimeError('Candidate-only target graph is incomplete')
    report['candidate_linkage'] = linkage
    (output / 'candidate-linkage.json').write_text(json.dumps(linkage, indent=2) + '\n', encoding='utf-8')
    run('build', ['cmake', '--build', build, '--config', args.configuration, '--parallel', '2'], timeout=600)

    def binary(name):
        suffix = '.exe' if os.name == 'nt' else ''
        direct = build / (name + suffix)
        return direct if direct.exists() else build / args.configuration / (name + suffix)

    corpus = reference / 'tests/fixtures/xml_reader/optimization-corpus.txt'
    baseline = reference / 'tests/fixtures/xml_reader/optimization-baseline.txt'
    run('reference-reader', [binary('reference_reader')])
    run('reference-corpus', [binary('reference_corpus'), corpus, baseline])
    candidate = run('candidate-reader', [binary('candidate_reader')])
    report['candidate_reader_contract'] = 'PASS'
    run('candidate-corpus', [binary('candidate_corpus'), corpus, baseline])
    if sys.platform == 'win32':
        for profile in ['reference', 'candidate']:
            run(profile + '-dispatch', [binary(profile + '_dispatch')])
            run(profile + '-storage', [binary(profile + '_storage'), output / (profile + '-storage-fixtures')])
    suite = binary('comparison_suite')
    run('corpus', [suite, 'corpus', corpus, output / 'corpus.tsv'])
    for operation in ['compatibility', 'safety', 'protocol', 'protocol-edges', 'storage']:
        run(operation, [suite, operation, output / (operation + '.tsv')])
    run('ownership', [suite, 'ownership'])
    run('mutations', [suite, 'fuzz', args.mutations, output / 'mutations.csv'])
    run('holdout', [suite, 'holdout', args.holdout_mutations, output / 'holdout.csv'])
    if not args.sanitizers:
        run('timing', [suite, 'benchmark', args.timing_iterations, output / 'timing.csv'])

    allocations = []
    for profile in ['tiny', 'pugi_checked']:
        for payload in ['small', 'rewrite', 'legacy', 'attributes', 'depth', 'error', 'cache-attribute', 'cache-text']:
            command = [binary('comparison_allocations'), profile, '-1', 'once', payload]
            result = run(f'allocation-{profile}-{payload}-control', command)
            control = json.loads(result.stdout)
            control['payload'] = payload
            allocations.append(control)
            for mode in ['once', 'persistent']:
                for failure in range(control['calls']):
                    reference_error = profile == 'tiny' and payload == 'error'
                    result = run(f'allocation-{profile}-{payload}-{mode}-{failure}',
                                 [binary('comparison_allocations'), profile, failure, mode, payload],
                                 allowed=(0, 1, 23) if reference_error else (0,),
                                 reference_oom_measurement=reference_error)
                    record = json.loads(next(line for line in result.stdout.splitlines() if line.startswith(b'{')))
                    if result.returncode and not (reference_error and record['outcome'] == 2
                            and record['outstanding'] > 0 and not record['tracking_overflow']):
                        raise RuntimeError('Unexpected reference allocator failure')
                    record['process_exit_code'] = result.returncode
                    record['payload'] = payload
                    allocations.append(record)
    (output / 'allocations.json').write_text(json.dumps(allocations, indent=2) + '\n', encoding='utf-8')

    def rows(name):
        with (output / name).open(encoding='utf-8', newline='') as stream:
            return list(csv.DictReader(stream, delimiter='\t'))
    report['corpus'] = rows('corpus.tsv')
    report['application_differences'] = {
        name: sum(row.get('equal_to_tiny', row.get('compared_with_tiny')) == '0'
                  for row in rows(name + '.tsv') if row['profile'] == 'pugi_checked')
        for name in ['protocol', 'protocol-edges', 'storage']}
    report['differential_mutations'] = {}
    for name in ['mutations', 'holdout']:
        with (output / (name + '.csv')).open(encoding='utf-8', newline='') as stream:
            report['differential_mutations'][name] = next(csv.DictReader(stream))
    report['allocations'] = {'processes': len(allocations),
                             'controls': [row for row in allocations if row['fail_at'] < 0],
                             'reference_oom_failures': [row for row in allocations if row.get('process_exit_code', 0)],
                             'fault_injections': sum(row['fail_at'] >= 0 for row in allocations)}
    if not args.sanitizers:
        with (output / 'timing.csv').open(encoding='utf-8', newline='') as stream:
            timing = list(csv.DictReader(stream))
        report['median_p50_ns'] = {profile: {
            case: statistics.median(int(row['p50_ns']) for row in timing
                                    if row['profile'] == profile and row['case'] == case)
            for case in sorted({row['case'] for row in timing})}
            for profile in ['tiny', 'pugi_checked']}
    candidate_corpus = next(row for row in report['corpus'] if row['profile'] == 'pugi_checked')
    incompatible = candidate.returncode != 0 or any(int(candidate_corpus[field]) for field in
        ['acceptance_diff', 'success_DOM_diff', 'failed_DOM_diff', 'error_code_diff', 'diagnostic_diff']) or any(report['application_differences'].values())
    if incompatible:
        raise RuntimeError('Candidate compatibility regression')
    report['decision'] = 'COMPATIBLE_ON_TESTED_CONTRACT_REQUIRES_PRODUCTION_AND_CONSUMER_REVIEW'
    report['complete'] = True
    (output / 'summary.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(report['decision'], flush=True)


if __name__ == '__main__':
    main()
