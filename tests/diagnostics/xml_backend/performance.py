# SPDX-License-Identifier: GPL-3.0-only

"""Validate paired benchmark receipts and summarize host-specific uncertainty."""
import math
import random
import statistics

PROFILES = ('tiny', 'pugi_checked')
CASES = ('small-request', 'large-text', 'one-MiB-text', 'many-elements',
         'reference-heavy', 'declaration-request', 'legacy-accepted',
         'unicode-reference', 'many-attributes', 'deep-elements')


def positive(value, name):
    number = float(value)
    if not math.isfinite(number) or number <= 0:
        raise ValueError('Invalid positive benchmark field: ' + name)
    return number


def summarize(rows, allocations, repeats, seed, bootstrap_samples=20000):
    if repeats < 10 or bootstrap_samples < 1000:
        raise ValueError('Insufficient benchmark rounds or resamples')
    pairs = {}
    for row in rows:
        case, profile, repeat = row['case'], row['profile'], int(row['repeat'])
        if case not in CASES or profile not in PROFILES or not 0 <= repeat < repeats:
            raise ValueError('Unexpected benchmark fixture/profile/round')
        key = case, repeat
        pair = pairs.setdefault(key, {})
        if profile in pair:
            raise ValueError('Duplicate benchmark profile in a paired round')
        if int(row['seed']) != seed or not 32 <= int(row['iterations']) <= 100000 or int(row['input_bytes']) <= 0:
            raise ValueError('Benchmark input/iteration/seed receipt mismatch')
        if not all(0 <= int(row[field]) < 1 << 64 for field in ('sink', 'input_hash')):
            raise ValueError('Invalid unsigned workload digest')
        if int(row['order']) not in (0, 1):
            raise ValueError('Invalid paired execution order')
        for field in ('mean_ns', 'p50_ns', 'p95_ns', 'p99_ns'):
            positive(row[field], field)
        if not float(row['p50_ns']) <= float(row['p95_ns']) <= float(row['p99_ns']):
            raise ValueError('Latency percentiles are out of order')
        pair[profile] = row
    if set(pairs) != {(case, repeat) for case in CASES for repeat in range(repeats)}:
        raise ValueError('Incomplete benchmark fixture/round matrix')
    for pair in pairs.values():
        if set(pair) != set(PROFILES):
            raise ValueError('Missing paired backend')
        baseline, candidate = (pair[p] for p in PROFILES)
        for field in ('iterations', 'input_bytes', 'sink', 'input_hash', 'seed'):
            if baseline[field] != candidate[field]:
                raise ValueError('Paired workload/semantic digest mismatch: ' + field)
        if {int(baseline['order']), int(candidate['order'])} != {0, 1}:
            raise ValueError('Paired execution order is not complementary')
    memory = {}
    for row in allocations:
        case, profile = row['case'], row['profile']
        if case not in CASES or profile not in PROFILES or (case, profile) in memory:
            raise ValueError('Unexpected or duplicate allocation measurement')
        if row['outcome'] or row['outstanding'] or row['tracking_overflow'] or row['fail_at'] != -1:
            raise ValueError('Allocation measurement is not a clean control')
        for field in ('calls', 'allocated_bytes', 'peak_live_bytes'):
            positive(row[field], field)
        if row['peak_live_bytes'] > row['allocated_bytes']:
            raise ValueError('Peak live bytes exceed total allocated bytes')
        memory[case, profile] = row
    if set(memory) != {(case, profile) for case in CASES for profile in PROFILES}:
        raise ValueError('Incomplete allocation matrix')
    result = {'schema': 'xml.performance.v1', 'decision': 'PER_FIXTURE_HOST_OBSERVATIONS',
              'rounds': repeats, 'seed': seed, 'bootstrap_samples': bootstrap_samples,
              'metric': 'paired geometric mean of complete parse/traverse/destroy mean latency',
              'interval': 'paired bootstrap; Bonferroni-adjusted nominal 95% family intervals',
              'scope': 'warm process; timer/check/loop overhead included; no FPS or universal speed claim',
              'cases': []}
    rng = random.Random(seed)
    tail = .05 / (2 * len(CASES))
    for case in CASES:
        paired = [pairs[case, repeat] for repeat in range(repeats)]
        for field in ('iterations', 'input_bytes', 'input_hash'):
            if len({p['tiny'][field] for p in paired}) != 1:
                raise ValueError('Fixture identity changed between rounds: ' + field)
        first_count=sum(int(p['tiny']['order'])==0 for p in paired)
        if abs(first_count-(repeats-first_count))>1:
            raise ValueError('Backend-first order is not balanced')
        logs = [math.log(float(p['pugi_checked']['mean_ns']) / float(p['tiny']['mean_ns'])) for p in paired]
        samples = sorted(math.exp(statistics.fmean(rng.choices(logs, k=repeats)))
                         for _ in range(bootstrap_samples))
        low, high = samples[int((bootstrap_samples-1)*tail)], samples[int((bootstrap_samples-1)*(1-tail))]
        ratio = math.exp(statistics.fmean(logs))
        observation = 'FASTER_IN_THIS_RUN' if high < 1 else 'SLOWER_IN_THIS_RUN' if low > 1 else 'INCONCLUSIVE'
        detail = {'case': case, 'input_bytes': int(paired[0]['tiny']['input_bytes']),
                  'candidate_over_reference_latency': ratio, 'interval_low': low, 'interval_high': high,
                  'observation': observation, 'profiles': {}}
        for profile in PROFILES:
            values = [pair[profile] for pair in paired]
            allocation = memory[case, profile]
            detail['profiles'][profile] = {
                'median_mean_ns': statistics.median(float(v['mean_ns']) for v in values),
                **{'median_' + field: statistics.median(float(v[field]) for v in values)
                   for field in ('p50_ns', 'p95_ns', 'p99_ns')},
                'calls': allocation['calls'], 'allocated_bytes': allocation['allocated_bytes'],
                'peak_live_bytes': allocation['peak_live_bytes']}
        if memory[case, 'tiny']['sink'] != memory[case, 'pugi_checked']['sink']:
            raise ValueError('Allocation workload digest mismatch')
        # Timed unsigned sums wrap modulo 2^64; memory probes consume one document.
        for repeat in range(repeats):
            for profile in PROFILES:
                timed = pairs[case, repeat][profile]
                if int(timed['sink']) != memory[case, profile]['sink'] * int(timed['iterations']) % (1 << 64):
                    raise ValueError('Timed and allocation work differ')
        detail['peak_heap_ratio'] = memory[case, 'pugi_checked']['peak_live_bytes'] / memory[case, 'tiny']['peak_live_bytes']
        result['cases'].append(detail)
    return result
