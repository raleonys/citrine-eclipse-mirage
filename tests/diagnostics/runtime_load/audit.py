# SPDX-License-Identifier: GPL-3.0-only
"""Offline consistency audit; execution provenance still belongs to the CI run."""
import argparse
import csv
import hashlib
import importlib.util
import io
import json
from pathlib import Path, PurePosixPath
import re
import stat
import zipfile

spec = importlib.util.spec_from_file_location('runtime_benchmark_runner', Path(__file__).with_name('run.py'))
benchmark = importlib.util.module_from_spec(spec)
spec.loader.exec_module(benchmark)
MAX_BYTES = 512 * 1024 * 1024


def member(name):
    if not isinstance(name, str) or not name or any(c in name for c in ('\\', ':', '\0')):
        raise ValueError('Invalid evidence member path')
    if name.startswith('/') or any(part in ('', '.', '..') for part in name.split('/')):
        raise ValueError('Evidence member cannot escape its selected root')
    return name


class DirectoryEvidence:
    def __init__(self, results, build):
        self.results, self.build = Path(results).resolve(), Path(build).resolve()

    def read(self, name):
        parts = PurePosixPath(member(name)).parts
        root = self.results if parts[0] == 'results' else self.build if parts[0] == 'binaries' else None
        if root is None:
            raise ValueError('Unknown evidence root')
        path = root.joinpath(*parts[1:])
        if not path.resolve().is_relative_to(root) or not path.is_file() or path.stat().st_size > MAX_BYTES:
            raise ValueError('Missing, redirected or oversized evidence member')
        if any(p.is_symlink() for p in (path, *path.parents) if p != root and root in p.parents):
            raise ValueError('Linked evidence member')
        return path.read_bytes()


class ZipEvidence:
    def __init__(self, path):
        self.zip = zipfile.ZipFile(path)
        names, size = set(), 0
        entries = self.zip.infolist()
        if len(entries) > 8192:
            raise ValueError('Oversized evidence member inventory')
        for entry in entries:
            name = member(entry.orig_filename.rstrip('/'))
            kind = stat.S_IFMT(entry.external_attr >> 16)
            if kind not in (0, stat.S_IFREG, stat.S_IFDIR):
                raise ValueError('Evidence ZIP contains a link or special file')
            if name.casefold() in names:
                raise ValueError('Duplicate or case-aliased evidence member')
            names.add(name.casefold())
            size += entry.file_size
        if size > MAX_BYTES:
            raise ValueError('Evidence ZIP exceeds its bounded decoded size')
        roots = [e.filename.removesuffix('results/results.json') for e in entries
                 if e.filename.endswith('results/results.json')]
        if len(roots) != 1:
            raise ValueError('Evidence ZIP must contain exactly one results root')
        self.prefix = roots[0]

    def read(self, name):
        return self.zip.read(self.prefix + member(name))


def audit(evidence):
    b = benchmark
    data = b.load_json(evidence.read('results/results.json'))
    if data.get('schema') != 'citrine.runtime-load.v3' or data.get('result') != 'PASS_CORRECTNESS_WITH_MEASUREMENTS':
        raise ValueError('Only complete v3 correctness receipts can pass this audit')
    if data.get('evidence_class') != 'WINDOWS_NATIVE_COMPONENT_BENCHMARK' or data.get('system') != 'Windows' or data.get('performance_thresholds') is not False:
        raise ValueError('Wrong declared evidence class or timing gate')
    b.integer(data.get('logical_cpus'), 'logical_cpus', 1)
    quick, repeats = data.get('quick'), data.get('repeats')
    if type(quick) is not bool or type(repeats) is not int or (repeats != 1 if quick else not 5 <= repeats <= 20):
        raise ValueError('Wrong smoke/full repetition contract')
    if data.get('statistical_comparison') != ('SMOKE_ONLY' if quick else 'PAIRED_ROUNDS_HOST_SPECIFIC'):
        raise ValueError('Wrong statistical evidence class')
    workloads = b.workloads_for(data['suite'])
    if data.get('workloads') != list(workloads):
        raise ValueError('Declared workload selection differs from the suite')
    inputs = data['source']
    if inputs != b.load_json(evidence.read('binaries/generated/inputs.json')):
        raise ValueError('Run and generated input receipts differ')
    if inputs.get('schema') != 'citrine.runtime-benchmark-inputs.v2' or any(inputs.get(k) != v for k, v in b.identity().items()):
        raise ValueError('Artifact source identity differs from the selected clean checkout')
    expected = b.generated_hashes()
    recorded = {f'asio_{n}.cpp': inputs.get('generated_service_sha256', {}).get(str(n)) for n in b.WORKERS}
    recorded['application.cpp'] = inputs.get('generated_application_sha256')
    if expected != recorded or inputs.get('production_worker_policy') != 2 or inputs.get('production_queue_limit') != 128:
        raise ValueError('Generated transformations or baseline policy differ')
    kinds = {b.binary_kind(w) for w in workloads}
    names = {f'{kind}_{n}' for kind in kinds for n in b.WORKERS}
    if set(data['binaries']) != names or set(data['builds']) != names:
        raise ValueError('Incomplete or unexpected linked candidate inventory')
    for name in sorted(names):
        binary = evidence.read(f'binaries/Release/{name}.exe')
        build = b.load_json(evidence.read(f'binaries/Release/{name}.exe.build.json'))
        sha = hashlib.sha256(binary).hexdigest()
        if data['binaries'][name] != sha or data['builds'][name] != build:
            raise ValueError('Archived executable/build receipt differs from the measured binary')
        b.validate_build(build, name, sha, inputs)
    if data.get('response_corruption_control') != 'EXPECTED_FAILURE':
        raise ValueError('Missing response negative control')
    if 'application_load' in kinds and data.get('persistence_corruption_control') != 'EXPECTED_FAILURE_XML_AND_INI':
        raise ValueError('Missing XML/INI persistence negative controls')
    clients, pipelines = ((1, 8), (1,)) if quick else ((1, 8, 32), (1, 8))
    expected_rows = repeats * len(workloads) * len(clients) * len(pipelines) * len(b.WORKERS)
    if len(data['rows']) != expected_rows:
        raise ValueError('Wrong measurement count')
    rows, paths = [], set()
    for compact in data['rows']:
        path = member(compact['samples_file'])
        if not re.fullmatch(r'samples/[0-9]{4,6}\.json', path) or path in paths:
            raise ValueError('Duplicate or unexpected raw sample reference')
        paths.add(path)
        raw = evidence.read('results/' + path)
        if hashlib.sha256(raw).hexdigest() != compact['samples_sha256']:
            raise ValueError('Raw sample SHA-256 mismatch')
        row = b.load_json(raw)
        if {k: v for k, v in compact.items() if k not in ('samples_file', 'samples_sha256')} != {k: v for k, v in row.items() if k not in b.SAMPLE_FIELDS}:
            raise ValueError('Raw sample and compact receipt disagree')
        params = b.parameters(row['workload'], row['clients'], row['pipeline'], row['workers'], quick)
        b.validate(row, params)
        rows.append(row)
    summary = b.summarize(rows, repeats, clients, pipelines, workloads)
    # Canonical JSON comparison also distinguishes booleans from integer metrics.
    canonical = lambda value: json.dumps(value, sort_keys=True, allow_nan=False)
    if canonical(summary) != canonical(data['summary']):
        raise ValueError('Recomputed summary differs from the reported summary')
    reader = csv.DictReader(io.StringIO(evidence.read('results/summary.csv').decode('utf-8')))
    if reader.fieldnames != list(summary[0]):
        raise ValueError('CSV columns differ or contain duplicates')
    table = list(reader)
    if len(table) != len(summary) or any(record != {k: '' if v is None else str(v) for k, v in row.items()} for record, row in zip(table, summary)):
        raise ValueError('CSV summary differs from recomputed JSON metrics')
    return {'schema': 'citrine.runtime-benchmark-audit.v1', 'result': 'PASS_INTEGRITY_NOT_EXECUTION_ATTESTATION',
            'source_commit': inputs['commit'], 'source_manifest_sha256': inputs['manifest_sha256'],
            'binary_count': len(names), 'measurements': len(rows),
            'verified_request_replies': sum(r['completed'] for r in rows), 'summary_cells': len(summary),
            'quick': quick, 'rounds_under_100ms': sum(r['elapsed_ns'] < 100000000 for r in rows),
            'zero_cpu_tick_rounds': sum(r['cpu_ns'] == 0 for r in rows),
            'min_samples_per_round': min(r['completed'] for r in rows),
            'interpretation': 'Declared fixture and host only; no game/FPS, universal worker, or all-schedule deadlock conclusion'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--artifact', type=Path)
    parser.add_argument('--artifact-sha256')
    parser.add_argument('--results', type=Path)
    parser.add_argument('--build-dir', type=Path)
    args = parser.parse_args()
    if bool(args.artifact) == bool(args.results) or (args.results and not args.build_dir):
        parser.error('Select --artifact ZIP or both --results and --build-dir')
    evidence = None
    try:
        benchmark.verify_source()
        if args.artifact_sha256 and (not args.artifact or benchmark.digest(args.artifact) != args.artifact_sha256):
            raise ValueError('Artifact ZIP digest differs from the independently supplied digest')
        evidence = ZipEvidence(args.artifact) if args.artifact else DirectoryEvidence(args.results, args.build_dir)
        result = audit(evidence)
    except (OSError, ValueError, KeyError, TypeError, zipfile.BadZipFile) as error:
        print(json.dumps({'result': 'FAIL', 'reason': str(error)}))
        return 1
    finally:
        if isinstance(evidence, ZipEvidence):
            evidence.zip.close()
    print(json.dumps(result, indent=2))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
