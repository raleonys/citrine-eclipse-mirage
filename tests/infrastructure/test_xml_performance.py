# SPDX-License-Identifier: GPL-3.0-only

"""Reject misleading, incomplete or semantically unequal performance receipts."""
import copy
from pathlib import Path
import sys
import unittest
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'diagnostics/xml_backend'))
from performance import CASES, PROFILES, summarize


def fixture(ratios=(.5,)*10):
    rows, memory = [], []
    for case in CASES:
        for repeat, ratio in enumerate(ratios):
            for position, profile in enumerate(PROFILES):
                scale = ratio if profile == 'pugi_checked' else 1
                rows.append(dict(profile=profile, case=case, repeat=str(repeat), order=str((position+repeat)%2),
                                 iterations='32', input_bytes='100', mean_ns=str(100*scale),
                                 p50_ns=str(100*scale), p95_ns=str(200*scale), p99_ns=str(300*scale),
                                 sink='224', input_hash='12345', seed='42'))
        for profile in PROFILES:
            memory.append(dict(case=case, profile=profile, outcome=0, outstanding=0,
                               tracking_overflow=False, fail_at=-1, calls=3,
                               allocated_bytes=200, peak_live_bytes=100, sink=7))
    return rows, memory


class PerformanceReceipts(unittest.TestCase):
    def test_latency_ratio_direction_and_exact_constant_interval(self):
        for ratio, label in [(.5, 'FASTER_IN_THIS_RUN'), (2, 'SLOWER_IN_THIS_RUN')]:
            rows, memory = fixture((ratio,)*10)
            result = summarize(rows, memory, 10, 42, 1000)
            for case in result['cases']:
                self.assertEqual(case['observation'], label)
                self.assertAlmostEqual(case['candidate_over_reference_latency'], ratio)
                self.assertAlmostEqual(case['interval_low'], ratio)
                self.assertAlmostEqual(case['interval_high'], ratio)

    def test_variable_or_equal_workloads_do_not_claim_a_speed_win(self):
        for ratios in [(1,)*10, (.5, 2)*5]:
            rows, memory = fixture(ratios)
            result = summarize(rows, memory, 10, 42, 1000)
            self.assertTrue(all(c['observation']=='INCONCLUSIVE' for c in result['cases']))

    def test_missing_duplicate_or_unpaired_rounds_are_rejected(self):
        rows, memory = fixture()
        for broken in [rows[:-1], rows+[rows[0]], [r for r in rows if r['profile']=='tiny']]:
            with self.assertRaises(ValueError):summarize(broken, memory, 10, 42, 1000)

    def test_changed_work_semantics_or_measurement_settings_are_rejected(self):
        rows, memory = fixture()
        for field, value in [('sink','225'), ('input_hash','999'), ('iterations','64'),
                             ('input_bytes','101'), ('seed','43'), ('order','1')]:
            broken=copy.deepcopy(rows);broken[0][field]=value
            with self.subTest(field=field), self.assertRaises(ValueError):
                summarize(broken, memory, 10, 42, 1000)

    def test_round_identity_and_backend_first_balance_are_required(self):
        rows, memory=fixture()
        changed=copy.deepcopy(rows)
        for row in changed:
            if row['case']==CASES[0] and row['repeat']=='1':row['input_hash']='56789'
        with self.assertRaises(ValueError):summarize(changed, memory, 10, 42, 1000)
        biased=copy.deepcopy(rows)
        for row in biased:row['order']='0' if row['profile']=='tiny' else '1'
        with self.assertRaises(ValueError):summarize(biased, memory, 10, 42, 1000)

    def test_invalid_latency_and_percentiles_are_rejected(self):
        rows, memory=fixture()
        for field, value in [('mean_ns','0'), ('p50_ns','nan'), ('p95_ns','inf'), ('p99_ns','1')]:
            broken=copy.deepcopy(rows);broken[0][field]=value
            with self.subTest(field=field), self.assertRaises(ValueError):
                summarize(broken, memory, 10, 42, 1000)

    def test_leaks_failed_controls_and_incomplete_memory_measurements_are_rejected(self):
        rows, memory=fixture()
        for field, value in [('outstanding',1), ('outcome',1), ('tracking_overflow',True),
                             ('fail_at',0), ('sink',8), ('peak_live_bytes',0), ('peak_live_bytes',201)]:
            broken=copy.deepcopy(memory);broken[0][field]=value
            with self.subTest(field=field), self.assertRaises(ValueError):
                summarize(rows, broken, 10, 42, 1000)
        with self.assertRaises(ValueError):summarize(rows, memory[:-1], 10, 42, 1000)
        with self.assertRaises(ValueError):summarize(rows, memory+[memory[0]], 10, 42, 1000)

    def test_memory_and_timing_must_consume_the_same_complete_document(self):
        rows, memory=fixture()
        for row in memory:row['sink']=8
        with self.assertRaises(ValueError):summarize(rows, memory, 10, 42, 1000)

    def test_short_runs_are_rejected(self):
        rows, memory=fixture()
        with self.assertRaises(ValueError):summarize(rows, memory, 5, 42, 1000)
        with self.assertRaises(ValueError):summarize(rows, memory, 10, 42, 100)


if __name__=='__main__':unittest.main()
