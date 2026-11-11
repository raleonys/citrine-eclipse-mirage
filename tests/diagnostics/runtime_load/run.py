# SPDX-License-Identifier: GPL-3.0-only
"""Opt-in native runtime load measurements; no production setting is changed."""
import argparse
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[3]
WORKERS = (1, 2, 4, 8, 0)  # 0 identifies the dynamic per-message control.
WORKLOADS = ('echo', 'xml', 'xml_locked', 'atomic_pair')
MARKER = 'std::array<std::thread,handler_policy::workers> workers;'
PUSH = 'jobs.push_back(std::move(fn));'
CONTROL_INSERTIONS = (
    ('    void start_workers() {', '\n        if constexpr(RUNTIME_BENCHMARK_WORKERS==0)return;'),
    ('    void stop_workers() {', '\n        if constexpr(RUNTIME_BENCHMARK_WORKERS==0){message_jobs.stop();return;}'),
    ('    void job(std::function<void()> fn) {', '''
        if constexpr(RUNTIME_BENCHMARK_WORKERS==0){
            if(stopping)return;
            message_jobs.submit(citrine_eclipse_mirage::benchmark::timed_job(std::move(fn)),
                [this](std::exception_ptr e)noexcept{factory.error("handler",e);});
            return;
        }'''))


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def generated_service(source, workers):
    if workers not in WORKERS or source.count(MARKER) != 1 or source.count(PUSH) != 1:
        raise ValueError('Unexpected candidate count or production queue/worker declaration')
    if '#include "probe.hpp"' in source:
        raise ValueError('Source is already instrumented')
    count = MARKER if workers in (0, 2) else MARKER.replace('handler_policy::workers', str(workers))
    generated = ('#include "probe.hpp"\n' + source.replace(MARKER, count)
            .replace(PUSH, 'jobs.push_back(citrine_eclipse_mirage::benchmark::timed_job(std::move(fn)));'))
    generated = generated.replace(count, count + '\n    citrine_eclipse_mirage::benchmark::PerMessageJobs message_jobs;')
    for needle, insertion in CONTROL_INSERTIONS:
        if generated.count(needle) != 1:
            raise ValueError('Production scheduling method changed')
        generated = generated.replace(needle, needle + insertion)
    return generated


def verify_source():
    sys.path.insert(0, str(ROOT / 'scripts'))
    from project_files import verify_manifest
    from verify_sources import verify
    errors = verify_manifest(ROOT)
    result = verify(ROOT)
    if errors or result['status'] != 'PASS':
        raise ValueError('Source identity verification failed: ' + repr(errors or result['errors']))
    subprocess.run([sys.executable, '-B', str(ROOT / 'scripts/vendor_sync.py'),
                    '--root', str(ROOT), '--verify-installed'], check=True,
                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=120)
    policy = (ROOT / 'include/citrine_eclipse_mirage/net/handler_policy.hpp').read_text()
    if 'workers=2;' not in policy or 'queued_jobs=128;' not in policy:
        raise ValueError('Production worker/queue policy changed; review benchmark baseline')


def identity():
    git = subprocess.run(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'],
                         capture_output=True, text=True, timeout=10)
    if git.returncode == 0:
        status = subprocess.run(['git', '-C', str(ROOT), 'status', '--porcelain'],
                                capture_output=True, text=True, check=True, timeout=10)
        if status.stdout.strip():
            raise ValueError('Commit reviewed benchmark inputs before measuring')
    return {'commit': git.stdout.strip() if git.returncode == 0 else None,
            'manifest_sha256': digest(ROOT / 'metadata/ACTIVE_SOURCE_MANIFEST.json'),
            'vendor_lock_sha256': digest(ROOT / 'metadata/VENDOR_LOCK.json'),
            'asio_source_sha256': digest(ROOT / 'src/net/asio_service.cpp')}


def prepare(output):
    verify_source()
    output = Path(output)
    if not output.resolve().is_relative_to(ROOT / 'build'):
        raise ValueError('Generated benchmark copies must stay under project build/')
    output.mkdir(parents=True, exist_ok=True)
    source = (ROOT / 'src/net/asio_service.cpp').read_text()
    hashes = {}
    for workers in WORKERS:
        path = output / f'asio_{workers}.cpp'
        path.write_text(generated_service(source, workers), encoding='utf-8', newline='\n')
        hashes[str(workers)] = digest(path)
    receipt = {'schema': 'citrine.runtime-benchmark-inputs.v1', **identity(),
               'generated_service_sha256': hashes,
               'instrumentation': 'common queue-clock wrapper/control branches; separate per-message scheduler',
               'production_worker_policy': 2, 'production_queue_limit': 128}
    (output / 'inputs.json').write_text(json.dumps(receipt, indent=2) + '\n')


def integer(value, name, minimum=0):
    if type(value) is not int or value < minimum:
        raise ValueError('Invalid integer benchmark field: ' + name)
    return value


def validate(row, expected):
    for key, value in expected.items():
        if row.get(key) != value or type(row.get(key)) is not type(value):
            raise ValueError('Benchmark workload identity differs: ' + key)
    total = expected['clients'] * expected['requests_per_client']
    if row.get('errors') != 0 or type(row.get('errors')) is not int:
        raise ValueError('Benchmark reported errors')
    if integer(row.get('completed'), 'completed') != total:
        raise ValueError('Missing or extra completed requests')
    elapsed = integer(row.get('elapsed_ns'), 'elapsed_ns', 1)
    for name in ('cpu_ns', 'cpu_cycles', 'startup_ns', 'shutdown_ns', 'peak_working_set_bytes', 'private_bytes'):
        integer(row.get(name), name, 0 if name == 'cpu_ns' else 1)
    for name in ('rtt_ns', 'queue_ns', 'handler_ns'):
        values = row.get(name)
        if not isinstance(values, list) or len(values) != total:
            raise ValueError('Incomplete per-request measurement: ' + name)
        for value in values:
            integer(value, name)
            if value > elapsed:
                raise ValueError('Sample exceeds measured interval: ' + name)
    observed = integer(row.get('observed_worker_threads'), 'observed_worker_threads', 1)
    peak = integer(row.get('peak_active_handlers'), 'peak_active_handlers', 1)
    bound = expected['workers'] or total
    if not (observed <= min(bound, total) and peak <= min(observed, expected['clients'])):
        raise ValueError('Worker/active-handler bound violated')
    created = integer(row.get('created_message_threads'), 'created_message_threads')
    if created != (0 if expected['workers'] else total):
        raise ValueError('Wrong per-message thread creation count')
    if row.get('profile') != ('thread-per-message' if expected['workers'] == 0 else 'pool-' + str(expected['workers'])):
        raise ValueError('Wrong scheduling profile')
    return row


def percentile(values, fraction):
    if not values or not 0 < fraction <= 1:
        raise ValueError('Invalid percentile input')
    return sorted(values)[max(0, math.ceil(len(values) * fraction) - 1)]


def ratio_interval(values, seed):
    if not values or any(not math.isfinite(v) or v <= 0 for v in values):
        raise ValueError('Invalid paired ratio')
    rng = random.Random(seed)
    estimates = sorted(statistics.median(rng.choices(values, k=len(values))) for _ in range(2000))
    return [percentile(estimates, .025), percentile(estimates, .975)]


def summarize(rows, repeats, clients, pipelines):
    grouped = {}
    for row in rows:
        params = {field: row.get(field) for field in
                  ('workers', 'clients', 'requests_per_client', 'pipeline', 'payload_bytes', 'workload')}
        for field in ('workers', 'clients', 'requests_per_client', 'pipeline', 'payload_bytes'):
            integer(params[field], field, 0 if field == 'workers' else 1)
        validate(row, params)
        if integer(row.get('order'), 'order') >= len(WORKERS):
            raise ValueError('Invalid candidate execution order')
        key = (row['workload'], row['clients'], row['pipeline'], row['workers'])
        rounds = grouped.setdefault(key, {})
        repeat = integer(row.get('repeat'), 'repeat')
        if repeat >= repeats or repeat in rounds:
            raise ValueError('Duplicate or unexpected benchmark round')
        rounds[repeat] = row
    expected = {(w, c, p, n) for w in WORKLOADS for c in clients for p in pipelines for n in WORKERS}
    if set(grouped) != expected or any(set(r) != set(range(repeats)) for r in grouped.values()):
        raise ValueError('Incomplete workload/candidate/repeat matrix')
    for workload in WORKLOADS:
        for count in clients:
            for pipeline in pipelines:
                for repeat in range(repeats):
                    if {grouped[workload, count, pipeline, n][repeat]['order'] for n in WORKERS} != set(range(len(WORKERS))):
                        raise ValueError('Duplicate candidate execution order')
    result = []
    for key, rounds in sorted(grouped.items()):
        workload, count, pipeline, workers = key
        base = grouped[workload, count, pipeline, 2]
        throughput_ratios, latency_ratios = [], []
        for repeat, row in rounds.items():
            baseline = base[repeat]
            for field in ('requests_per_client', 'payload_bytes'):
                if row[field] != baseline[field]:
                    raise ValueError('Paired workloads differ: ' + field)
            throughput_ratios.append(baseline['elapsed_ns'] / row['elapsed_ns'])
            latency_ratios.append(percentile(row['rtt_ns'], .99) / max(1, percentile(baseline['rtt_ns'], .99)))
        values = list(rounds.values())
        result.append({'workload': workload, 'clients': count, 'pipeline': pipeline,
                       'profile': 'thread-per-message' if workers == 0 else 'pool-' + str(workers),
                       'workers': workers, 'rounds': repeats,
                       'samples_per_round': values[0]['completed'],
                       'median_requests_per_second': statistics.median(r['completed'] * 1e9 / r['elapsed_ns'] for r in values),
                       'median_round_p50_us': statistics.median(percentile(r['rtt_ns'], .50) / 1000 for r in values),
                       'median_round_p95_us': statistics.median(percentile(r['rtt_ns'], .95) / 1000 for r in values),
                       'median_round_p99_us': statistics.median(percentile(r['rtt_ns'], .99) / 1000 for r in values),
                       'median_round_queue_p95_us': statistics.median(percentile(r['queue_ns'], .95) / 1000 for r in values),
                       'median_round_handler_p95_us': statistics.median(percentile(r['handler_ns'], .95) / 1000 for r in values),
                       'median_cpu_ns_per_request': statistics.median(r['cpu_ns'] / r['completed'] for r in values),
                       'median_cpu_cycles_per_request': statistics.median(r['cpu_cycles'] / r['completed'] for r in values),
                       'median_peak_working_set_bytes': statistics.median(r['peak_working_set_bytes'] for r in values),
                       'median_private_bytes': statistics.median(r['private_bytes'] for r in values),
                       'median_observed_worker_threads': statistics.median(r['observed_worker_threads'] for r in values),
                       'median_created_message_threads': statistics.median(r['created_message_threads'] for r in values),
                       'median_shutdown_us': statistics.median(r['shutdown_ns'] / 1000 for r in values),
                       'median_startup_us': statistics.median(r['startup_ns'] / 1000 for r in values),
                       'rounds_under_100ms': sum(r['elapsed_ns'] < 100000000 for r in values),
                       'throughput_ratio_to_2_workers': statistics.median(throughput_ratios),
                       'throughput_ratio_95pct_bootstrap': ratio_interval(throughput_ratios, 107) if repeats >= 5 else None,
                       'p99_ratio_to_2_workers': statistics.median(latency_ratios),
                       'p99_ratio_95pct_bootstrap': ratio_interval(latency_ratios, 211) if repeats >= 5 else None})
    return result


def execute(binary, params, output, fault=False):
    with tempfile.TemporaryDirectory(prefix='runtime-load-', dir=output) as directory:
        command = [str(binary), str(params['clients']), str(params['requests_per_client']),
                   str(params['pipeline']), str(params['payload_bytes']), params['workload'],
                   str(Path(directory) / 'files'), str(params['workers'])]
        env = dict(os.environ)
        env.pop('RUNTIME_BENCHMARK_FAULT', None)
        if fault:
            env['RUNTIME_BENCHMARK_FAULT'] = 'corrupt-response'
        cp = subprocess.run(command, capture_output=True, text=True, timeout=60, env=env)
        if fault:
            if cp.returncode == 0 or 'loss, corruption or response reordering' not in cp.stderr:
                raise ValueError('Deliberately corrupt response did not fail correctness checking')
            return
        if cp.returncode:
            raise ValueError('Native benchmark child failed: ' + cp.stderr[-2000:])
        return validate(json.loads(cp.stdout), params)


def run(build, output, repeats=5, quick=False):
    if platform.system() != 'Windows':
        raise ValueError('Measure on native Windows; a cross build or Wine run is not this evidence class')
    if type(repeats) is not int or not 5 <= repeats <= 20:
        raise ValueError('Full measurements require 5..20 independent rounds')
    verify_source()
    build, output = Path(build).resolve(), Path(output).resolve()
    if not output.is_relative_to(ROOT / 'build') or output.exists():
        raise ValueError('Use a new output directory under project build/')
    inputs = json.loads((build / 'generated/inputs.json').read_text())
    if any(inputs.get(key) != value for key, value in identity().items()):
        raise ValueError('Build input identity differs from current source')
    binaries = {n: build / 'Release' / f'runtime_load_{n}.exe' for n in WORKERS}
    if not all(p.is_file() for p in binaries.values()):
        raise ValueError('Build all five Release native benchmark binaries first')
    output.mkdir(parents=True)
    (output / 'samples').mkdir()
    data_rows = []
    receipt = {'schema': 'citrine.runtime-load.v1', 'result': 'RUNNING',
               'evidence_class': 'WINDOWS_NATIVE_COMPONENT_BENCHMARK',
               'source': inputs, 'system': platform.system(), 'logical_cpus': os.cpu_count(),
               'binaries': {str(n): digest(p) for n, p in binaries.items()},
               'quick': quick, 'rows': [], 'performance_thresholds': False}
    def save():
        (output / 'results.json').write_text(json.dumps(receipt, indent=2, allow_nan=False) + '\n')
    save()
    try:
        control = {'workers': 2, 'clients': 2, 'requests_per_client': 1,
                   'pipeline': 1, 'payload_bytes': 128, 'workload': 'echo'}
        execute(binaries[2], control, output, fault=True)
        receipt['response_corruption_control'] = 'EXPECTED_FAILURE'
        counts, pipelines = ((1, 8), (1,)) if quick else ((1, 8, 32), (1, 8))
        rounds = 1 if quick else repeats
        rng = random.Random(48731)
        for repeat in range(rounds):
            cases = [(w, c, p) for w in WORKLOADS for c in counts for p in pipelines]
            rng.shuffle(cases)
            for workload, clients, pipeline in cases:
                order = list(WORKERS)
                rng.shuffle(order)
                for position, workers in enumerate(order):
                    params = {'workers': workers, 'clients': clients,
                              'requests_per_client': 8 if quick else (16 if workload == 'atomic_pair' else 256),
                              'pipeline': pipeline, 'payload_bytes': 1024, 'workload': workload}
                    row = execute(binaries[workers], params, output)
                    row.update(repeat=repeat, order=position)
                    sample = output / 'samples' / f'{len(data_rows):04d}.json'
                    sample.write_text(json.dumps(row, separators=(',', ':'), allow_nan=False) + '\n')
                    data_rows.append(row)
                    compact = {key: value for key, value in row.items()
                               if key not in ('rtt_ns', 'queue_ns', 'handler_ns')}
                    compact.update(samples_file=sample.relative_to(output).as_posix(),
                                   samples_sha256=digest(sample))
                    receipt['rows'].append(compact)
                    save()
                    print(f"{len(receipt['rows'])}: {workload} clients={clients} pipeline={pipeline} workers={workers}", flush=True)
        receipt['summary'] = summarize(data_rows, rounds, counts, pipelines)
        receipt['result'] = 'PASS_CORRECTNESS_WITH_MEASUREMENTS'
        receipt['statistical_comparison'] = 'SMOKE_ONLY' if quick else 'PAIRED_ROUNDS_HOST_SPECIFIC'
        fields = list(receipt['summary'][0])
        with (output / 'summary.csv').open('w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=fields)
            writer.writeheader()
            writer.writerows(receipt['summary'])
        save()
    except BaseException as error:
        receipt['result'] = 'FAIL'
        receipt['failure_type'] = type(error).__name__
        save()
        raise


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--prepare', type=Path)
    ap.add_argument('--build-dir', type=Path)
    ap.add_argument('--output', type=Path)
    ap.add_argument('--repeats', type=int, default=5)
    ap.add_argument('--quick', action='store_true')
    args = ap.parse_args()
    if args.prepare and not args.build_dir and not args.output:
        prepare(args.prepare)
    elif args.build_dir and args.output and not args.prepare:
        run(args.build_dir, args.output, args.repeats, args.quick)
    else:
        ap.error('Select --prepare or both --build-dir and --output')
