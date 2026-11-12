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
import re
import statistics
import subprocess
import sys
import tempfile
import uuid

ROOT = Path(__file__).resolve().parents[3]
WORKERS = (1, 2, 4, 8, 0)  # 0 identifies the dynamic per-message control.
COMPONENT_WORKLOADS = ('echo', 'xml', 'xml_locked', 'atomic_pair')
APPLICATION_WORKLOADS = ('app_profile', 'app_profile_encrypted',
                         'app_mixed_encrypted', 'app_achievement_encrypted')
WORKLOADS = COMPONENT_WORKLOADS + APPLICATION_WORKLOADS
SAMPLE_FIELDS = ('rtt_ns', 'queue_ns', 'handler_ns', 'lock_wait_ns', 'lock_hold_ns', 'lock_calls')
APPLICATION_INSERTIONS = (
    ('server::PreparedResponse prepare_response(std::string_view s) override{return handler.prepare_response(s);}',
     '''server::PreparedResponse prepare_response(std::string_view s) override{
            benchmark::ApplicationRequest measured;
            auto response=handler.prepare_response(s);
            benchmark::application_fault(response.bytes);
            return response;
        }'''),
    ('std::lock_guard lock(app.handler_state_mutex);return router.registered_handler(rva,n,out);',
     '''const auto waiting=benchmark::Clock::now();
            std::lock_guard lock(app.handler_state_mutex);
            benchmark::ApplicationLock measured_lock(waiting);
            return router.registered_handler(rva,n,out);'''))
THREAD_ALIAS = '''#ifdef CITRINE_ECLIPSE_MIRAGE_TEST_ASIO_THREAD
    using OwnedThread=CITRINE_ECLIPSE_MIRAGE_TEST_ASIO_THREAD;
#else
    using OwnedThread=std::thread;
#endif'''
MARKER = 'std::array<OwnedThread,handler_policy::workers> workers;'
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


def load_json(data):
    def unique(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError('Duplicate JSON key: ' + key)
            result[key] = value
        return result
    def invalid(value):
        raise ValueError('Nonfinite JSON value: ' + value)
    return json.loads(data, object_pairs_hook=unique, parse_constant=invalid)


def workloads_for(suite):
    return {'all': WORKLOADS, 'component': COMPONENT_WORKLOADS,
            'application': APPLICATION_WORKLOADS}[suite]


def parameters(workload, clients, pipeline, workers, quick=False):
    if workload not in WORKLOADS or workers not in WORKERS:
        raise ValueError('Unknown workload or scheduling candidate')
    return {'workers': workers, 'clients': clients,
            'requests_per_client': 8 if quick else (16 if workload in ('atomic_pair', 'app_achievement_encrypted') else 256),
            'pipeline': pipeline, 'payload_bytes': 0 if workload in APPLICATION_WORKLOADS else 1024,
            'workload': workload}


def binary_kind(workload):
    return 'application_load' if workload in APPLICATION_WORKLOADS else 'runtime_load'


def generated_service(source, workers):
    if (workers not in WORKERS or source.count(THREAD_ALIAS) != 1
            or source.count(MARKER) != 1 or source.count(PUSH) != 1):
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


def generated_application(source):
    if '#include "application_probe.hpp"' in source:
        raise ValueError('Application is already instrumented')
    for original, replacement in APPLICATION_INSERTIONS:
        if source.count(original) != 1:
            raise ValueError('Application dispatch/lock boundary changed')
        source = source.replace(original, replacement)
    return '#include "application_probe.hpp"\n' + source


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
    commit = None
    # An unpacked source archive must not adopt a parent checkout's HEAD.
    # Ignore inherited repository selectors for these local, read-only commands.
    if (ROOT / '.git').exists():
        env = dict(os.environ)
        for key in ('GIT_DIR', 'GIT_COMMON_DIR', 'GIT_WORK_TREE', 'GIT_INDEX_FILE',
                    'GIT_OBJECT_DIRECTORY', 'GIT_ALTERNATE_OBJECT_DIRECTORIES', 'GIT_CEILING_DIRECTORIES'):
            env.pop(key, None)
        top = subprocess.run(['git', '-C', str(ROOT), 'rev-parse', '--show-toplevel'],
                             capture_output=True, text=True, check=True, timeout=10, env=env)
        if Path(top.stdout.strip()).resolve() != ROOT.resolve():
            raise ValueError('Git identity belongs to another source root')
        git = subprocess.run(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'],
                             capture_output=True, text=True, check=True, timeout=10, env=env)
        commit = git.stdout.strip()
        if not re.fullmatch('[0-9a-f]{40}', commit):
            raise ValueError('Invalid Git source commit')
        status = subprocess.run(['git', '-C', str(ROOT), 'status', '--porcelain'],
                                capture_output=True, text=True, check=True, timeout=10, env=env)
        if status.stdout.strip():
            raise ValueError('Commit reviewed benchmark inputs before measuring')
    return {'commit': commit,
            'manifest_sha256': digest(ROOT / 'metadata/ACTIVE_SOURCE_MANIFEST.json'),
            'vendor_lock_sha256': digest(ROOT / 'metadata/VENDOR_LOCK.json'),
            'asio_source_sha256': digest(ROOT / 'src/net/asio_service.cpp'),
            'application_source_sha256': digest(ROOT / 'src/runtime/application.cpp')}


def write_generated(payloads):
    # Preflight the complete generated set before changing any destination.
    # Exclusive staging/replacement never truncates a borrowed source inode.
    for path in payloads:
        if path.is_symlink() or (path.exists() and
                (not path.is_file() or path.stat().st_nlink != 1)):
            raise ValueError('Unsafe generated benchmark output')
    staged = []
    try:
        for path, payload in payloads.items():
            with tempfile.NamedTemporaryFile(dir=path.parent, prefix='.' + path.name,
                                             suffix='.partial', delete=False) as output:
                temporary = Path(output.name)
                staged.append((temporary, path))
                output.write(payload)
                output.flush()
                os.fsync(output.fileno())
        for temporary, path in staged:
            os.replace(temporary, path)
    finally:
        for temporary, _ in staged:
            temporary.unlink(missing_ok=True)


def prepare(output):
    verify_source()
    output = Path(output)
    if not output.resolve().is_relative_to(ROOT / 'build'):
        raise ValueError('Generated benchmark copies must stay under project build/')
    output.mkdir(parents=True, exist_ok=True)
    source = (ROOT / 'src/net/asio_service.cpp').read_text()
    hashes, payloads = {}, {}
    for workers in WORKERS:
        path = output / f'asio_{workers}.cpp'
        payload = generated_service(source, workers).encode('utf-8')
        payloads[path] = payload
        hashes[str(workers)] = hashlib.sha256(payload).hexdigest()
    app_payload = generated_application((ROOT / 'src/runtime/application.cpp').read_text()).encode('utf-8')
    payloads[output / 'application.cpp'] = app_payload
    receipt = {'schema': 'citrine.runtime-benchmark-inputs.v2', **identity(),
               'generated_service_sha256': hashes,
               'generated_application_sha256': hashlib.sha256(app_payload).hexdigest(),
               'instrumentation': 'common queue-clock wrapper/control branches; separate per-message scheduler',
               'production_worker_policy': 2, 'production_queue_limit': 128}
    payloads[output / 'inputs.json'] = (json.dumps(receipt, indent=2) + '\n').encode('utf-8')
    write_generated(payloads)


def generated_hashes():
    source = (ROOT / 'src/net/asio_service.cpp').read_text()
    expected = {f'asio_{n}.cpp': hashlib.sha256(generated_service(source, n).encode()).hexdigest()
                for n in WORKERS}
    app = generated_application((ROOT / 'src/runtime/application.cpp').read_text()).encode()
    expected['application.cpp'] = hashlib.sha256(app).hexdigest()
    return expected


def verify_generated(build, inputs):
    expected = generated_hashes()
    recorded = {f'asio_{n}.cpp': inputs.get('generated_service_sha256', {}).get(str(n)) for n in WORKERS}
    recorded['application.cpp'] = inputs.get('generated_application_sha256')
    if recorded != expected or any(digest(build / 'generated' / name) != value for name, value in expected.items()):
        raise ValueError('Generated Application/Asio copy identity differs from reviewed source')


def record_build(build, binary, kind, workers, configuration, compiler_id, compiler_version):
    build, binary = Path(build).resolve(), Path(binary).resolve()
    if kind not in ('runtime_load', 'application_load') or workers not in WORKERS:
        raise ValueError('Unknown linked candidate')
    if configuration not in ('Release', 'Debug', 'RelWithDebInfo', 'MinSizeRel'):
        raise ValueError('Unknown build configuration')
    if not build.is_relative_to(ROOT / 'build') or binary.parent != build / configuration:
        raise ValueError('Linked candidate must stay in its owned configuration directory')
    if compiler_id not in ('MSVC', 'Clang', 'GNU') or not re.fullmatch(r'[0-9][0-9A-Za-z.+_-]{0,60}', compiler_version):
        raise ValueError('Invalid compiler identity')
    if binary.name != f'{kind}_{workers}.exe' or not binary.is_file() or binary.stat().st_nlink != 1:
        raise ValueError('Unexpected or aliased linked candidate')
    inputs = load_json((build / 'generated/inputs.json').read_bytes())
    if any(inputs.get(key) != value for key, value in identity().items()):
        raise ValueError('Reconfigure after source identity changes before linking')
    verify_generated(build, inputs)
    receipt = {'schema': 'citrine.runtime-benchmark-build.v1', 'source': inputs,
               'binary': binary.name, 'sha256': digest(binary), 'kind': kind, 'workers': workers,
               'configuration': configuration, 'compiler_id': compiler_id,
               'compiler_version': compiler_version, 'cxx_standard': 20}
    write_generated({Path(str(binary) + '.build.json'): (json.dumps(receipt, indent=2) + '\n').encode()})


def validate_build(receipt, name, sha256, inputs):
    match = re.fullmatch(r'(runtime_load|application_load)_([01248])', name)
    if not match or receipt.get('schema') != 'citrine.runtime-benchmark-build.v1':
        raise ValueError('Missing linked-build receipt or unknown candidate')
    expected = {'source': inputs, 'binary': name + '.exe', 'sha256': sha256,
                'kind': match[1], 'workers': int(match[2]), 'configuration': 'Release', 'cxx_standard': 20}
    if any(receipt.get(key) != value or type(receipt.get(key)) is not type(value) for key, value in expected.items()):
        raise ValueError('Linked binary/source/configuration identity differs: ' + name)
    if receipt.get('compiler_id') not in ('MSVC', 'Clang', 'GNU') or not re.fullmatch(r'[0-9][0-9A-Za-z.+_-]{0,60}', receipt.get('compiler_version', '')):
        raise ValueError('Invalid linked compiler identity')
    return receipt


def verified_builds(binaries, inputs):
    result = {}
    for (kind, workers), binary in binaries.items():
        if binary.is_symlink() or not binary.is_file() or binary.stat().st_nlink != 1:
            raise ValueError('Build the selected regular Release binaries first')
        name = f'{kind}_{workers}'
        result[name] = validate_build(load_json(Path(str(binary) + '.build.json').read_bytes()), name, digest(binary), inputs)
    return result


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
    application = expected['workload'] in APPLICATION_WORKLOADS
    for name in SAMPLE_FIELDS[:5] if application else SAMPLE_FIELDS[:3]:
        values = row.get(name)
        if not isinstance(values, list) or len(values) != total:
            raise ValueError('Incomplete per-request measurement: ' + name)
        for value in values:
            integer(value, name)
            if value > elapsed:
                raise ValueError('Sample exceeds measured interval: ' + name)
    if application:
        if row.get('lock_calls') != [1] * total or any(type(n) is not int for n in row['lock_calls']):
            raise ValueError('Every Application request must observe exactly one handler lock')
        if row.get('application_routed') is not True or integer(row.get('verified_responses'), 'verified_responses') != total:
            raise ValueError('Missing verified Application responses')
        if integer(row.get('handler_lock_calls'), 'handler_lock_calls') != total:
            raise ValueError('Application handler lock observations are incomplete')
        for wait, hold, handler in zip(row['lock_wait_ns'], row['lock_hold_ns'], row['handler_ns']):
            if wait + hold > handler:
                raise ValueError('Application lock intervals exceed handler duration')
        if expected['workload'] == 'app_achievement_encrypted' and row.get('persistence_verified') is not True:
            raise ValueError('Application persistence was not verified')
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


def summarize(rows, repeats, clients, pipelines, workloads=WORKLOADS):
    grouped = {}
    for row in rows:
        params = {field: row.get(field) for field in
                  ('workers', 'clients', 'requests_per_client', 'pipeline', 'payload_bytes', 'workload')}
        for field in ('workers', 'clients', 'requests_per_client', 'pipeline', 'payload_bytes'):
            integer(params[field], field, 0 if field in ('workers', 'payload_bytes') else 1)
        validate(row, params)
        if integer(row.get('order'), 'order') >= len(WORKERS):
            raise ValueError('Invalid candidate execution order')
        key = (row['workload'], row['clients'], row['pipeline'], row['workers'])
        rounds = grouped.setdefault(key, {})
        repeat = integer(row.get('repeat'), 'repeat')
        if repeat >= repeats or repeat in rounds:
            raise ValueError('Duplicate or unexpected benchmark round')
        rounds[repeat] = row
    expected = {(w, c, p, n) for w in workloads for c in clients for p in pipelines for n in WORKERS}
    if set(grouped) != expected or any(set(r) != set(range(repeats)) for r in grouped.values()):
        raise ValueError('Incomplete workload/candidate/repeat matrix')
    for workload in workloads:
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
                       'p99_rank_per_round': math.ceil(values[0]['completed'] * .99),
                       'samples_at_or_above_p99_rank': values[0]['completed'] - math.ceil(values[0]['completed'] * .99) + 1,
                       'median_round_duration_ms': statistics.median(r['elapsed_ns'] / 1e6 for r in values),
                       'rounds_with_zero_cpu_ticks': sum(r['cpu_ns'] == 0 for r in values),
                       'median_requests_per_second': statistics.median(r['completed'] * 1e9 / r['elapsed_ns'] for r in values),
                       'median_round_p50_us': statistics.median(percentile(r['rtt_ns'], .50) / 1000 for r in values),
                       'median_round_p95_us': statistics.median(percentile(r['rtt_ns'], .95) / 1000 for r in values),
                       'median_round_p99_us': statistics.median(percentile(r['rtt_ns'], .99) / 1000 for r in values),
                       'median_round_queue_p95_us': statistics.median(percentile(r['queue_ns'], .95) / 1000 for r in values),
                       'median_round_handler_p95_us': statistics.median(percentile(r['handler_ns'], .95) / 1000 for r in values),
                       'median_round_lock_wait_p95_us': statistics.median(percentile(r['lock_wait_ns'], .95) / 1000 for r in values) if workload in APPLICATION_WORKLOADS else None,
                       'median_round_lock_hold_p95_us': statistics.median(percentile(r['lock_hold_ns'], .95) / 1000 for r in values) if workload in APPLICATION_WORKLOADS else None,
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
            env['RUNTIME_BENCHMARK_FAULT'] = 'corrupt-response' if fault is True else fault
        cp = subprocess.run(command, capture_output=True, text=True, timeout=60, env=env)
        if fault:
            failure = ('Application persistence validation failed' if fault in ('corrupt-persistence', 'corrupt-ini') else
                       'application response validation failed' if params['workload'] in APPLICATION_WORKLOADS
                       else 'loss, corruption or response reordering')
            if cp.returncode == 0 or failure not in cp.stderr:
                raise ValueError('Deliberately corrupt response did not fail correctness checking')
            return
        if cp.returncode:
            raise ValueError('Native benchmark child failed: ' + cp.stderr[-2000:])
        return validate(load_json(cp.stdout), params)


def run(build, output, repeats=5, quick=False, suite='all'):
    if platform.system() != 'Windows':
        raise ValueError('Measure on native Windows; a cross build or Wine run is not this evidence class')
    if type(repeats) is not int or not 5 <= repeats <= 20:
        raise ValueError('Full measurements require 5..20 independent rounds')
    verify_source()
    build, output = Path(build).resolve(), Path(output).resolve()
    if not output.is_relative_to(ROOT / 'build') or output.exists():
        raise ValueError('Use a new output directory under project build/')
    inputs = load_json((build / 'generated/inputs.json').read_bytes())
    if any(inputs.get(key) != value for key, value in identity().items()):
        raise ValueError('Build input identity differs from current source')
    verify_generated(build, inputs)
    workloads = workloads_for(suite)
    kinds = {binary_kind(w) for w in workloads}
    binaries = {(kind, n): build / 'Release' / f'{kind}_{n}.exe' for kind in kinds for n in WORKERS}
    if not all(p.is_file() for p in binaries.values()):
        raise ValueError('Build all selected Release native benchmark binaries first')
    builds = verified_builds(binaries, inputs)
    output.mkdir(parents=True)
    (output / 'samples').mkdir()
    data_rows = []
    receipt = {'schema': 'citrine.runtime-load.v3', 'result': 'RUNNING',
               'run_id': uuid.uuid4().hex,
               'evidence_class': 'WINDOWS_NATIVE_COMPONENT_BENCHMARK',
               'source': inputs, 'system': platform.system(), 'logical_cpus': os.cpu_count(),
               'binaries': {f'{kind}_{n}': digest(p) for (kind, n), p in binaries.items()},
               'builds': builds, 'repeats': 1 if quick else repeats,
               'suite': suite, 'workloads': list(workloads),
               'quick': quick, 'rows': [], 'performance_thresholds': False}
    def save():
        (output / 'results.json').write_text(json.dumps(receipt, indent=2, allow_nan=False) + '\n')
    save()
    try:
        for kind in sorted(kinds):
            control = {'workers': 2, 'clients': 2, 'requests_per_client': 1,
                       'pipeline': 1, 'payload_bytes': 0 if kind == 'application_load' else 1024,
                       'workload': 'app_profile' if kind == 'application_load' else 'echo'}
            execute(binaries[kind, 2], control, output, fault=True)
        receipt['response_corruption_control'] = 'EXPECTED_FAILURE'
        if 'application_load' in kinds:
            control = {'workers': 2, 'clients': 2, 'requests_per_client': 1,
                       'pipeline': 1, 'payload_bytes': 0, 'workload': 'app_achievement_encrypted'}
            for fault in ('corrupt-persistence', 'corrupt-ini'):
                execute(binaries['application_load', 2], control, output, fault=fault)
            receipt['persistence_corruption_control'] = 'EXPECTED_FAILURE_XML_AND_INI'
        counts, pipelines = ((1, 8), (1,)) if quick else ((1, 8, 32), (1, 8))
        rounds = 1 if quick else repeats
        rng = random.Random(48731)
        for repeat in range(rounds):
            cases = [(w, c, p) for w in workloads for c in counts for p in pipelines]
            rng.shuffle(cases)
            for workload, clients, pipeline in cases:
                order = list(WORKERS)
                rng.shuffle(order)
                for position, workers in enumerate(order):
                    params = parameters(workload, clients, pipeline, workers, quick)
                    kind = binary_kind(workload)
                    row = execute(binaries[kind, workers], params, output)
                    row.update(repeat=repeat, order=position, run_id=receipt['run_id'],
                               binary_sha256=receipt['binaries'][f'{kind}_{workers}'])
                    sample = output / 'samples' / f'{len(data_rows):04d}.json'
                    sample.write_text(json.dumps(row, separators=(',', ':'), allow_nan=False) + '\n')
                    data_rows.append(row)
                    compact = {key: value for key, value in row.items()
                               if key not in SAMPLE_FIELDS}
                    compact.update(samples_file=sample.relative_to(output).as_posix(),
                                   samples_sha256=digest(sample))
                    receipt['rows'].append(compact)
                    save()
                    print(f"{len(receipt['rows'])}: {workload} clients={clients} pipeline={pipeline} workers={workers}", flush=True)
        receipt['summary'] = summarize(data_rows, rounds, counts, pipelines, workloads)
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
    ap.add_argument('--suite', choices=('all', 'component', 'application'), default='all')
    ap.add_argument('--record-build', type=Path)
    ap.add_argument('--kind', choices=('runtime_load', 'application_load'))
    ap.add_argument('--workers', type=int)
    ap.add_argument('--configuration')
    ap.add_argument('--compiler-id')
    ap.add_argument('--compiler-version')
    args = ap.parse_args()
    if args.record_build and args.build_dir and not args.output and not args.prepare:
        record_build(args.build_dir, args.record_build, args.kind, args.workers,
                     args.configuration, args.compiler_id, args.compiler_version)
    elif args.prepare and not args.build_dir and not args.output:
        prepare(args.prepare)
    elif args.build_dir and args.output and not args.prepare:
        run(args.build_dir, args.output, args.repeats, args.quick, args.suite)
    else:
        ap.error('Select --prepare or both --build-dir and --output')
