# SPDX-License-Identifier: GPL-3.0-only

"""Reject misleading, incomplete or semantically unequal performance receipts."""
import copy
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'diagnostics/xml_backend'))
from performance import CASES, PROFILES, summarize, compile_receipt
from run import validate_corpus_policy


class DepthPolicyReceipt(unittest.TestCase):
    def row(self):
        return dict(profile='pugi_checked', cases='12087', acceptance_diff='2',
                    success_DOM_diff='0', failed_DOM_diff='0', error_code_diff='2',
                    diagnostic_diff='2', exact_equal='12085', policy_rejections='0',
                    declared_depth_deltas='2', unexpected_differences='0')

    def test_exact_declared_depth_differences_remain_visible(self):
        validate_corpus_policy(self.row())

    def test_hidden_extra_missing_or_reclassified_differences_fail(self):
        for field, value in [('acceptance_diff', '0'), ('acceptance_diff', '3'),
                             ('failed_DOM_diff', '1'), ('success_DOM_diff', '1'),
                             ('diagnostic_diff', '3'), ('declared_depth_deltas', '1'),
                             ('unexpected_differences', '1'), ('exact_equal', '12087'),
                             ('cases', '12086')]:
            row = self.row(); row[field] = value
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                validate_corpus_policy(row)


class ReferenceBindings(unittest.TestCase):
    def run_binding(self, directory, statement):
        root = Path(directory)
        helper = Path(__file__).resolve().parents[1] / 'diagnostics/xml_backend/reference_bindings.cmake'
        script = root / 'bind.cmake'
        output = root / 'bound.txt'
        script.write_text(
            f'include("{helper.as_posix()}")\n'
            f'set(reference "{root.as_posix()}")\n' + statement + '\n'
            f'file(WRITE "{output.as_posix()}" "${{bound}}")\n', encoding='utf-8')
        cmake = shutil.which('cmake')
        self.assertIsNotNone(cmake, 'CMake is required for XML reference binding regressions')
        result = subprocess.run([cmake, '-P', str(script)], capture_output=True, text=True, timeout=15)
        return result, output

    def test_reads_historical_header_even_when_current_identity_decoy_exists(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            legacy = root / 'include/preloader_l/xml/reader.hpp'
            decoy = root / 'include/laureondo/xml/reader.hpp'
            legacy.parent.mkdir(parents=True)
            decoy.parent.mkdir(parents=True)
            legacy.write_text('#include "preloader_l/router/router.hpp"\n'
                              'namespace preloader_l::xml_reader_bridge {}\n', encoding='utf-8')
            decoy.write_text('#error wrong reference header\n', encoding='utf-8')
            before = legacy.read_bytes(), decoy.read_bytes()
            result, output = self.run_binding(directory, 'xml_read_reference_header("${reference}" bound)')
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            self.assertEqual('#include "laureondo/router/router.hpp"\n'
                             'namespace laureondo::xml_reader_bridge {}\n', output.read_text())
            self.assertEqual(before, (legacy.read_bytes(), decoy.read_bytes()))

    def test_binds_fixture_aliases_without_renaming_compatibility_filename(self):
        with tempfile.TemporaryDirectory() as directory:
            statement = ('set(bound [=[#include "preloader_l/xml/reader.hpp"\n'
                         'namespace reader=preloader_l::xml_reader_bridge;\n'
                         'using namespace preloader_l;\n'
                         'const char* filename="preloader_l.dll";\n]=])\n'
                         'xml_bind_legacy_reference(bound)')
            result, output = self.run_binding(directory, statement)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            self.assertEqual('#include "laureondo/xml/reader.hpp"\n'
                             'namespace reader=laureondo::xml_reader_bridge;\n'
                             'using namespace laureondo;\n'
                             'const char* filename="preloader_l.dll";\n', output.read_text())

    def test_missing_historical_header_does_not_adopt_current_identity_file(self):
        with tempfile.TemporaryDirectory() as directory:
            decoy = Path(directory) / 'include/laureondo/xml/reader.hpp'
            decoy.parent.mkdir(parents=True)
            decoy.write_text('// not the pinned historical header\n', encoding='utf-8')
            result, _ = self.run_binding(directory, 'xml_read_reference_header("${reference}" bound)')
            self.assertNotEqual(0, result.returncode)
            self.assertIn('preloader_l/xml/reader.hpp', result.stderr)


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
    def test_cmake_compiler_fragments_without_role_are_preserved(self):
        for flags in ['-O3 -DNDEBUG -std=c++20', '/O2 /Ob2 /DNDEBUG -MT /std:c++20']:
            target={'name':'backend', 'compileGroups':[{'language':'CXX',
                'compileCommandFragments':[{'fragment':flags}],
                'languageStandard':{'standard':'20'}, 'defines':[{'define':'PUGIXML_NO_XPATH'}]}]}
            self.assertEqual(compile_receipt(target), {'flags':[flags], 'standards':['20'],
                                                       'definitions':['PUGIXML_NO_XPATH']})

    def test_missing_flags_or_wrong_language_standard_cannot_claim_provenance(self):
        target={'name':'backend', 'compileGroups':[{'language':'CXX',
            'compileCommandFragments':[{'fragment':'-O3'}], 'languageStandard':{'standard':'20'}}]}
        for missing in ['compileCommandFragments','languageStandard']:
            broken=copy.deepcopy(target);del broken['compileGroups'][0][missing]
            with self.assertRaises(ValueError):compile_receipt(broken)
        broken=copy.deepcopy(target);broken['compileGroups'][0]['languageStandard']['standard']='17'
        with self.assertRaises(ValueError):compile_receipt(broken)

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
