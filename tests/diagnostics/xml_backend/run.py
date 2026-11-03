# SPDX-License-Identifier: GPL-3.0-only

"""Compare the selected production XML reader with the pinned external control."""
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
from performance import CASES, PROFILES, summarize

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


def verify_inputs(reference, pugi):
    def git(*args):
        return subprocess.check_output(['git', '-C', str(reference), *args], text=True).strip()
    if Path(git('rev-parse', '--show-toplevel')).resolve() != reference:
        raise ValueError('Reference root must be its own exact checkout')
    if git('rev-parse', 'HEAD') != REFERENCE:
        raise ValueError('Reference checkout is not the reviewed PR #108 commit ' + REFERENCE)
    # Reject tracked source drift, including staged edits, before compiling it.
    if git('diff', '--name-only', 'HEAD', '--', 'include', 'src', 'tests', 'third_party/tinyxml2'):
        raise ValueError('Reference inputs have local tracked changes')
    if pugi != (REPOSITORY / 'third_party/pugixml').resolve():
        raise ValueError('Production comparison requires the installed pinned pugixml tree')
    for name, expected in PUGI_FILES.items():
        path = pugi / name
        if hashlib.sha256(path.read_bytes()).hexdigest() != expected:
            raise ValueError('External pugixml input hash mismatch: ' + name)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference-root', type=Path, required=True)
    parser.add_argument('--pugixml-root', type=Path, default=REPOSITORY / 'third_party/pugixml')
    parser.add_argument('--output', type=Path, default=REPOSITORY / 'build/xml-backend-comparison')
    parser.add_argument('--verify-only', action='store_true')
    parser.add_argument('--sanitizers', action='store_true')
    parser.add_argument('--configuration', choices=['Debug', 'Release'], default='Release')
    parser.add_argument('--mutations', type=int, default=100000)
    parser.add_argument('--holdout-mutations', type=int, default=500000)
    parser.add_argument('--timing-iterations', type=int, default=2000)
    parser.add_argument('--timing-repeats', type=int, default=20)
    parser.add_argument('--timing-seed', type=int, default=424242)
    args = parser.parse_args()
    if args.mutations < 1 or args.holdout_mutations < 1 or args.timing_iterations < 1:
        parser.error('Iteration counts must be positive')
    if not 32 <= args.timing_iterations <= 100000 or not 10 <= args.timing_repeats <= 100:
        parser.error('Timing requires 32..100000 iterations and 10..100 paired rounds')
    if not 0 <= args.timing_seed < 1 << 32:
        parser.error('Timing seed must be an unsigned 32-bit integer')
    reference, pugi = args.reference_root.resolve(), args.pugixml_root.resolve()
    verify_inputs(reference, pugi)
    if args.verify_only:
        print('PASS: pinned PR #108 reference and exact owner-upload pugixml core/license bytes')
        return
    output = output_path(args.output)
    output.mkdir(parents=True, exist_ok=True)
    for name in ['performance.json', 'performance.txt', 'performance-allocations.json', 'timing.csv']:
        (output / name).unlink(missing_ok=True)
    build = output / 'binaries'
    report = {'reference_commit': REFERENCE, 'pugixml_commit': PUGIXML,
              'pugixml_files': PUGI_FILES, 'host': platform.platform(),
              'harness_files': {path.name: hashlib.sha256(path.read_bytes()).hexdigest()
                                for path in sorted(HERE.iterdir()) if path.is_file()},
              'production_reader_files': {str(path.relative_to(REPOSITORY)): hashlib.sha256(path.read_bytes()).hexdigest()
                  for path in sorted((REPOSITORY / 'src/xml').glob('reader*')) if path.is_file()},
              'configuration': args.configuration, 'sanitizers': args.sanitizers,
              'mutations': args.mutations, 'holdout_mutations': args.holdout_mutations,
              'seed': 85621504, 'holdout_seed': 202892583, 'commands': [],
              'production_migration': 'PUGIXML_SELECTED', 'complete': False,
              'timing_repeats': args.timing_repeats, 'timing_seed': args.timing_seed,
              'cpu': platform.processor(), 'architecture': platform.machine(),
              'wire_fixture_sha256': hashlib.sha256((REPOSITORY / 'tests/fixtures/xml_reader/wire_contracts.hpp').read_bytes()).hexdigest()}

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
    (query.parent / 'toolchains-v1').touch()
    configure = ['cmake', '-S', HERE, '-B', build, f'-DXML_REFERENCE_ROOT={reference.as_posix()}',
                 f'-DPUGIXML_ROOT={pugi.as_posix()}', f'-DCMAKE_BUILD_TYPE={args.configuration}',
                 f'-DXML_COMPARISON_SANITIZERS={"ON" if args.sanitizers else "OFF"}']
    run('configure', configure)
    # Evaluate the generated target graph, including transitive dependencies.
    # The candidate-only executables must work without the TinyXML2 oracle.
    reply = build / '.cmake/api/v1/reply'
    index = json.loads(max(reply.glob('index-*.json')).read_text(encoding='utf-8'))
    model = json.loads((reply / index['reply']['codemodel-v2']['jsonFile']).read_text(encoding='utf-8'))
    toolchains = json.loads((reply / index['reply']['toolchains-v1']['jsonFile']).read_text(encoding='utf-8'))
    report['compilers'] = [{key: item['compiler'].get(key) for key in ['id', 'version']}
                           for item in toolchains['toolchains'] if item['language']=='CXX']
    linkage = []
    report['compile_flags'] = {}
    for configuration in model['configurations']:
        targets = {target['id']: json.loads((reply / target['jsonFile']).read_text(encoding='utf-8'))
                   for target in configuration['targets']}
        selected=next(target for target in targets.values() if target['name']=='comparison_candidate')
        report['compile_flags'][configuration['name']] = [fragment['fragment']
            for group in selected.get('compileGroups', [])
            for fragment in group.get('compileCommandFragments', []) if fragment.get('role')=='flags']
        actual_sources={str((Path(source['path']) if Path(source['path']).is_absolute()
                            else HERE / source['path']).resolve())
                        for source in selected.get('sources',[]) if source['path'].endswith('.cpp')}
        expected_sources={str((REPOSITORY / ('src/xml/' + name)).resolve())
                          for name in ['reader.cpp','reader_policy.cpp','reader_values.cpp']}
        if actual_sources!=expected_sources:
            raise RuntimeError('Comparison candidate is not the selected production reader source set')
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
    reader_result = run('candidate-reader', [binary('candidate_reader')])
    report['candidate_reader_contract'] = 'PASS'
    run('candidate-corpus', [binary('candidate_corpus'), corpus, baseline])
    run('candidate-allocation-contracts', [binary('candidate_allocations')])
    if sys.platform == 'win32':
        for profile in ['reference', 'candidate']:
            run(profile + '-dispatch', [binary(profile + '_dispatch')])
            run(profile + '-storage', [binary(profile + '_storage'), output / (profile + '-storage-fixtures')])
    suite = binary('comparison_suite')
    run('corpus', [suite, 'corpus', corpus, output / 'corpus.tsv'])
    for operation in ['compatibility', 'safety', 'protocol', 'protocol-edges', 'storage']:
        run(operation, [suite, operation, output / (operation + '.tsv')])
    run('ownership', [suite, 'ownership'])
    run('wire-contracts', [suite, 'wire-contracts'])
    run('mutations', [suite, 'fuzz', args.mutations, output / 'mutations.csv'])
    run('holdout', [suite, 'holdout', args.holdout_mutations, output / 'holdout.csv'])
    measure_performance = not args.sanitizers and args.configuration == 'Release'
    memory_measurements = []
    if measure_performance:
        run('timing', [suite, 'benchmark', args.timing_iterations, args.timing_repeats,
                       args.timing_seed, output / 'timing.csv'])
        for case in CASES:
            for profile in PROFILES:
                result = run('memory-' + profile + '-' + case,
                    [binary('comparison_allocations'), profile, '-1', 'once', 'benchmark:' + case])
                record = json.loads(result.stdout)
                record['case'] = case
                memory_measurements.append(record)
        (output / 'performance-allocations.json').write_text(json.dumps(memory_measurements, indent=2) + '\n', encoding='utf-8')

    allocations = []
    debug_stl_termination_confirmed = False
    termination_control = run('unknown-termination-control',
        [binary('comparison_allocations'), '--terminate-control'], allowed=(86,))
    if b'ASSESSMENT_TERMINATE' not in termination_control.stdout or any(
            line.startswith(b'{') for line in termination_control.stdout.splitlines()):
        raise RuntimeError('Unknown termination was misclassified as an injected Debug allocation')
    report['unknown_termination_control'] = 'REJECTED_AS_EXPECTED'
    for profile in ['tiny', 'pugi_checked']:
        for payload in ['stl-control', 'small', 'rewrite', 'legacy', 'attributes', 'depth', 'error', 'cache-attribute', 'cache-text']:
            command = [binary('comparison_allocations'), profile, '-1', 'once', payload]
            result = run(f'allocation-{profile}-{payload}-control', command)
            control = json.loads(result.stdout)
            control['payload'] = payload
            allocations.append(control)
            for mode in ['once', 'persistent']:
                for failure in range(control['calls']):
                    reference_error = profile == 'tiny' and payload == 'error'
                    debug_measurement = sys.platform == 'win32' and args.configuration == 'Debug'
                    allowed = (0, 1, 23) if reference_error else (0,)
                    if debug_measurement and (payload == 'stl-control' or debug_stl_termination_confirmed):
                        allowed += (86,)
                    result = run(f'allocation-{profile}-{payload}-{mode}-{failure}',
                                 [binary('comparison_allocations'), profile, failure, mode, payload],
                                 allowed=allowed,
                                 reference_oom_measurement=reference_error)
                    record = json.loads(next(line for line in result.stdout.splitlines() if line.startswith(b'{')))
                    debug_termination = (result.returncode == 86 and debug_measurement
                        and record['outcome'] == 4 and record.get('unrecoverable_debug_bad_alloc') is True
                        and record.get('debug_iterator_level', 0) > 0 and not record['tracking_overflow'])
                    if debug_termination and payload == 'stl-control':
                        debug_stl_termination_confirmed = True
                    if result.returncode and not debug_termination and not (reference_error and record['outcome'] == 2
                            and record['outstanding'] > 0 and not record['tracking_overflow']):
                        raise RuntimeError('Unexpected reference allocator failure')
                    if debug_termination:
                        record['recovery_result'] = 'UNRECOVERABLE_DEBUG_OOM_NOT_A_RECOVERY_PASS'
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
                             'reference_oom_failures': [row for row in allocations if row.get('process_exit_code', 0) in (1, 23)],
                             'debug_unrecoverable_allocations': [row for row in allocations if row.get('process_exit_code') == 86],
                             'debug_stl_control_confirmed': debug_stl_termination_confirmed,
                             'fault_injections': sum(row['fail_at'] >= 0 for row in allocations)}
    report['allocation_recovery'] = ('UNPROVEN_FOR_MSVC_DEBUG_NOEXCEPT_OOM'
        if report['allocations']['debug_unrecoverable_allocations'] else 'CANDIDATE_FAULTS_PASS')
    if measure_performance:
        with (output / 'timing.csv').open(encoding='utf-8', newline='') as stream:
            timing = list(csv.DictReader(stream))
        report['median_p50_ns'] = {profile: {
            case: statistics.median(int(row['p50_ns']) for row in timing
                                    if row['profile'] == profile and row['case'] == case)
            for case in sorted({row['case'] for row in timing})}
            for profile in ['tiny', 'pugi_checked']}
    candidate_corpus = next(row for row in report['corpus'] if row['profile'] == 'pugi_checked')
    incompatible = reader_result.returncode != 0 or any(int(candidate_corpus[field]) for field in
        ['acceptance_diff', 'success_DOM_diff', 'failed_DOM_diff', 'error_code_diff', 'diagnostic_diff']) or any(report['application_differences'].values())
    if incompatible:
        raise RuntimeError('Candidate compatibility regression')
    if measure_performance:
        measured = summarize(timing, memory_measurements, args.timing_repeats, args.timing_seed)
        measured['provenance'] = {key: report[key] for key in
            ['host', 'cpu', 'architecture', 'configuration', 'compilers', 'compile_flags',
             'production_reader_files', 'harness_files', 'wire_fixture_sha256', 'reference_commit', 'pugixml_commit']}
        report['performance'] = measured
        (output / 'performance.json').write_text(json.dumps(measured, indent=2) + '\n', encoding='utf-8')
        lines = ['Measured complete parse/traverse/destroy cost; lower ratio is faster.',
                 'Host-specific observations with paired bootstrap family intervals; no global speed claim.',
                 'Allocation measurements are separate ordinary C++/pugixml tracked heap controls, not RSS.',
                 '',
                 'case | TinyXML2 us | pugixml us | latency ratio [interval] | tracked peak bytes TinyXML2/pugixml | observation',
                 '--- | --- | --- | --- | --- | ---']
        for case in measured['cases']:
            baseline, candidate = (case['profiles'][profile] for profile in PROFILES)
            lines.append(f"{case['case']} | {baseline['median_mean_ns']/1000:.3f} | {candidate['median_mean_ns']/1000:.3f} | "
                         f"{case['candidate_over_reference_latency']:.3f} [{case['interval_low']:.3f}, {case['interval_high']:.3f}] | "
                         f"{baseline['peak_live_bytes']}/{candidate['peak_live_bytes']} | {case['observation']}")
        (output / 'performance.txt').write_text('\n'.join(lines) + '\n', encoding='utf-8')
        print('\n'.join(lines), flush=True)
    report['decision'] = 'PRODUCTION_READER_COMPATIBLE_REQUIRES_FULL_NATIVE_GOLDEN_AND_CONSUMER_REVIEW'
    report['complete'] = True
    (output / 'summary.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(report['decision'], flush=True)


if __name__ == '__main__':
    main()
