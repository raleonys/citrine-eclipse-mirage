# SPDX-License-Identifier: GPL-3.0-only

"""Tamper, failure-propagation and source/package identity regressions."""
from pathlib import Path
import ast
import contextlib
import csv
import hashlib
import io
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile
sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import check_falcion_golden_cleaning as falcion
import check_metadata_ownership as ownership
import check_human_launcher_contract as launchers
import project_files as inventory
import package_source as packaging
import extract_source as extraction
import run_golden_pipeline as golden
import ursynex_golden_finalizer as finalizer
import verify_mustavex_golden_metadata as mustavex
import generate_avatar_data as avatar_data
import clear_tool_python_cache as tool_cache
import check_repository_contract as repository
import verify_sources as source_verification
import check_msvc_build_evidence as msvc_evidence
import run_source_qa as source_qa
import ci_validation_readiness as ci_readiness
import check_dll_exports as dll_exports
from golden_policy import load_target, PolicyPending


class PublicDefHeaderTests(unittest.TestCase):
    def test_def_requires_single_ordered_library_and_exports_headers(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            (root / 'exports').mkdir()
            shutil.copyfile(ROOT / 'metadata/PUBLIC_DLL_ABI.json',
                            root / 'metadata/PUBLIC_DLL_ABI.json')
            original = (ROOT / 'exports/citrine_eclipse_mirage.def').read_text(encoding='utf-8')
            destination = root / 'exports/citrine_eclipse_mirage.def'
            header = 'LIBRARY "citrine_eclipse_mirage.unsigned.dll"'
            self.assertIn(header, original)
            self.assertIn('EXPORTS\n', original)
            entry = next(line for line in original.splitlines(keepends=True)
                         if line.strip().split() == ['DllMain', '@1'])
            variants = {
                'missing_library': original.replace(header + '\n', ''),
                'missing_exports': original.replace('EXPORTS\n', ''),
                'duplicate_library': original.replace(header + '\n', header + '\n' + header + '\n'),
                'duplicate_exports': original.replace('EXPORTS\n', 'EXPORTS\nEXPORTS\n'),
                'headers_out_of_order': original.replace(header + '\nEXPORTS\n',
                                                         'EXPORTS\n' + header + '\n'),
                'export_before_exports': original.replace(entry, '', 1).replace(
                    'EXPORTS\n', entry + 'EXPORTS\n'),
            }
            spaced = '\n; controlled header spacing\n' + original.replace(
                'EXPORTS\n', '\n; before exports\nEXPORTS\n\n')
            for valid in (original, spaced):
                destination.write_text(valid, encoding='utf-8')
                self.assertEqual([], dll_exports.check_source(root))
            for label, candidate in variants.items():
                with self.subTest(case=label):
                    self.assertNotEqual(candidate, original)
                    destination.write_text(candidate, encoding='utf-8')
                    self.assertTrue(dll_exports.check_source(root),
                                    'Malformed application DEF header was accepted: ' + label)
            destination.write_text(original, encoding='utf-8')
            self.assertEqual([], dll_exports.check_source(root))


class PublicAbiReportTests(unittest.TestCase):
    script = ROOT / 'scripts/check_dll_exports.py'

    def fixture(self, directory):
        # Synthetic PE32+ export tables, never loaded or executed. This tests
        # the host checker/CLI, not native linking, Golden or Consumer behavior.
        data = bytearray(0x800)
        data[:2] = b'MZ'
        struct.pack_into('<I', data, 0x3c, 0x80)
        data[0x80:0x84] = b'PE\0\0'
        coff, opt = 0x84, 0x98
        struct.pack_into('<HH', data, coff, 0x8664, 2)
        struct.pack_into('<H', data, coff + 16, 0xf0)
        struct.pack_into('<H', data, opt, 0x20b)
        struct.pack_into('<II', data, opt + 32, 0x1000, 0x200)
        struct.pack_into('<II', data, opt + 56, 0x3000, 0x200)
        struct.pack_into('<I', data, opt + 108, 16)
        struct.pack_into('<II', data, opt + 112, 0x2000, 0x200)
        for i, (name, va, raw, flags) in enumerate((
                (b'.text', 0x1000, 0x200, 0x60000020),
                (b'.edata', 0x2000, 0x400, 0x40000040))):
            offset = opt + 0xf0 + i * 40
            data[offset:offset + 8] = name.ljust(8, b'\0')
            struct.pack_into('<IIII', data, offset + 8, 0x200, va, 0x200, raw)
            struct.pack_into('<I', data, offset + 36, flags)
        struct.pack_into('<IIIIII', data, 0x410, 1, 3, 3, 0x2040, 0x2050, 0x2060)
        for i in range(3):
            struct.pack_into('<I', data, 0x440 + i * 4, 0x1010 + i * 16)
        for i, (name, ordinal_index) in enumerate((
                (b'DllMain', 0), (b'DllShutdown', 2), (b'SURGE', 1))):
            struct.pack_into('<I', data, 0x450 + i * 4, 0x2080 + i * 20)
            struct.pack_into('<H', data, 0x460 + i * 2, ordinal_index)
            start = 0x480 + i * 20
            data[start:start + len(name) + 1] = name + b'\0'
        dll = directory / 'application.dll'
        dll.write_bytes(data)
        self.assertEqual('PASS_PUBLIC_C_EXPORT_CONTRACT', dll_exports.inspect(dll, ROOT)['result'])
        return dll, bytes(data)

    def run_check(self, dll, report):
        return subprocess.run([sys.executable, '-B', str(self.script), str(dll),
                               '--report', str(report)], capture_output=True,
                              text=True, timeout=15)

    def assert_refused(self, dll, report, original):
        result = self.run_check(dll, report)
        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertNotIn('PASS_PUBLIC_C_EXPORT_CONTRACT', result.stdout)
        self.assertEqual(original, dll.read_bytes())

    def test_distinct_report_preserves_input_and_exact_inspection(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            dll, original = self.fixture(directory)
            report = directory / 'nested' / 'ABI.json'
            result = self.run_check(dll, report)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            value = json.loads(report.read_text(encoding='utf-8'))
            self.assertEqual(json.loads(result.stdout), value)
            self.assertEqual(hashlib.sha256(original).hexdigest(), value['dll_sha256'])
            self.assertEqual('NOT_RUN', value['native_consumer_acceptance'])
            self.assertEqual(original, dll.read_bytes())

    def test_invalid_abi_still_writes_a_failure_report_read_only(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            dll, original = self.fixture(directory)
            invalid = bytearray(original)
            struct.pack_into('<I', invalid, 0x410, 10)
            dll.write_bytes(invalid)
            report = directory / 'ABI.json'
            result = self.run_check(dll, report)
            self.assertEqual(1, result.returncode, result.stdout + result.stderr)
            self.assertEqual('FAIL', json.loads(report.read_text(encoding='utf-8'))['result'])
            self.assertEqual(invalid, dll.read_bytes())

    def test_report_cannot_replace_input_name(self):
        with tempfile.TemporaryDirectory() as temporary:
            dll, original = self.fixture(Path(temporary))
            self.assert_refused(dll, dll, original)

    def test_report_cannot_truncate_an_input_hardlink(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            dll, original = self.fixture(directory)
            report = directory / 'ABI.json'
            os.link(dll, report)
            self.assert_refused(dll, report, original)
            self.assertEqual(original, report.read_bytes())
            self.assertTrue(report.samefile(dll))

    def test_report_cannot_follow_an_input_symlink(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            dll, original = self.fixture(directory)
            report = directory / 'ABI.json'
            try:
                report.symlink_to(dll.name)
            except (OSError, NotImplementedError):
                self.skipTest('Host does not permit fixture symlinks')
            self.assert_refused(dll, report, original)
            self.assertTrue(report.is_symlink())

    def test_report_cannot_replace_input_through_a_directory_alias(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            actual = directory / 'actual'
            actual.mkdir()
            dll, original = self.fixture(actual)
            alias = directory / 'alias'
            try:
                alias.symlink_to(actual, target_is_directory=True)
            except (OSError, NotImplementedError):
                self.skipTest('Host does not permit fixture directory symlinks')
            self.assert_refused(dll, alias / dll.name, original)
            self.assertTrue(alias.is_symlink())

    def test_replacing_a_report_does_not_truncate_its_other_hardlink(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            dll, original = self.fixture(directory)
            unrelated = directory / 'unrelated.txt'
            unrelated.write_bytes(b'retained unrelated bytes')
            report = directory / 'ABI.json'
            os.link(unrelated, report)
            result = self.run_check(dll, report)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            self.assertEqual('PASS_PUBLIC_C_EXPORT_CONTRACT', json.loads(report.read_text(encoding='utf-8'))['result'])
            self.assertEqual(b'retained unrelated bytes', unrelated.read_bytes())
            self.assertFalse(report.samefile(unrelated))
            self.assertEqual(original, dll.read_bytes())

    def test_report_failures_preserve_old_output_and_remove_temporary(self):
        for operation in ('serialize', 'replace'):
            with self.subTest(operation=operation), tempfile.TemporaryDirectory() as temporary:
                directory = Path(temporary)
                dll, original = self.fixture(directory)
                report = directory / 'ABI.json'
                report.write_bytes(b'previous report')
                before = set(directory.iterdir())
                target = dll_exports.json if operation == 'serialize' else dll_exports.os
                name = 'dump' if operation == 'serialize' else 'replace'
                with patch.object(target, name, side_effect=OSError('controlled report failure')):
                    with self.assertRaises(OSError):
                        dll_exports._write_report(report, {'result': 'controlled'})
                self.assertEqual(b'previous report', report.read_bytes())
                self.assertEqual(original, dll.read_bytes())
                self.assertEqual(before, set(directory.iterdir()))


class MsvcCompilerReceiptTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()
        self.build = self.root / 'build'
        self.build.mkdir()
        (self.root / 'metadata').mkdir()
        (self.root / 'metadata/ACTIVE_SOURCE_MANIFEST.json').write_bytes(b'owned source identity')
        self.compiler = self.root / 'toolchain/cl.exe'
        self.compiler.parent.mkdir()
        self.compiler.write_bytes(b'owned executable identity fixture')
        self.dll = self.build / 'owned.dll'
        self.dll.write_bytes(b'owned DLL identity fixture')
        self.cache = self.build / 'CMakeCache.txt'
        self.cache.write_text('CMAKE_CXX_COMPILER:FILEPATH=' + str(self.compiler) + '\n', encoding='utf-8')
        record = self.build / 'CMakeFiles/owned/CMakeCXXCompiler.cmake'
        record.parent.mkdir(parents=True)
        record.write_text('set(CMAKE_CXX_COMPILER_ID "MSVC")\n')
        self.receipt = dict(build_succeeded=True, mode='Golden', configuration='Release',
                            compiler='cl.exe', compiler_sha256=msvc_evidence.sha(self.compiler),
                            dll_sha256=msvc_evidence.sha(self.dll), cmake_cache_sha256=msvc_evidence.sha(self.cache),
                            source_manifest_sha256=msvc_evidence.sha(self.root / 'metadata/ACTIVE_SOURCE_MANIFEST.json'))

    def errors(self):
        (self.build / 'BUILD_RECEIPT.json').write_text(json.dumps(self.receipt), encoding='utf-8')
        # This fixture isolates compiler identity; real Golden still audits its link rule.
        with patch.object(msvc_evidence, 'collect_text', return_value=('owned fixture', None, [])), \
             patch.object(msvc_evidence, 'audit', return_value=[]):
            return msvc_evidence.check(self.root, self.build, self.dll)

    def test_basename_and_matching_legacy_absolute_receipts_bind_to_cache_tool(self):
        self.assertEqual([], self.errors())
        self.receipt['compiler'] = str(self.compiler)
        self.assertEqual([], self.errors())

    def test_wrong_hash_or_different_receipt_tool_is_rejected(self):
        self.receipt['compiler_sha256'] = '0' * 64
        self.assertIn('MSVC compiler tool identity mismatch', self.errors())
        self.receipt['compiler_sha256'] = msvc_evidence.sha(self.compiler)
        self.receipt['compiler'] = str(self.root / 'other/cl.exe')
        self.assertIn('MSVC compiler tool identity mismatch', self.errors())

    def test_cwd_and_path_compiler_decoys_cannot_replace_the_cache_identity(self):
        decoy = self.root / 'cl.exe'
        decoy.write_bytes(b'different compiler in current directory and PATH')
        previous = Path.cwd()
        try:
            os.chdir(self.root)
            with patch.dict(os.environ, {'PATH': str(self.root)}):
                self.assertEqual([], self.errors())
                self.receipt['compiler_sha256'] = msvc_evidence.sha(decoy)
                self.assertIn('MSVC compiler tool identity mismatch', self.errors())
        finally:
            os.chdir(previous)

    def test_relative_receipt_directory_is_not_an_accepted_basename(self):
        self.receipt['compiler'] = str(Path('toolchain') / 'cl.exe')
        self.assertIn('MSVC compiler tool identity mismatch', self.errors())

    def test_missing_ambiguous_relative_and_changed_cache_are_rejected(self):
        original = self.cache.read_text(encoding='utf-8')
        for text in ('', original + original, 'CMAKE_CXX_COMPILER:FILEPATH=cl.exe\n'):
            with self.subTest(cache=text):
                self.cache.write_text(text, encoding='utf-8')
                self.receipt['cmake_cache_sha256'] = msvc_evidence.sha(self.cache)
                self.assertTrue(self.errors())
        self.cache.write_text(original, encoding='utf-8')
        self.assertIn('Build identity mismatch: cmake_cache_sha256', self.errors())

    def test_literal_compiler_id_survives_generated_cmake_lexical_context(self):
        declaration = 'set(CMAKE_CXX_COMPILER_ID "MSVC")\n'
        cases = {
            'ordinary': declaration,
            'line_comments': '# set(CMAKE_CXX_COMPILER_ID "Clang")\r\n' + declaration + '# trailing comment\n',
            'bracket_comments': '#[==[\nset(CMAKE_CXX_COMPILER_ID "Clang")\n]==]\n' + declaration,
            'quoted_argument': 'set(OTHER "set(CMAKE_CXX_COMPILER_ID \\"Clang\\")")\n' + declaration,
            'bracket_argument': 'set(OTHER [=[\nset(CMAKE_CXX_COMPILER_ID "Clang")\n]=])\n' + declaration,
            'nested_arguments': 'message(\n set(CMAKE_CXX_COMPILER_ID "Clang")\n)\n' + declaration,
            'multiline_command': 'SeT (\n CMAKE_CXX_COMPILER_ID\n "MSVC"\n)\n',
            'other_control_blocks': declaration + 'foreach(item IN ITEMS one two)\nif(FALSE)\nset(OTHER "ignored")\nendif()\nendforeach()\n',
            'utf8_bom': '\ufeff' + declaration,
        }
        record = self.build / 'CMakeFiles/owned/CMakeCXXCompiler.cmake'
        for name, text in cases.items():
            with self.subTest(context=name):
                record.write_text(text, encoding='utf-8')
                self.assertEqual([], self.errors())

    def test_compiler_id_rejects_inert_ambiguous_and_nonliteral_evidence(self):
        declaration = 'set(CMAKE_CXX_COMPILER_ID "MSVC")\n'
        clang = 'set(CMAKE_CXX_COMPILER_ID "Clang")\n'
        cases = {
            'line_comment_only': '# ' + declaration,
            'line_comment_decoy': '# ' + declaration + clang,
            'bracket_comment_only': '#[[\n' + declaration + ']]\n',
            'bracket_comment_decoy': '#[=[\n' + declaration + ']=]\n' + clang,
            'quoted_argument_only': 'set(OTHER "set(CMAKE_CXX_COMPILER_ID \\"MSVC\\")")\n',
            'bracket_argument_only': 'set(OTHER [==[\n' + declaration + ']==])\n',
            'nested_argument_only': 'message(\n' + declaration + ')\n',
            'duplicate_identical': declaration + declaration,
            'conflicting_later': declaration + clang,
            'conflicting_earlier': clang + declaration,
            'conditional': 'if(FALSE)\n' + declaration + 'endif()\n',
            'function_body': 'function(deferred)\n' + declaration + 'endfunction()\n',
            'unset': declaration + 'unset(CMAKE_CXX_COMPILER_ID)\n',
            'quoted_unset': declaration + 'unset("CMAKE_CXX_COMPILER_ID")\n',
            'bracket_unset': declaration + 'unset([=[CMAKE_CXX_COMPILER_ID]=])\n',
            'quoted_assignment': declaration + 'set("CMAKE_CXX_COMPILER_ID" "Clang")\n',
            'bracket_assignment': declaration + 'set([==[\nCMAKE_CXX_COMPILER_ID]==] "Clang")\n',
            'computed_value': 'set(CMAKE_CXX_COMPILER_ID "${DECOY}")\n',
            'wrong_variable_case': 'set(cmake_cxx_compiler_id "MSVC")\n',
            'different_command': 'notset(CMAKE_CXX_COMPILER_ID "MSVC")\n',
            'different_variable': 'set(CMAKE_CXX_COMPILER_ID_RUN "MSVC")\n',
            'unterminated_comment': '#[=[\n' + declaration,
            'unterminated_quote': declaration + 'set(OTHER "unfinished\n',
            'unterminated_command': declaration + 'set(OTHER value\n',
            'mismatched_control': declaration + 'endfunction()\n',
        }
        record = self.build / 'CMakeFiles/owned/CMakeCXXCompiler.cmake'
        for name, text in cases.items():
            with self.subTest(context=name):
                record.write_text(text, encoding='utf-8')
                self.assertIn('Exact build compiler ID is not MSVC', self.errors())

    def test_compiler_id_rejection_preserves_actual_link_validation(self):
        (self.build / 'BUILD_RECEIPT.json').write_text(json.dumps(self.receipt), encoding='utf-8')
        target = self.build / 'CMakeFiles/citrine_eclipse_mirage.dir'
        target.mkdir()
        link = target / 'link.txt'
        command = 'link.exe /DLL /DEBUG:NONE /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj'
        link.write_text(command + '\n', encoding='utf-8')
        self.assertEqual([], msvc_evidence.check(self.root, self.build, self.dll))
        record = self.build / 'CMakeFiles/owned/CMakeCXXCompiler.cmake'
        record.write_text('# set(CMAKE_CXX_COMPILER_ID "MSVC")\nset(CMAKE_CXX_COMPILER_ID "Clang")\n', encoding='utf-8')
        self.assertEqual(['Exact build compiler ID is not MSVC'],
                         msvc_evidence.check(self.root, self.build, self.dll))
        link.write_text(command + ' /NODEFAULTLIB\n', encoding='utf-8')
        errors = msvc_evidence.check(self.root, self.build, self.dll)
        self.assertIn('Exact build compiler ID is not MSVC', errors)
        self.assertIn('/NODEFAULTLIB is forbidden for normalized CRT shipping', errors)

    def test_actual_receipt_link_audit_rejects_dash_option_conflicts(self):
        # Real receipt/cache/compiler binding and link-text collection, with
        # owned byte fixtures only: no MSVC executable or DLL is run here.
        (self.build / 'BUILD_RECEIPT.json').write_text(json.dumps(self.receipt), encoding='utf-8')
        target = self.build / 'CMakeFiles/citrine_eclipse_mirage.dir'
        target.mkdir()
        source = target / 'link.txt'
        response = self.build / 'shipping.rsp'
        flags = '/DLL /DEBUG:NONE /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj'
        forbidden = (
            ('-DEBUG:FULL', 'forbidden debug'),
            ('-INCREMENTAL:YES', 'forbidden incremental'),
            ('-LTCG', 'forbidden LTCG'),
            ('-MANIFEST:EMBED', 'forbidden manifest'),
            ('-NODEFAULTLIB', '/NODEFAULTLIB is forbidden'),
            ('-ENTRY:UserEntry', 'custom /ENTRY is forbidden'),
            ('-Brepro', '/Brepro is forbidden'),
        )
        for use_response in (False, True):
            def command(text):
                source.write_text('link.exe @shipping.rsp\n' if use_response else 'link.exe ' + text + '\n', encoding='utf-8')
                if use_response:
                    response.write_text(text + '\n', encoding='utf-8')
            command(flags)
            self.assertEqual([], msvc_evidence.check(self.root, self.build, self.dll))
            command(flags.replace('/', '-'))
            with self.subTest(response_file=use_response, recognized_dash_options=True):
                self.assertEqual([], msvc_evidence.check(self.root, self.build, self.dll))
            for option, diagnostic in forbidden:
                with self.subTest(response_file=use_response, option=option):
                    command(flags + ' ' + option)
                    errors = msvc_evidence.check(self.root, self.build, self.dll)
                    self.assertTrue(any(diagnostic in error for error in errors), errors)


class ValidationReadinessTests(unittest.TestCase):
    head = 'a' * 40
    base = 'b' * 40
    repository = 'owner/project'

    def setUp(self):
        self.event = {'action': 'synchronize', 'number': 23,
                      'before': 'e' * 40, 'after': self.head,
                      'repository': {'default_branch': 'main'},
                      'pull_request': {'head': {'sha': self.head},
                                       'updated_at': '2026-11-04T12:00:00Z',
                                       'base': {'sha': 'c' * 40}}}
        self.current = {'state': 'open', 'created_at': '2026-11-04T10:00:00Z',
                        'head': {'sha': self.head, 'ref': 'feature/sync',
                                 'repo': {'full_name': self.repository}},
                        'base': {'ref': 'main'}}
        self.main = self.base
        self.selected = self.head
        self.comparison = {'status': 'ahead', 'merge_base_commit': {'sha': self.base}}
        self.previous = {'status': 'diverged', 'merge_base_commit': {'sha': 'c' * 40}}
        self.associated = []
        self.calls = []
        self.sync_runs = {1: []}
        self.sync_jobs = {1: []}

    def api(self, endpoint):
        self.calls.append(endpoint)
        prefix = 'repos/' + self.repository
        if endpoint == prefix + '/pulls/23':
            return self.current
        if endpoint == prefix + '/pulls?state=open&base=main&head=owner%3Afeature%2Fsync&per_page=100':
            return self.associated
        if endpoint == prefix + '/git/ref/heads/main':
            return {'object': {'sha': self.main}}
        if endpoint == prefix + '/git/ref/heads/feature%2Fsync':
            return {'object': {'sha': self.selected}}
        if endpoint == prefix + '/compare/' + self.main + '...' + self.head:
            return self.comparison
        if endpoint == prefix + '/compare/' + self.main + '...' + self.event.get('before', ''):
            return self.previous
        match = re.fullmatch(re.escape(prefix + '/actions/workflows/msvc-reference-graph.yml/runs?'
                                       'event=workflow_dispatch&head_sha=' + self.head +
                                       '&per_page=100&page=') + r'([0-9]+)', endpoint)
        if match:
            return {'workflow_runs': self.sync_runs.get(int(match[1]), [])}
        match = re.fullmatch(re.escape(prefix + '/actions/runs/101/jobs?filter=latest&per_page=100&page=') +
                             r'([0-9]+)', endpoint)
        if match:
            return {'jobs': self.sync_jobs.get(int(match[1]), [])}
        self.fail('Unexpected GitHub endpoint: ' + endpoint)

    def decision(self, name='pull_request', ref='refs/heads/feature/sync'):
        if name == 'workflow_dispatch':
            self.event.setdefault('inputs', {'operation': 'diagnostic'})
        return ci_readiness.decide(name, self.event, self.repository, self.head, ref, self.api)

    def test_open_reopen_and_metadata_events_do_not_start_full_tests(self):
        for action in ('opened', 'reopened', 'edited', 'labeled', 'ready_for_review'):
            self.event['action'] = action
            self.assertFalse(self.decision().run_tests)
        self.assertEqual([], self.calls)

    def test_sync_checks_live_main_instead_of_the_event_base_snapshot(self):
        decision = self.decision()
        self.assertTrue(decision.run_tests)
        self.assertEqual(self.base, decision.base_sha)
        self.assertNotEqual(self.event['pull_request']['base']['sha'], decision.base_sha)
        self.assertTrue(any(self.base + '...' + self.head in call for call in self.calls))

    def request_pr_validation(self, verified=True):
        self.event['action'] = 'labeled'
        self.event['label'] = {'name': 'run-full-validation'}
        self.current['labels'] = [{'name': 'run-full-validation'}]
        # A real labeled webhook has neither synchronization identity field.
        del self.event['before']
        del self.event['after']
        if verified:
            self.sync_runs[1] = [{'id': 101, 'event': 'workflow_dispatch',
                                  'path': ci_readiness.VALIDATION_WORKFLOW,
                                  'head_sha': self.head, 'head_branch': 'feature/sync',
                                  'status': 'completed', 'conclusion': 'success',
                                  'created_at': '2026-11-04T11:00:00Z',
                                  'updated_at': '2026-11-04T11:01:00Z'}]
            self.sync_jobs[1] = [{'name': ci_readiness.sync_job_name(23, self.base, self.head),
                                  'status': 'completed', 'conclusion': 'success'}]

    def test_explicit_label_requests_tests_in_pr_context_without_a_head_push(self):
        self.request_pr_validation()
        decision = self.decision()
        self.assertTrue(decision.run_tests)
        self.assertEqual(self.head, decision.head_sha)
        self.assertEqual(self.base, decision.base_sha)
        self.assertIn('repos/owner/project/pulls/23', self.calls)

    def test_initial_label_on_a_current_branch_cannot_start_full_tests(self):
        self.request_pr_validation(verified=False)
        decision = self.decision()
        self.assertFalse(decision.run_tests)
        self.assertIn('subsequent PR synchronization request', decision.reason)

    def test_repeated_initial_labels_cannot_create_their_own_sync_proof(self):
        self.request_pr_validation(verified=False)
        for _ in range(3):
            self.assertFalse(self.decision().run_tests)

    def test_sync_proof_is_bound_to_pr_number_head_and_current_main(self):
        self.request_pr_validation()
        for number, base, head in ((24, self.base, self.head),
                                   (23, 'd' * 40, self.head),
                                   (23, self.base, 'd' * 40)):
            self.sync_jobs[1][0]['name'] = ci_readiness.sync_job_name(number, base, head)
            self.assertFalse(self.decision().run_tests)

    def test_only_a_successful_preparation_job_from_the_selected_workflow_is_proof(self):
        self.request_pr_validation()
        original = dict(self.sync_runs[1][0])
        for key, value in (('event', 'pull_request'), ('path', '.github/workflows/other.yml'),
                           ('head_sha', 'd' * 40), ('head_branch', 'other'),
                           ('status', 'in_progress'), ('conclusion', 'failure')):
            self.sync_runs[1] = [dict(original, **{key: value})]
            self.assertFalse(self.decision().run_tests)
        self.sync_runs[1] = [original]
        for conclusion in ('skipped', 'failure', 'cancelled'):
            self.sync_jobs[1][0]['conclusion'] = conclusion
            self.assertFalse(self.decision().run_tests)

    def test_premature_label_cannot_adopt_a_later_sync_request(self):
        self.request_pr_validation()
        self.event['pull_request']['updated_at'] = '2026-11-04T10:30:00Z'
        self.assertFalse(self.decision().run_tests)
        self.event['pull_request']['updated_at'] = '2026-11-04T12:00:00Z'
        self.assertTrue(self.decision().run_tests)

    def test_preparation_before_opening_or_after_labeling_is_not_proof(self):
        self.request_pr_validation()
        original = dict(self.sync_runs[1][0])
        for changes in ({'created_at': '2026-11-04T09:00:00Z'},
                        {'updated_at': '2026-11-04T13:00:00Z'},
                        {'created_at': '2026-11-04T11:02:00Z'}):
            self.sync_runs[1] = [dict(original, **changes)]
            self.assertFalse(self.decision().run_tests)

    def test_sync_proof_search_paginates_runs_and_jobs(self):
        self.request_pr_validation()
        self.sync_runs[2] = self.sync_runs[1]
        self.sync_runs[1] = [{'event': 'push'}] * 100
        self.sync_jobs[2] = self.sync_jobs[1]
        self.sync_jobs[1] = [{'name': 'Standalone diagnostic'}] * 100
        self.assertTrue(self.decision().run_tests)
        self.assertTrue(any('runs?' in c and c.endswith('page=2') for c in self.calls))
        self.assertTrue(any('jobs?' in c and c.endswith('page=2') for c in self.calls))

    def prepare_sync_request(self):
        self.event['inputs'] = {'operation': 'synchronize-pr', 'pr_number': '23',
                                'sync_base': self.base}

    def record_sync(self, ref='refs/heads/feature/sync'):
        return ci_readiness.record_sync_request('workflow_dispatch', self.event,
                                                self.repository, self.head, ref, self.api)

    def test_later_noop_sync_verification_succeeds_without_starting_tests(self):
        self.prepare_sync_request()
        decision = self.record_sync()
        self.assertFalse(decision.run_tests)
        self.assertEqual(self.head, decision.head_sha)
        self.assertEqual(self.base, decision.base_sha)
        self.assertIn('verified', decision.reason)

    def test_preparation_mode_cannot_fall_through_to_full_diagnostics(self):
        self.prepare_sync_request()
        self.assertFalse(self.decision('workflow_dispatch').run_tests)
        self.assertEqual([], self.calls)

    def test_missing_and_unknown_manual_operations_cannot_start_full_tests(self):
        for inputs in ({}, {'operation': ''}, {'operation': 'sync'}, {'operation': 'DIAGNOSTIC'}):
            self.event['inputs'] = inputs
            self.assertFalse(self.decision('workflow_dispatch').run_tests)
        self.assertEqual([], self.calls)

    def test_sync_verification_requires_the_exact_open_owned_branch(self):
        self.prepare_sync_request()
        for ref in ('refs/heads/main', 'refs/tags/v1'):
            with self.assertRaises(ValueError):
                self.record_sync(ref)
        original = json.loads(json.dumps(self.current))
        for change in ('closed', 'head', 'fork', 'base'):
            self.current = json.loads(json.dumps(original))
            if change == 'closed': self.current['state'] = 'closed'
            if change == 'head': self.current['head']['sha'] = 'd' * 40
            if change == 'fork': self.current['head']['repo']['full_name'] = 'other/project'
            if change == 'base': self.current['base']['ref'] = 'other'
            with self.assertRaises(ValueError):
                self.record_sync()

    def test_sync_verification_rejects_stale_main_and_an_unremoved_initial_label(self):
        self.prepare_sync_request()
        self.current['labels'] = [{'name': 'run-full-validation'}]
        with self.assertRaises(ValueError): self.record_sync()
        self.current['labels'] = []
        self.event['inputs']['sync_base'] = 'd' * 40
        with self.assertRaises(ValueError): self.record_sync()
        self.event['inputs']['sync_base'] = self.base
        self.comparison = {'status': 'diverged'}
        with self.assertRaises(ValueError): self.record_sync()

    def test_preparation_validates_operation_pr_number_and_commit_inputs(self):
        self.prepare_sync_request()
        for number in ('', '0', '-1', '23/other', '023', 23):
            self.event['inputs']['pr_number'] = number
            with self.assertRaises(ValueError): self.record_sync()
        self.event['inputs']['pr_number'] = '23'
        self.event['inputs']['operation'] = 'diagnostic'
        with self.assertRaises(ValueError): self.record_sync()

    def test_bad_sync_receipt_identity_and_timestamp_fail_closed(self):
        self.request_pr_validation()
        self.sync_runs[1][0]['id'] = '101/jobs'
        with self.assertRaises(ValueError): self.decision()
        self.sync_runs[1][0]['id'] = 101
        self.sync_runs[1][0]['updated_at'] = '2026-11-04T11:01:00'
        with self.assertRaises(ValueError): self.decision()

    def run_cli(self, directory, event_name, record=False):
        root = Path(directory)
        event = root / 'event.json'
        event.write_text(json.dumps(self.event))
        output = root / 'output'
        argv = ['ci_validation_readiness.py', '--event-name', event_name,
                '--event-path', str(event), '--repository', self.repository,
                '--sha', self.head, '--ref', 'refs/heads/feature/sync',
                '--output', str(output), '--summary', str(root / 'summary')]
        if record: argv.append('--record-sync')
        with patch.object(sys, 'argv', argv), \
                patch.object(ci_readiness, 'github_json', side_effect=self.api), \
                contextlib.redirect_stdout(io.StringIO()):
            result = ci_readiness.main()
        return result, output.read_text()

    def test_cli_preparation_records_current_identity_without_full_test_permission(self):
        self.prepare_sync_request()
        with tempfile.TemporaryDirectory() as directory:
            status, output = self.run_cli(directory, 'workflow_dispatch', record=True)
        self.assertEqual(0, status)
        self.assertIn('run_tests=false\n', output)
        self.assertIn('head_sha=' + self.head + '\n', output)
        self.assertIn('base_sha=' + self.base + '\n', output)
        self.assertIn('verified', output)

    def test_cli_premature_label_defers_without_fabricating_a_test_failure(self):
        self.request_pr_validation(verified=False)
        with tempfile.TemporaryDirectory() as directory:
            status, output = self.run_cli(directory, 'pull_request')
        self.assertEqual(0, status)
        self.assertIn('run_tests=false\n', output)
        self.assertNotIn('run_tests=true', output)

    def test_cli_failed_preparation_cannot_create_a_successful_receipt(self):
        self.prepare_sync_request()
        self.event['inputs']['sync_base'] = 'd' * 40
        with tempfile.TemporaryDirectory() as directory:
            status, output = self.run_cli(directory, 'workflow_dispatch', record=True)
        self.assertEqual(1, status)
        self.assertIn('run_tests=false\n', output)
        self.assertNotIn('head_sha=' + self.head, output)

    def test_removed_request_label_cannot_start_full_tests(self):
        self.request_pr_validation()
        self.current['labels'] = []
        self.assertFalse(self.decision().run_tests)

    def test_explicit_label_still_requires_current_main_and_current_open_head(self):
        self.request_pr_validation()
        self.comparison = {'status': 'diverged'}
        self.assertFalse(self.decision().run_tests)
        self.comparison = {'status': 'ahead', 'merge_base_commit': {'sha': self.base}}
        self.current['head']['sha'] = 'd' * 40
        self.assertFalse(self.decision().run_tests)
        self.current['head']['sha'] = self.head
        self.current['state'] = 'closed'
        self.assertFalse(self.decision().run_tests)

    def test_retained_label_does_not_turn_ordinary_edits_into_full_test_requests(self):
        self.current['labels'] = [{'name': 'run-full-validation'}]
        self.previous = {'status': 'ahead', 'merge_base_commit': {'sha': self.base}}
        self.assertFalse(self.decision().run_tests)

    def test_other_label_actions_do_not_request_full_tests(self):
        self.event['label'] = {'name': 'changelog-exempt'}
        self.event['action'] = 'labeled'
        self.assertFalse(self.decision().run_tests)
        self.event['label'] = {'name': 'run-full-validation'}
        self.event['action'] = 'unlabeled'
        self.assertFalse(self.decision().run_tests)
        self.assertEqual([], self.calls)

    def test_regular_commit_on_an_already_synced_branch_waits_for_manual_request(self):
        self.previous = {'status': 'ahead', 'merge_base_commit': {'sha': self.base}}
        self.assertFalse(self.decision().run_tests)
        self.assertTrue(self.decision('workflow_dispatch').run_tests)

    def test_missing_previous_head_cannot_claim_a_main_integration(self):
        del self.event['before']
        with self.assertRaises(KeyError):
            self.decision()

    def test_inconsistent_previous_comparison_cannot_start_tests(self):
        for previous in ({}, {'status': 'ahead', 'merge_base_commit': {'sha': 'c' * 40}},
                         {'status': 'diverged', 'merge_base_commit': {'sha': self.base}}):
            self.previous = previous
            self.assertFalse(self.decision().run_tests)

    def test_update_after_sha_must_match_the_actual_pr_head(self):
        self.event['after'] = 'd' * 40
        self.assertFalse(self.decision().run_tests)

    def test_stale_diverged_and_malformed_comparisons_are_not_ready(self):
        for comparison in ({'status': 'behind'}, {'status': 'diverged'}, {},
                           {'status': 'ahead', 'merge_base_commit': {'sha': 'c' * 40}}):
            self.comparison = comparison
            self.assertFalse(self.decision().run_tests)

    def test_identical_manual_main_is_allowed(self):
        self.head = self.base
        self.comparison = {'status': 'identical', 'merge_base_commit': {'sha': self.base}}
        self.assertTrue(self.decision('workflow_dispatch', 'refs/heads/main').run_tests)

    def test_closed_changed_or_non_main_pr_is_not_ready(self):
        for current in ({'state': 'closed', 'head': {'sha': self.head}, 'base': {'ref': 'main'}},
                        {'state': 'open', 'head': {'sha': 'd' * 40}, 'base': {'ref': 'main'}},
                        {'state': 'open', 'head': {'sha': self.head}, 'base': {'ref': 'research'}}):
            self.current = current
            self.assertFalse(self.decision().run_tests)

    def test_manual_dispatch_requires_the_selected_current_branch_head(self):
        self.assertTrue(self.decision('workflow_dispatch').run_tests)
        self.selected = 'd' * 40
        self.assertFalse(self.decision('workflow_dispatch').run_tests)
        self.assertFalse(self.decision('workflow_dispatch', 'refs/tags/v1').run_tests)

    def test_manual_stale_branch_does_not_allocate_full_tests(self):
        self.comparison = {'status': 'diverged'}
        self.assertFalse(self.decision('workflow_dispatch').run_tests)

    def test_changelog_exemption_uses_live_pr_labels(self):
        self.assertFalse(self.decision().changelog_exempt)
        self.current['labels'] = [{'name': 'changelog-exempt'}]
        self.assertTrue(self.decision().changelog_exempt)

    def test_manual_branch_keeps_changelog_requirement_unless_approved(self):
        self.assertFalse(self.decision('workflow_dispatch').changelog_exempt)
        self.associated = [self.current]
        self.assertFalse(self.decision('workflow_dispatch').changelog_exempt)
        self.current['labels'] = [{'name': 'changelog-exempt'}]
        self.assertTrue(self.decision('workflow_dispatch').changelog_exempt)

    def test_manual_request_cannot_borrow_exemption_from_another_head(self):
        self.associated = [{'state': 'open', 'base': {'ref': 'main'},
                            'head': {'sha': 'd' * 40},
                            'labels': [{'name': 'changelog-exempt'}]}]
        self.assertFalse(self.decision('workflow_dispatch').changelog_exempt)

    def test_bad_commit_and_repository_identities_are_rejected(self):
        self.event['inputs'] = {'operation': 'diagnostic'}
        with self.assertRaises(ValueError):
            ci_readiness.decide('workflow_dispatch', self.event, '../project', self.head,
                                'refs/heads/main', self.api)
        self.event['pull_request']['head']['sha'] = self.head + '\nrun_tests=true'
        with self.assertRaises(ValueError):
            self.decision()

    def test_cli_api_failure_writes_false_and_returns_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            event = root / 'event.json'
            event.write_text(json.dumps(self.event))
            output, summary = root / 'output', root / 'summary'
            argv = ['ci_validation_readiness.py', '--event-name', 'pull_request',
                    '--event-path', str(event), '--repository', self.repository,
                    '--output', str(output), '--summary', str(summary)]
            with patch.object(sys, 'argv', argv), \
                    patch.object(ci_readiness, 'github_json', side_effect=RuntimeError('API failure')), \
                    contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(1, ci_readiness.main())
            self.assertIn('run_tests=false\n', output.read_text())
            self.assertNotIn('run_tests=true', output.read_text())
            self.assertIn('could not be verified', summary.read_text())

    def test_new_main_after_a_prior_success_invalidates_eligibility(self):
        self.assertTrue(self.decision().run_tests)
        self.main = 'd' * 40
        self.comparison = {'status': 'diverged', 'merge_base_commit': {'sha': self.base}}
        self.assertFalse(self.decision().run_tests)

    def test_deferred_jobs_are_gated_before_matrices_and_runner_allocation(self):
        workflow = (ROOT / '.github/workflows/msvc-reference-graph.yml').read_text()
        trigger = workflow.split('\npermissions:', 1)[0]
        self.assertIn('types: [synchronize, labeled]', trigger)
        self.assertIn("github.event.label.name != 'run-full-validation' && 'unrequested-label'", workflow)
        self.assertIn('  push:\n    branches: [main]\n', trigger)
        self.assertNotIn('pull_request_target', workflow)
        blocks = dict(re.findall(r'^  ([a-z-]+):\n(.*?)(?=^  [a-z-]+:|\Z)',
                                 workflow.split('\njobs:\n', 1)[1], re.M | re.S))
        required = {'source-qa', 'windows', 'windows-portable', 'reference-graph', 'codeql'}
        self.assertEqual(required | {'readiness', 'sync-request'}, set(blocks))
        self.assertIn("if: github.event_name != 'push' && (github.event_name != 'workflow_dispatch' || inputs.operation == 'diagnostic')", blocks['readiness'])
        self.assertIn("if: github.event_name == 'workflow_dispatch' && inputs.operation == 'synchronize-pr'", blocks['sync-request'])
        self.assertIn('scripts/ci_validation_readiness.py --record-sync', blocks['sync-request'])
        self.assertIn("inputs.operation == 'synchronize-pr' && 'sync-request'", workflow)
        self.assertIn("name: 'PR sync #${{ inputs.pr_number }}: ${{ inputs.sync_base }} / ${{ github.sha }}'", blocks['sync-request'])
        for name in required - {'codeql'}:
            block = blocks[name]
            self.assertIn("\n    if: ${{ needs.readiness.result == 'success' && needs.readiness.outputs.run_tests == 'true' }}", block)
            self.assertIn('needs: readiness', block)
            self.assertNotIn('Require verified main synchronization', block)
            self.assertNotIn('if: ${{ always() }}', block)
            self.assertIn('ref: ${{ needs.readiness.outputs.head_sha }}', block)
            if name not in {'source-qa', 'codeql'}:
                self.assertIn('runs-on: windows-2022', block)
                self.assertNotIn('ubuntu-24.04', block)
                self.assertIn('    strategy:\n', block)
        profiles = set(re.findall(r'- profile: ([a-z-]+)', blocks['windows']))
        contexts = {'Source QA and portable tests'} | {'Windows ' + p for p in profiles}
        contexts |= {'Windows portable ' + c for c in ('Debug', 'Release')}
        contexts |= {'Windows x64 reference objects ' + c for c in ('Debug', 'Release')}
        self.assertEqual(11, len(contexts))
        self.assertEqual({'native', 'native-debug', 'production', 'crt', 'debug', 'compile'}, profiles)
        self.assertIn("name: ${{ needs.readiness.outputs.run_tests == 'true' && 'Source QA and portable tests' || 'Source QA deferred' }}", blocks['source-qa'])
        self.assertIn("needs.readiness.outputs.changelog_exempt != 'true'", blocks['source-qa'])
        self.assertIn('needs.readiness.outputs.head_sha != needs.readiness.outputs.base_sha', blocks['source-qa'])
        self.assertIn('name: Windows ${{ matrix.profile }}', blocks['windows'])
        self.assertIn('name: Windows portable ${{ matrix.configuration }}', blocks['windows-portable'])
        self.assertIn('name: Windows x64 reference objects ${{ matrix.configuration }}', blocks['reference-graph'])
        self.assertIn('runs-on: ubuntu-24.04', blocks['codeql'])
        self.assertIn('language: [actions, c-cpp, python]', blocks['codeql'])
        self.assertIn('security-events: write', blocks['codeql'])
        selected_sha = "${{ github.sha }}"
        self.assertIn('ref: ' + selected_sha, blocks['codeql'])
        self.assertIn('sha: ' + selected_sha, blocks['codeql'])
        self.assertNotIn('needs:', blocks['codeql'])
        self.assertIn('build-mode: none', blocks['codeql'])
        self.assertNotIn('always()', blocks['codeql'])
        self.assertIn('ref: refs/heads/main', blocks['codeql'])
        self.assertNotIn('refs/pull/', blocks['codeql'])

    @staticmethod
    def workflow_expression(expression, context):
        """Evaluate only the boolean/selection subset used by the actual job gates.

        No Python eval or GitHub API is used. Unknown syntax fails the regression
        instead of silently modelling an unsupported workflow expression.
        """
        expression = expression.removeprefix('${{').removesuffix('}}').strip()
        expression = re.sub(r'!(?!=)', ' not ', expression).replace('&&', ' and ').replace('||', ' or ')

        def visit(node):
            if isinstance(node, ast.Constant):
                return node.value
            if isinstance(node, ast.Name):
                return context[node.id]
            if isinstance(node, ast.Attribute):
                return visit(node.value)[node.attr]
            if isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and node.func.id == 'cancelled':
                assert not node.args and not node.keywords
                return context['cancelled']
            if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.Not):
                return not visit(node.operand)
            if isinstance(node, ast.BoolOp):
                result = visit(node.values[0])
                for value in node.values[1:]:
                    if isinstance(node.op, ast.And) and not result:
                        return result
                    if isinstance(node.op, ast.Or) and result:
                        return result
                    result = visit(value)
                return result
            if isinstance(node, ast.Compare) and len(node.ops) == 1:
                left, right = visit(node.left), visit(node.comparators[0])
                if isinstance(node.ops[0], ast.Eq):
                    return left == right
                if isinstance(node.ops[0], ast.NotEq):
                    return left != right
            raise AssertionError('Unsupported workflow expression: ' + ast.dump(node))

        return visit(ast.parse(expression.strip(), mode='eval').body)

    def test_main_push_runs_only_codeql_even_when_readiness_is_skipped(self):
        workflow = (ROOT / '.github/workflows/msvc-reference-graph.yml').read_text()
        blocks = dict(re.findall(r'^  ([a-z-]+):\n(.*?)(?=^  [a-z-]+:|\Z)',
                                 workflow.split('\njobs:\n', 1)[1], re.M | re.S))
        gates = {name: re.search(r'^    if: (.+)$', block, re.M)[1]
                 for name, block in blocks.items()}
        # Exercise the actual job expressions for main, ready/unready PRs,
        # manual diagnostics, preparation and cancellation. Only accepted-main
        # pushes allocate CodeQL; all eleven existing merge contexts stay gated.
        self.assertIn('!cancelled()', gates['codeql'])
        builds = {'source-qa', 'windows', 'windows-portable', 'reference-graph'}
        scenarios = [
            ('push', 'refs/heads/main', '', 'skipped', '', False, {'codeql'}),
            ('push', 'refs/heads/feature', '', 'skipped', '', False, set()),
            ('push', 'refs/heads/main', '', 'skipped', '', True, set()),
            ('pull_request', 'refs/pull/1/merge', '', 'success', 'true', False, builds),
            ('pull_request', 'refs/pull/1/merge', '', 'success', 'false', False, set()),
            ('pull_request', 'refs/pull/1/merge', '', 'failure', 'true', False, set()),
            ('pull_request', 'refs/pull/1/merge', '', 'cancelled', 'true', True, set()),
            ('workflow_dispatch', 'refs/heads/feature', 'diagnostic', 'success', 'true', False, builds),
            ('workflow_dispatch', 'refs/heads/main', 'diagnostic', 'success', 'true', False, builds),
            ('workflow_dispatch', 'refs/heads/feature', 'synchronize-pr', 'skipped', '', False, set()),
        ]
        for event, ref, operation, result, run_tests, cancelled, expected in scenarios:
            with self.subTest(event=event, ref=ref, operation=operation, result=result, run_tests=run_tests, cancelled=cancelled):
                context = {'github': {'event_name': event, 'ref': ref, 'sha': 'a'*40},
                           'inputs': {'operation': operation}, 'cancelled': cancelled,
                           'needs': {'readiness': {'result': result, 'outputs': {
                               'run_tests': run_tests, 'head_sha': 'b'*40}}}}
                runnable = {name for name in builds | {'codeql'}
                            if self.workflow_expression(gates[name], context)
                            and (name == 'codeql' or (result == 'success' and not cancelled))}
                self.assertEqual(expected, runnable)
                readiness = self.workflow_expression(gates['readiness'], context)
                self.assertEqual(event != 'push' and operation != 'synchronize-pr', readiness)
                prepare = self.workflow_expression(gates['sync-request'], context)
                self.assertEqual(event == 'workflow_dispatch' and operation == 'synchronize-pr', prepare)
                selection = re.search(r'^          sha: (.+)$', blocks['codeql'], re.M)[1]
                self.assertEqual('a'*40,
                                 self.workflow_expression(selection, context))
        trigger = workflow.split('\npermissions:', 1)[0]
        self.assertNotIn('opened', trigger)
        self.assertIn("github.event.pull_request.head.ref || github.ref_name", workflow)
        self.assertIn('cancel-in-progress: true', workflow)

    def test_opening_ci_is_integrity_only_and_diagnostics_are_manual(self):
        fast = (ROOT / '.github/workflows/ci.yml').read_text()
        self.assertIn('scripts/verify_sources.py', fast)
        self.assertIn('scripts/check_manifest.py', fast)
        self.assertNotIn('run_source_qa.py', fast)
        self.assertNotIn('cmake --', fast)
        jobs = re.findall(r'^  ([a-z-]+):\n', fast.split('\njobs:\n', 1)[1], re.M)
        self.assertEqual(['source-integrity'], jobs)
        self.assertNotIn('setup-python', fast)
        self.assertNotIn('codeql-action', fast)
        self.assertNotIn('strategy:', fast)
        for path in (ROOT / '.github/workflows').glob('*.yml'):
            if path.name in ('ci.yml', 'msvc-reference-graph.yml', 'weekly-release.yml', 'repository-badges.yml'):
                continue
            trigger = path.read_text().split('\non:\n', 1)[1].split('\npermissions:', 1)[0]
            self.assertIn('  workflow_dispatch:', trigger, path.name)
            self.assertNotIn('pull_request:', trigger, path.name)
            self.assertNotIn('  push:', trigger, path.name)


class IdentityBoundaryTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)
        (self.root/'docs').mkdir()
        (self.root/'src').mkdir()
        (self.root/'metadata').mkdir()
        (self.root/'docs/IDENTIFIER_POLICY.md').write_bytes((ROOT/'docs/IDENTIFIER_POLICY.md').read_bytes())
        self.label=repository.load_active_name_policy(self.root)[0].upper()

    def test_documents_are_allowed_but_active_comments_and_schemas_are_rejected(self):
        (self.root/'docs/history.json').write_text(json.dumps({'origin':self.label}))
        (self.root/'LICENSE').write_text('Source attribution: '+self.label)
        self.assertEqual([],repository.check_active_identifiers(self.root))
        source=self.root/'src/module.cpp'
        source.write_text('// '+self.label+'\n')
        self.assertTrue(repository.check_active_identifiers(self.root))
        source.unlink()
        (self.root/'metadata/current.json').write_text(json.dumps({'schema':self.label.lower()+'.test.v1'}))
        self.assertTrue(repository.check_active_identifiers(self.root))

    def test_active_paths_are_checked_and_pinned_vendor_content_is_excluded(self):
        (self.root/'third_party').mkdir()
        (self.root/'third_party/vendor.txt').write_text(self.label)
        self.assertEqual([],repository.check_active_identifiers(self.root))
        (self.root/'src'/(self.label+'.hpp')).write_text('#pragma once\n')
        self.assertTrue(repository.check_active_identifiers(self.root))

    def test_namespace_boundary_uses_the_owned_root(self):
        for folder in ['include','tests','third_party']:(self.root/folder).mkdir()
        path=self.root/'src/module.cpp'
        for namespace,accepted in [('citrine_eclipse_mirage::module',True),('upstream::module',False)]:
            payload=('namespace '+namespace+' {}\n').encode('utf-8');path.write_bytes(payload)
            (self.root/'metadata/SOURCE_FILES.json').write_text(json.dumps({'files':{'src/module.cpp':{'bytes':len(payload),'sha256':inventory.digest(payload)}}}))
            self.assertEqual(accepted,source_verification.verify(self.root)['status']=='PASS')

    def test_source_sizes_are_required_integer_byte_counts(self):
        for folder in ['include','tests','third_party']:(self.root/folder).mkdir()
        # UTF-8 and CRLF distinguish bytes from characters and normalized text.
        payload=b'// \xc3\xa9\r\nnamespace citrine_eclipse_mirage::fixture {}\r\n'
        (self.root/'src/module.cpp').write_bytes(payload)
        manifest=self.root/'metadata/SOURCE_FILES.json'
        entry={'bytes':len(payload),'sha256':inventory.digest(payload)}
        def verify(record):
            manifest.write_text(json.dumps({'files':{'src/module.cpp':record}}))
            return source_verification.verify(self.root)
        self.assertEqual('PASS',verify(entry)['status'])
        for size in [len(payload)-1,len(payload)+1,len(payload.decode('utf-8')),
                     -1,True,float(len(payload)),str(len(payload)),None]:
            with self.subTest(size=size):
                result=verify(dict(entry,bytes=size))
                self.assertEqual('FAIL',result['status'])
                self.assertTrue(any('source size' in error.lower() for error in result['errors']))
                self.assertFalse(any('Source differs:' in error for error in result['errors']))
        self.assertEqual('FAIL',verify({'sha256':entry['sha256']})['status'])
        self.assertEqual('PASS',verify(entry)['status'])


    def test_source_verifier_cli_reports_result_and_exit_status(self):
        for folder in ('include', 'tests', 'third_party', 'scripts'):
            (self.root / folder).mkdir()
        for name in ('verify_sources.py', 'project_files.py'):
            shutil.copyfile(ROOT / 'scripts' / name, self.root / 'scripts' / name)
        payload = b'namespace citrine_eclipse_mirage::fixture {}\n'
        (self.root / 'src/module.cpp').write_bytes(payload)
        manifest = self.root / 'metadata/SOURCE_FILES.json'
        for valid in (True, False):
            with self.subTest(valid=valid):
                manifest.write_text(json.dumps({'files': {'src/module.cpp': {
                    'bytes': len(payload),
                    'sha256': inventory.digest(payload) if valid else '0' * 64,
                }}}), encoding='utf-8')
                result = subprocess.run(
                    [sys.executable, '-B', str(self.root / 'scripts/verify_sources.py')],
                    cwd=self.root, capture_output=True, text=True, timeout=10)
                self.assertEqual(0 if valid else 1, result.returncode, result.stderr)
                report = json.loads(result.stdout)
                self.assertEqual('PASS' if valid else 'FAIL', report['status'])
                self.assertEqual(1, report['files_checked'])
                self.assertEqual(not valid, bool(report['errors']))

    def test_duplicate_source_inventory_object_keys_fail_closed(self):
        for folder in ('include', 'tests', 'third_party'):
            (self.root / folder).mkdir()
        payload=b'namespace citrine_eclipse_mirage::fixture {}\n'
        (self.root / 'src/module.cpp').write_bytes(payload)
        entry={'bytes':len(payload), 'sha256':inventory.digest(payload)}
        serialized=json.dumps(entry, separators=(',', ':'))
        record='"src/module.cpp":'+serialized
        valid='{"files":{'+record+'}}'
        destination=self.root / 'metadata/SOURCE_FILES.json'
        destination.write_text(valid, encoding='utf-8')
        self.assertEqual('PASS', source_verification.verify(self.root)['status'])
        ambiguous=[
            '{"files":{'+record+','+record+'}}',
            '{"files":{},"files":{'+record+'}}',
            '{"files":{"src/module.cpp":{"bytes":'+str(len(payload))+
                ',"bytes":'+str(len(payload))+',"sha256":'+json.dumps(entry['sha256'])+'}}}',
        ]
        for index, body in enumerate(ambiguous):
            with self.subTest(duplicate=index):
                # Standard json.loads silently accepts the semantically identical
                # duplicate; the source verifier must not.
                self.assertEqual(json.loads(valid), json.loads(body))
                destination.write_text(body, encoding='utf-8')
                with self.assertRaisesRegex(ValueError, 'Duplicate JSON object key'):
                    source_verification.verify(self.root)
        destination.write_text(valid, encoding='utf-8')
        self.assertEqual('PASS', source_verification.verify(self.root)['status'])


class PackageTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.root = self.base / 'source'
        (self.root / 'metadata').mkdir(parents=True)
        (self.root / 'src').mkdir()
        (self.root / 'src/value.cpp').write_text('int value() { return 7; }\n')
        (self.root / 'CMakeLists.txt').write_text('fixture only\n')
        (self.root / 'metadata/REPOSITORY_CONTRACT.json').write_text(json.dumps({
            'required_files': ['CMakeLists.txt'], 'source_directories': ['src', 'metadata']}))
        inventory.write_manifest(self.root)
        self.zip = self.base / 'source.zip'

    def test_manifest_refresh_replaces_hardlink_without_modifying_other_owner(self):
        destination = self.root / inventory.MANIFEST
        original = destination.read_bytes()
        unrelated = self.base / 'hardlink-owner.txt'
        unrelated.write_bytes(original)
        destination.unlink()
        try:
            os.link(unrelated, destination)
        except OSError:
            self.skipTest('hardlink creation unavailable on this filesystem')
        (self.root / 'src/value.cpp').write_text('int value() { return 9; }\n')
        inventory.write_manifest(self.root)
        self.assertEqual(original, unrelated.read_bytes(),
                         'manifest refresh must not truncate the other hardlink owner')
        self.assertNotEqual(original, destination.read_bytes())
        self.assertEqual([], inventory.verify_manifest(self.root))
        self.assertFalse(list(destination.parent.glob('.' + destination.name + '.*.partial')))

    def test_manifest_refresh_replace_failure_preserves_prior_manifest(self):
        destination = self.root / inventory.MANIFEST
        original = destination.read_bytes()
        (self.root / 'src/value.cpp').write_text('int value() { return 9; }\n')
        with patch.object(inventory.os, 'replace', side_effect=OSError('synthetic manifest commit failure')):
            with self.assertRaisesRegex(OSError, 'synthetic manifest commit failure'):
                inventory.write_manifest(self.root)
        self.assertEqual(original, destination.read_bytes(),
                         'failed publication must retain the complete old manifest')
        self.assertFalse(list(destination.parent.glob('.' + destination.name + '.*.partial')))
        self.assertTrue(inventory.verify_manifest(self.root),
                        'the previous manifest must still reject changed source bytes')
        inventory.write_manifest(self.root)
        self.assertEqual([], inventory.verify_manifest(self.root))

    def test_extractor_rejects_high_expansion_before_crc_and_preserves_cleanup(self):
        # The source package is manifest-consistent: rejection is the budget,
        # not a hash, CRC, filename, or ownership mismatch.
        payload = b'X' * (128 * 1024)
        (self.root / 'src/compressible.txt').write_bytes(payload)
        inventory.write_manifest(self.root)
        packaging.package(self.root, self.zip)
        with zipfile.ZipFile(self.zip) as archive:
            member = archive.getinfo('src/compressible.txt')
            self.assertLess(member.compress_size, member.file_size // 100)
        destination = self.base / 'expanded'
        with patch.object(extraction, 'MAX_MEMBER_BYTES', 64 * 1024):
            # The CRC pass may not decompress the disallowed member first.
            with patch.object(zipfile.ZipFile, 'testzip', side_effect=AssertionError('early decompression')):
                with self.assertRaisesRegex(ValueError, 'member exceeds extraction budget'):
                    extraction.extract(self.zip, destination)
        self.assertFalse(destination.exists())
        self.assertFalse(list(self.base.glob('.surge-extract-*')))

    def test_extractor_rejects_cumulative_expansion_before_crc(self):
        packaging.package(self.root, self.zip)
        with zipfile.ZipFile(self.zip) as archive:
            largest = max(info.file_size for info in archive.infolist())
            total = sum(info.file_size for info in archive.infolist())
        self.assertGreater(total, largest)
        destination = self.base / 'expanded'
        with patch.object(extraction, 'MAX_MEMBER_BYTES', largest), \
             patch.object(extraction, 'MAX_ARCHIVE_BYTES', total - 1), \
             patch.object(zipfile.ZipFile, 'testzip', side_effect=AssertionError('early decompression')):
            with self.assertRaisesRegex(ValueError, 'total extraction budget'):
                extraction.extract(self.zip, destination)
        self.assertFalse(destination.exists())
        self.assertFalse(list(self.base.glob('.surge-extract-*')))

    def test_flat_roundtrip_and_determinism(self):
        first = packaging.package(self.root, self.zip)
        second = packaging.package(self.root, self.base / 'second.zip')
        self.assertEqual(first['sha256'], second['sha256'])
        self.assertEqual(first['sha256'] + '  source.zip\n', Path(first['sha256_file']).read_text())
        self.assertEqual(second['sha256'] + '  second.zip\n', Path(second['sha256_file']).read_text())
        self.assertEqual([], falcion.check(self.root, self.zip))
        with zipfile.ZipFile(self.zip) as archive:
            self.assertIn('CMakeLists.txt', archive.namelist())
            self.assertEqual({'CMakeLists.txt','src','metadata'}, {Path(name).parts[0] for name in archive.namelist()})

    @contextlib.contextmanager
    def ansi_default_text_reads(self):
        read_text = Path.read_text
        reads = []
        def ansi_read_text(path, encoding=None, errors=None, **kwargs):
            reads.append(path)
            return read_text(path, encoding=encoding or 'cp1252', errors=errors, **kwargs)
        with patch.object(Path, 'read_text', ansi_read_text):
            yield reads

    def unicode_source_metadata(self, selected, ascii_only):
        member = 'src/file_\u00e4\u00f6\u00fc\U0001d11e_data.txt'
        (self.root / member).write_bytes(b'owned Unicode-path source fixture\r\n')
        contract = {'required_files': ['CMakeLists.txt', member],
                    'source_directories': ['src', 'metadata']}
        contract_path = self.root / 'metadata/REPOSITORY_CONTRACT.json'
        contract_path.write_bytes(json.dumps(contract).encode('utf-8'))
        manifest = inventory.write_manifest(self.root)
        document = manifest if selected == inventory.MANIFEST else contract
        (self.root / selected).write_bytes(
            (json.dumps(document, ensure_ascii=ascii_only) + '\n').encode('utf-8'))
        if selected != inventory.MANIFEST:
            inventory.write_manifest(self.root)
        return member

    def test_utf8_package_metadata_is_independent_of_ansi_default(self):
        for selected in [inventory.MANIFEST, 'metadata/REPOSITORY_CONTRACT.json']:
            for ascii_only in [True, False]:
                with self.subTest(selected=selected, ascii_only=ascii_only):
                    member = self.unicode_source_metadata(selected, ascii_only)
                    with self.ansi_default_text_reads() as reads:
                        self.assertEqual([], inventory.verify_manifest(self.root))
                        self.assertEqual([], falcion.check(self.root))
                        receipt = packaging.package(self.root, self.zip)
                        self.assertEqual([], falcion.check(self.root, self.zip))
                    self.assertIn(self.root / inventory.MANIFEST, reads)
                    self.assertIn(self.root / 'metadata/REPOSITORY_CONTRACT.json', reads)
                    self.assertEqual(receipt['sha256'], inventory.digest(self.zip.read_bytes()))
                    with zipfile.ZipFile(self.zip) as archive:
                        for name in [member, inventory.MANIFEST, 'metadata/REPOSITORY_CONTRACT.json']:
                            self.assertEqual((self.root / name).read_bytes(), archive.read(name))

    def test_invalid_utf8_metadata_cannot_replace_a_complete_package(self):
        for selected in [inventory.MANIFEST, 'metadata/REPOSITORY_CONTRACT.json']:
            with self.subTest(selected=selected):
                self.unicode_source_metadata(selected, True)
                packaging.package(self.root, self.zip)
                sidecar = Path(str(self.zip) + '.sha256')
                prior = self.zip.read_bytes(), sidecar.read_bytes()
                path = self.root / selected
                original = path.read_bytes()
                document = json.loads(original)
                document['fixture_note'] = 'placeholder'
                path.write_bytes(json.dumps(document).encode('utf-8').replace(
                    b'"fixture_note": "placeholder"', b'"fixture_note": "\xff"'))
                if selected != inventory.MANIFEST:
                    inventory.write_manifest(self.root)
                with self.ansi_default_text_reads():
                    with self.assertRaises(UnicodeDecodeError):
                        falcion.check(self.root)
                    with self.assertRaises(UnicodeDecodeError):
                        packaging.package(self.root, self.zip)
                self.assertEqual(prior, (self.zip.read_bytes(), sidecar.read_bytes()))
                self.assertFalse(list(self.base.glob('.*.partial')))
                self.assertFalse(list(self.base.glob('.*.rollback')))
                path.write_bytes(original)
                inventory.write_manifest(self.root)
                self.assertEqual([], falcion.check(self.root))


    def test_checksum_commit_failure_retains_previous_complete_package(self):
        previous = packaging.package(self.root, self.zip)
        sidecar = Path(previous['sha256_file'])
        original_zip, original_sidecar = self.zip.read_bytes(), sidecar.read_bytes()
        (self.root / 'src/value.cpp').write_text('int value() { return 9; }\n')
        inventory.write_manifest(self.root)

        replace = packaging.os.replace
        def reject_checksum(source, destination):
            if Path(destination) == sidecar:
                raise OSError('injected checksum publication error')
            return replace(source, destination)

        with patch.object(packaging.os, 'replace', side_effect=reject_checksum):
            with self.assertRaisesRegex(OSError, 'injected checksum publication error'):
                packaging.package(self.root, self.zip)
        self.assertEqual(original_zip, self.zip.read_bytes())
        self.assertEqual(original_sidecar, sidecar.read_bytes())
        self.assertFalse(list(self.base.glob('.*.partial')))
        self.assertFalse(list(self.base.glob('.*.rollback')))

        replacement = packaging.package(self.root, self.zip)
        self.assertNotEqual(previous['sha256'], replacement['sha256'])
        self.assertEqual(replacement['sha256'] + '  source.zip\n', sidecar.read_text())

    def test_checksum_commit_failure_without_old_pair_removes_new_zip(self):
        sidecar = Path(str(self.zip) + '.sha256')
        replace = packaging.os.replace
        def reject_checksum(source, destination):
            if Path(destination) == sidecar:
                raise OSError('injected checksum publication error')
            return replace(source, destination)
        with patch.object(packaging.os, 'replace', side_effect=reject_checksum):
            with self.assertRaisesRegex(OSError, 'injected checksum publication error'):
                packaging.package(self.root, self.zip)
        self.assertFalse(self.zip.exists())
        self.assertFalse(sidecar.exists())
        self.assertFalse(list(self.base.glob('.*.partial')))

    def test_incomplete_existing_pair_is_not_overwritten(self):
        self.zip.write_bytes(b'old ZIP without receipt')
        with self.assertRaisesRegex(ValueError, 'incomplete source ZIP/checksum pair'):
            packaging.package(self.root, self.zip)
        self.assertEqual(b'old ZIP without receipt', self.zip.read_bytes())
        self.zip.unlink()
        sidecar = Path(str(self.zip) + '.sha256')
        sidecar.write_text('orphaned receipt\n')
        with self.assertRaisesRegex(ValueError, 'incomplete source ZIP/checksum pair'):
            packaging.package(self.root, self.zip)
        self.assertEqual('orphaned receipt\n', sidecar.read_text())

    def test_preexisting_package_partials_cannot_redirect_writes(self):
        victim = self.base / 'unrelated-fixture.txt'
        victim.write_bytes(b'unmodified audit fixture')
        for name in ('source.zip.partial', 'source.zip.sha256.partial'):
            with self.subTest(partial=name):
                unsafe = self.base / name
                try:
                    unsafe.symlink_to(victim)
                except (OSError, NotImplementedError):
                    self.skipTest('Symlink fixture unavailable on this host')
                try:
                    with self.assertRaisesRegex(ValueError, 'Unsafe existing source package partial path'):
                        packaging.package(self.root, self.zip)
                    self.assertEqual(b'unmodified audit fixture', victim.read_bytes())
                    self.assertFalse(self.zip.exists())
                    self.assertFalse(list(self.base.glob('.source.zip.*.partial')))
                finally:
                    unsafe.unlink()

    def test_symlinked_package_output_is_rejected_without_modifying_target(self):
        victim = self.base / 'unrelated-fixture.txt'
        victim.write_bytes(b'unchanged output target')
        try:
            self.zip.symlink_to(victim)
        except (OSError, NotImplementedError):
            self.skipTest('Symlink fixture unavailable on this host')
        with self.assertRaisesRegex(ValueError, 'symlinked source ZIP'):
            packaging.package(self.root, self.zip)
        self.assertEqual(b'unchanged output target', victim.read_bytes())
        self.assertTrue(self.zip.is_symlink())

    def test_source_drift_is_not_automatically_legitimized(self):
        (self.root / 'src/value.cpp').write_text('changed')
        with self.assertRaises(ValueError):
            packaging.package(self.root, self.zip)
        self.assertFalse(self.zip.exists())


    def test_duplicate_active_manifest_object_keys_fail_closed(self):
        path=self.root / inventory.MANIFEST
        original=path.read_text(encoding='utf-8')
        self.assertEqual([], inventory.verify_manifest(self.root))
        original_files=json.loads(original)['files']
        name='src/value.cpp'
        existing=json.dumps(name)+': '+json.dumps(original_files[name])
        self.assertIn(existing, original)
        malformed=[
            original.replace(existing,existing+',\n    '+existing,1),
            original.replace('"files": {','"files": {},\n  "files": {',1),
            original.replace('"schema": "'+inventory.MANIFEST_SCHEMA+'",',
                '"schema": "'+inventory.MANIFEST_SCHEMA+'",\n  "schema": "'+
                inventory.MANIFEST_SCHEMA+'",',1),
        ]
        try:
            for index, body in enumerate(malformed):
                with self.subTest(duplicate=index):
                    self.assertNotEqual(body, original)
                    self.assertEqual(json.loads(original), json.loads(body))
                    path.write_text(body, encoding='utf-8')
                    with self.assertRaisesRegex(ValueError, 'Duplicate JSON object key'):
                        inventory.verify_manifest(self.root)
        finally:
            path.write_text(original, encoding='utf-8')
        self.assertEqual([], inventory.verify_manifest(self.root))

    def test_source_manifest_schema_and_release_classification_fail_closed(self):
        manifest_path = self.root / inventory.MANIFEST
        original = manifest_path.read_bytes()
        content = json.loads(original)
        self.assertEqual(inventory.MANIFEST_SCHEMA, content['schema'])
        self.assertNotIn('classification', content)
        self.assertEqual([], inventory.verify_manifest(self.root))

        for mutate, expected in (
                ({'schema': 'unexpected.audit.schema'}, 'Unrecognized active source manifest schema'),
                ({'classification': 'RELEASE'}, 'Release classification does not belong')):
            with self.subTest(mutate=mutate):
                modified = dict(content)
                modified.update(mutate)
                manifest_path.write_text(json.dumps(modified) + '\n', encoding='utf-8')
                self.assertTrue(any(expected in message
                                    for message in inventory.verify_manifest(self.root)))
                manifest_path.write_bytes(original)
        self.assertEqual([], inventory.verify_manifest(self.root))

    def test_unrecorded_source_is_rejected(self):
        (self.root / 'src/extra.cpp').write_text('unexpected')
        self.assertTrue(inventory.verify_manifest(self.root))

    def test_embedded_archives_and_binaries_are_rejected(self):
        for name in ['input.zip', 'input.tar.gz', 'old.dll', 'old.pdb']:
            path = self.root / 'src' / name
            path.write_bytes(b'payload')
            with self.assertRaises(ValueError):
                inventory.source_files(self.root)
            path.unlink()

    def test_falcion_preflights_member_sizes_and_names_before_decompression(self):
        packaging.package(self.root, self.zip)
        with zipfile.ZipFile(self.zip) as archive:
            baseline = {name: archive.read(name) for name in archive.namelist()}
        cases = (
            ('larger-source', 'src/value.cpp', b'X' * (128 * 1024),
             'Package member size differs from verified source: src/value.cpp'),
            ('unknown-source', 'src/foreign.cpp', b'X' * (128 * 1024),
             'Unexpected package member: src/foreign.cpp'),
            ('larger-manifest', inventory.MANIFEST,
             baseline[inventory.MANIFEST] + b' ',
             'Package/source manifest identity mismatch'),
        )
        for case, member, payload, expected_error in cases:
            with self.subTest(case=case):
                modified = dict(baseline)
                modified[member] = payload
                archive_path = self.base / (case + '.zip')
                with zipfile.ZipFile(archive_path, 'w') as archive:
                    for name, data in sorted(modified.items()):
                        info = zipfile.ZipInfo(name)
                        info.create_system = 3
                        info.external_attr = 0o100644 << 16
                        info.compress_type = zipfile.ZIP_DEFLATED
                        archive.writestr(info, data)
                with patch.object(zipfile.ZipFile, 'read',
                                  side_effect=AssertionError('read before preflight')), \
                     patch.object(zipfile.ZipFile, 'testzip',
                                  side_effect=AssertionError('CRC before preflight')):
                    errors = falcion.check(self.root, archive_path)
                self.assertIn(expected_error, errors)
        self.assertEqual([], falcion.check(self.root, self.zip))

    def test_falcion_same_size_modified_source_still_fails_hash_validation(self):
        packaging.package(self.root, self.zip)
        with zipfile.ZipFile(self.zip) as archive:
            modified = {name: archive.read(name) for name in archive.namelist()}
        original = modified['src/value.cpp']
        modified['src/value.cpp'] = bytes([original[0] ^ 1]) + original[1:]
        archive_path = self.base / 'same-size-mutant.zip'
        with zipfile.ZipFile(archive_path, 'w') as archive:
            for name, data in sorted(modified.items()):
                info = zipfile.ZipInfo(name)
                info.create_system = 3
                info.external_attr = 0o100644 << 16
                info.compress_type = zipfile.ZIP_DEFLATED
                archive.writestr(info, data)
        errors = falcion.check(self.root, archive_path)
        self.assertIn('Package differs from verified source: src/value.cpp', errors)

    def test_package_tamper_and_private_manifest_are_rejected(self):
        packaging.package(self.root, self.zip)
        with zipfile.ZipFile(self.zip) as archive:
            members = {n: archive.read(n) for n in archive.namelist()}
        members['src/value.cpp'] = b'tampered'
        manifest = json.loads(members[inventory.MANIFEST])
        manifest['files']['src/value.cpp'] = inventory.digest(members['src/value.cpp'])
        members[inventory.MANIFEST] = json.dumps(manifest).encode()
        with zipfile.ZipFile(self.zip, 'w') as archive:
            for name, data in members.items():
                archive.writestr(name, data)
        errors = falcion.check(self.root, self.zip)
        self.assertIn('Package/source manifest identity mismatch', errors)
        self.assertTrue(any('src/value.cpp' in e for e in errors))

    def test_unsafe_archive_members(self):
        for name in ['../escape', '/absolute', 'C:/drive', 'dir\\ambiguous', 'a//b', './a', 'a/../b']:
            with self.subTest(name=name), self.assertRaises(ValueError):
                inventory.safe_member(name)


    def test_windows_unsafe_component_names(self):
        invalid = [
            'dir/file.', 'dir/file ', 'dir/file:stream', 'dir/CON.txt',
            'dir/con', 'dir/NUL.cfg', 'dir/COM9.log', 'dir/LPT3',
            'dir/COM\u00b9.log', 'dir/LPT\u00b3.tmp', 'dir/CONIN$',
            'dir/file<bad>.txt', 'dir/wild?.txt', 'dir/wild*.txt',
            'dir/double"quote', 'dir/pipe|name',
        ]
        for name in invalid:
            with self.subTest(member=name), self.assertRaisesRegex(ValueError, 'Unsafe Windows package path'):
                inventory.safe_member(name)
        valid = [
            'docs/readme.md', 'src/value.cpp', 'dir/.gitignore',
            'dir/COM10.txt', 'dir/auxiliary.txt', 'dir/file..name',
            'dir/normal name.txt', 'dir/non-ascii-\u00e7.txt',
        ]
        for name in valid:
            with self.subTest(member=name):
                self.assertEqual(name, inventory.safe_member(name))

    def test_manifest_consistent_windows_invalid_zip_is_not_published(self):
        packaging.package(self.root, self.zip)
        with zipfile.ZipFile(self.zip) as archive:
            original = {name: archive.read(name) for name in archive.namelist()}
        for index, unsafe in enumerate([
                'docs/audit_portability/file:stream', 'docs/CON.txt', 'docs/trailing.']):
            members = dict(original)
            payload = b'cross-platform filename regression'
            manifest = json.loads(members[inventory.MANIFEST])
            manifest['files'][unsafe] = inventory.digest(payload)
            members[unsafe] = payload
            members[inventory.MANIFEST] = (json.dumps(manifest, sort_keys=True) + '\n').encode('utf-8')
            mutant = self.base / f'windows-name-mutant-{index}.zip'
            with zipfile.ZipFile(mutant, 'w') as archive:
                for name, data in sorted(members.items()):
                    info = zipfile.ZipInfo(name)
                    info.create_system = 3
                    info.external_attr = 0o100644 << 16
                    info.compress_type = zipfile.ZIP_DEFLATED
                    archive.writestr(info, data)
            output = self.base / f'extracted-{index}'
            with self.subTest(member=unsafe), self.assertRaisesRegex(ValueError, 'Unsafe Windows package path'):
                extraction.extract(mutant, output)
            self.assertFalse(output.exists())
        self.assertFalse(any(p.name.startswith('.surge-extract-') for p in self.base.iterdir()))

    def test_casefold_collision(self):
        (self.root / 'src/VALUE.cpp').write_text('collision')
        if len(list((self.root / 'src').glob('*'))) < 2:
            self.skipTest('Host filesystem already rejects case-fold coexistence')
        with self.assertRaises(ValueError):
            inventory.source_files(self.root)

    def test_source_symlink(self):
        link = self.root / 'src/alias.cpp'
        try:
            link.symlink_to(self.root / 'src/value.cpp')
        except (OSError, NotImplementedError):
            self.skipTest('Host cannot create test symlinks')
        with self.assertRaises(ValueError):
            inventory.source_files(self.root)

    def test_no_product_zip_inside_source(self):
        with self.assertRaises(ValueError):
            packaging.package(self.root, self.root / 'bundle.zip')

    def test_extraction_rejects_symlinked_parent_before_staging(self):
        packaging.package(self.root, self.zip)
        external = self.base / 'external-extraction'
        external.mkdir()
        alias = self.base / 'linked-extraction-parent'
        try:
            alias.symlink_to(external, target_is_directory=True)
        except (OSError, NotImplementedError):
            self.skipTest('Directory symlinks unavailable on this host')
        for destination in (alias / 'extracted', alias / 'new-parent' / 'extracted'):
            with self.subTest(destination=destination):
                with self.assertRaisesRegex(ValueError, 'symlink or junction'):
                    extraction.extract(self.zip, destination)
                self.assertFalse(destination.exists())
                self.assertEqual([], list(external.iterdir()))
        ordinary = self.base / 'ordinary-extract' / 'extracted'
        receipt = extraction.extract(self.zip, ordinary)
        self.assertEqual('PASS', receipt['result'])
        self.assertEqual([], inventory.verify_manifest(ordinary))

    def test_output_directory_alias_to_source_root_is_rejected_before_staging(self):
        alias = self.base / 'source-root-alias'
        try:
            alias.symlink_to(self.root, target_is_directory=True)
        except (OSError, NotImplementedError):
            self.skipTest('Directory symlinks unavailable on this host')
        destination = alias / 'source.zip'
        before = (self.root / inventory.MANIFEST).read_bytes()
        # The old guard reaches mkstemp through the alias before FALCION
        # discovers the unrecorded source-tree partial. Reject earlier.
        with patch.object(packaging.tempfile, 'mkstemp',
                          side_effect=AssertionError('staging reached source root')):
            with self.assertRaisesRegex(ValueError, 'Write the source ZIP outside'):
                packaging.package(self.root, destination)
        self.assertEqual(before, (self.root / inventory.MANIFEST).read_bytes())
        self.assertFalse((self.root / 'source.zip').exists())
        self.assertFalse((self.root / 'source.zip.sha256').exists())
        self.assertFalse(list(self.root.glob('.source.zip.*')))

    def test_output_directory_alias_to_external_tree_is_supported(self):
        external = self.base / 'external-output'
        external.mkdir()
        alias = self.base / 'external-output-alias'
        try:
            alias.symlink_to(external, target_is_directory=True)
        except (OSError, NotImplementedError):
            self.skipTest('Directory symlinks unavailable on this host')
        receipt = packaging.package(self.root, alias / 'source.zip')
        self.assertEqual(receipt['sha256'], inventory.digest((external / 'source.zip').read_bytes()))
        self.assertEqual(receipt['sha256'] + '  source.zip\n',
                         (external / 'source.zip.sha256').read_text(encoding='utf-8'))

    def test_duplicate_package_member(self):
        packaging.package(self.root, self.zip)
        with zipfile.ZipFile(self.zip, 'a') as archive:
            import warnings
            with warnings.catch_warnings():
                warnings.simplefilter('ignore', UserWarning)
                archive.writestr('src/value.cpp', 'duplicate')
        self.assertTrue(any('Duplicate' in e for e in falcion.check(self.root, self.zip)))


class SourceQaReportTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        # Windows temp paths may use an 8.3 alias; match the resolved ROOT
        # used by the real QA entrypoint before comparing output ancestry.
        self.root = Path(self.temp.name).resolve()
        (self.root / 'src').mkdir()
        (self.root / 'build').mkdir()
        self.source = self.root / 'src/sentinel.cpp'
        self.source.write_text('owned source sentinel\n')
        self.marker = self.root / 'checks-ran'
        # One actual disposable child process; never recursively run project QA.
        (self.root / 'check.py').write_text(
            'from pathlib import Path\n'
            'Path(__file__).with_name("checks-ran").write_text("ran\\n")\n')

    def run_fixture(self, report, checks=None):
        if checks is None:
            checks = [('fixture check', ['check.py'])]
        with patch.object(source_qa, 'ROOT', self.root), \
             patch.object(source_qa, 'CHECKS', checks), \
             patch.object(sys, 'argv', ['run_source_qa.py', '--report', str(report)]), \
             contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            return source_qa.main()

    def test_hardlinked_report_preserves_source_inode(self):
        report = self.root / 'build/report.json'
        os.link(self.source, report)
        before = self.source.read_bytes()
        self.assertEqual(0, self.run_fixture(report))
        self.assertEqual(before, self.source.read_bytes())
        self.assertFalse(os.path.samefile(self.source, report))
        self.assertEqual('PASS_SOURCE_QA', json.loads(report.read_text())['result'])
        self.assertEqual([report], list(report.parent.iterdir()))

    def test_invalid_report_is_rejected_before_checks(self):
        directory = self.root / 'build/directory'
        directory.mkdir()
        blocker = self.root / 'build/not-a-directory'
        blocker.write_text('preserved parent file\n')
        for report in (self.source, directory, blocker / 'report.json'):
            with self.subTest(report=report), self.assertRaises(SystemExit) as error:
                self.run_fixture(report)
            self.assertEqual(2, error.exception.code)
            self.assertFalse(self.marker.exists())
            self.assertEqual('owned source sentinel\n', self.source.read_text())
        self.assertEqual([], list(directory.iterdir()))
        self.assertEqual('preserved parent file\n', blocker.read_text())

    def test_missing_parents_receive_complete_report(self):
        report = self.root / 'build/qa/nested/report.json'
        self.assertEqual(0, self.run_fixture(report))
        data = json.loads(report.read_text())
        self.assertEqual('surge.source-qa.v1', data['schema'])
        self.assertEqual('NOT_RUN', data['native_game_acceptance'])
        self.assertEqual(0, data['checks'][0]['exit_code'])
        self.assertTrue(self.marker.exists())
        self.assertEqual([report], list(report.parent.iterdir()))

    def test_replace_failure_preserves_previous_report(self):
        report = self.root / 'build/report.json'
        report.write_bytes(b'previous complete report\n')
        with patch.object(os, 'replace', side_effect=PermissionError('fixture replace failure')):
            with self.assertRaises(PermissionError):
                self.run_fixture(report)
        self.assertEqual(b'previous complete report\n', report.read_bytes())
        self.assertTrue(self.marker.exists())
        self.assertEqual([report], list(report.parent.iterdir()))

    def test_partial_write_failure_preserves_previous_report(self):
        report = self.root / 'build/report.json'
        report.write_bytes(b'previous complete report\n')
        def fail_write(value, output, **kwargs):
            output.write('{')
            raise OSError('fixture partial write failure')
        with patch.object(source_qa.json, 'dump', side_effect=fail_write):
            with self.assertRaises(OSError):
                self.run_fixture(report)
        self.assertEqual(b'previous complete report\n', report.read_bytes())
        self.assertTrue(self.marker.exists())
        self.assertEqual([report], list(report.parent.iterdir()))

    def test_failed_check_retains_nonzero_result_and_report(self):
        with (self.root / 'check.py').open('a') as output:
            output.write('raise SystemExit(7)\n')
        report = self.root / 'build/report.json'
        self.assertEqual(1, self.run_fixture(report))
        data = json.loads(report.read_text())
        self.assertEqual('FAIL', data['result'])
        self.assertEqual(7, data['checks'][0]['exit_code'])
        self.assertEqual([report], list(report.parent.iterdir()))


    def test_timeout_records_partial_diagnostics_without_claiming_pass(self):
        report = self.root / 'build/report.json'
        timeout = subprocess.TimeoutExpired(
            ['python', 'check.py'], 90,
            output=b'partial stdout: \xff\n', stderr=b'partial stderr\n')
        with patch.object(source_qa.subprocess, 'run', side_effect=timeout):
            self.assertEqual(1, self.run_fixture(report))
        data = json.loads(report.read_text())
        self.assertEqual('FAIL', data['result'])
        self.assertEqual(124, data['checks'][0]['exit_code'])
        self.assertIn('QA timeout', data['checks'][0]['output'])
        self.assertIn('partial stdout: \ufffd\n', data['checks'][0]['output'])
        self.assertIn('partial stderr\n', data['checks'][0]['output'])
        self.assertFalse(self.marker.exists())

    def test_timeout_keeps_later_results_in_complete_report(self):
        report = self.root / 'build/report.json'
        timeout = subprocess.TimeoutExpired(['python', 'check.py'], 90,
                                             output=b'early diagnostic\n')
        after = subprocess.CompletedProcess(['python', 'check.py'], 0, 'later success\n', '')
        checks = [('timed out', ['check.py']), ('after timeout', ['check.py'])]
        with patch.object(source_qa.subprocess, 'run', side_effect=[timeout, after]):
            self.assertEqual(1, self.run_fixture(report, checks))
        data = json.loads(report.read_text())
        self.assertEqual('FAIL', data['result'])
        self.assertEqual(['timed out', 'after timeout'], [x['check'] for x in data['checks']])
        self.assertEqual([124, 0], [x['exit_code'] for x in data['checks']])
        self.assertIn('early diagnostic', data['checks'][0]['output'])
        self.assertEqual('later success\n', data['checks'][1]['output'])
        self.assertFalse(self.marker.exists())

    def test_unavailable_child_is_recorded_as_failed_check(self):
        report = self.root / 'build/report.json'
        with patch.object(source_qa.subprocess, 'run',
                          side_effect=FileNotFoundError('fixture Python executable missing')):
            self.assertEqual(1, self.run_fixture(report))
        data = json.loads(report.read_text())
        self.assertEqual('FAIL', data['result'])
        self.assertEqual(127, data['checks'][0]['exit_code'])
        self.assertIn('fixture Python executable missing', data['checks'][0]['output'])
        self.assertFalse(self.marker.exists())


class BranchPushGateTests(unittest.TestCase):
    def test_real_pushes_preserve_gates_for_temporary_audits_and_research(self):
        # All refs, including main, belong to this disposable local fixture.
        # The QA stub records invocation; it does not impersonate project QA.
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            checkout, remote = base / 'checkout', base / 'remote.git'
            checkout.mkdir()
            # A real pre-push hook exports repository-local Git variables.
            # Do not let them redirect fixture commands into the source repo.
            environment = os.environ.copy()
            local_names = subprocess.run(['git', 'rev-parse', '--local-env-vars'],
                                         cwd=ROOT, capture_output=True, text=True,
                                         check=True, timeout=30).stdout.splitlines()
            for name in local_names:
                environment.pop(name, None)

            def git(*args, accept=True, cwd=checkout):
                result = subprocess.run(['git', '-C', str(cwd), *args],
                                        capture_output=True, text=True, timeout=30,
                                        env=environment)
                if accept:
                    self.assertEqual(0, result.returncode, result.stdout + result.stderr)
                return result

            git('init', '--initial-branch=fix/fixture')
            git('config', 'user.name', 'Push gate fixture')
            git('config', 'user.email', 'push-gate@example.invalid')
            git('config', 'core.autocrlf', 'false')
            git('config', 'commit.gpgsign', 'false')
            self.assertEqual(checkout.resolve(), Path(git('rev-parse', '--show-toplevel').stdout.strip()).resolve())
            hooks = checkout / '.github/hooks'
            hooks.mkdir(parents=True)
            hook = hooks / 'pre-push'
            hook.write_bytes((ROOT / '.github/hooks/pre-push').read_bytes())
            hook.chmod(hook.stat().st_mode | 0o111)
            (checkout / '.gitignore').write_text('build/\n')
            (checkout / 'scripts').mkdir()
            (checkout / 'scripts/run_source_qa.py').write_text(
                'from pathlib import Path\n'
                'p = Path("build/qa-ran")\n'
                'p.parent.mkdir(parents=True, exist_ok=True)\n'
                'p.write_text("fixture QA invoked")\n')
            git('add', '.')
            git('update-index', '--chmod=+x', '.github/hooks/pre-push')
            git('commit', '-m', 'Seed disposable gate fixture')
            seed = git('rev-parse', 'HEAD').stdout.strip()
            git('init', '--bare', str(remote))
            git('remote', 'add', 'origin', str(remote))
            git('push', 'origin', 'HEAD:refs/heads/seed')
            for name in ['main', 'bugfix', 'fix/done']:
                git('update-ref', 'refs/heads/' + name, seed, cwd=remote)
            git('config', 'core.hooksPath', hooks.as_posix())
            (checkout / 'change.txt').write_text('reviewed fixture change\n')
            git('add', 'change.txt')
            git('commit', '-m', 'Prepare next fixture commit')
            head = git('rev-parse', 'HEAD').stdout.strip()
            marker = checkout / 'build/qa-ran'

            for name in ['bugfix', 'fix/done']:
                with self.subTest(deletion=name):
                    git('push', 'origin', '--delete', name)
                    self.assertNotEqual(0, git('show-ref', '--verify', 'refs/heads/' + name,
                                              cwd=remote, accept=False).returncode)
                    self.assertFalse(marker.exists(), 'pure deletion must not run QA')

            for refspec, message in [
                    ('HEAD:refs/heads/bugfix', 'shared bugfix branch is retired'),
                    ('HEAD:refs/heads/main', 'main is updated only by owner-reviewed PR merges'),
                    (':refs/heads/main', 'main is updated only by owner-reviewed PR merges'),
                    (seed + ':refs/heads/fix/old-tip', 'check out the commit being pushed')]:
                with self.subTest(rejection=refspec):
                    rejected = git('push', 'origin', refspec, accept=False)
                    self.assertNotEqual(0, rejected.returncode)
                    self.assertIn(message, rejected.stderr)
                    self.assertFalse(marker.exists(), 'rejection must happen before QA')
            self.assertEqual(seed, git('rev-parse', 'refs/heads/main', cwd=remote).stdout.strip())

            git('push', 'origin', 'HEAD:refs/heads/fix/new-task')
            self.assertTrue(marker.exists(), 'fresh PR branch must run the QA gate')
            self.assertEqual(head, git('rev-parse', 'refs/heads/fix/new-task', cwd=remote).stdout.strip())
            # The push hook checks source integrity, not the branch's retention
            # designation: a routine audit is still a temporary PR head.
            for name in ('audit/one-shot-finding', 'audit/owner-approved-campaign',
                         'research/reconstruction-package'):
                with self.subTest(project_branch=name):
                    marker.unlink()
                    git('push', 'origin', 'HEAD:refs/heads/' + name)
                    self.assertTrue(marker.exists(), 'every PR head must pass the same QA gate')
                    self.assertEqual(head, git('rev-parse', 'refs/heads/' + name,
                                               cwd=remote).stdout.strip())
            marker.unlink()
            git('push', 'origin', '--delete', 'audit/one-shot-finding')
            self.assertNotEqual(0, git('show-ref', '--verify',
                                      'refs/heads/audit/one-shot-finding',
                                      cwd=remote, accept=False).returncode)
            for retained in ('audit/owner-approved-campaign',
                             'research/reconstruction-package'):
                self.assertEqual(head, git('rev-parse', 'refs/heads/' + retained,
                                           cwd=remote).stdout.strip())
            self.assertFalse(marker.exists(), 'deleting a temporary audit head must not run QA')
            (checkout / 'change.txt').write_text('uncommitted fixture change\n')
            rejected = git('push', 'origin', 'HEAD:refs/heads/fix/dirty', accept=False)
            self.assertNotEqual(0, rejected.returncode)
            self.assertIn('commit the reviewed changes', rejected.stderr)
            self.assertFalse(marker.exists())
            self.assertNotEqual(0, git('show-ref', '--verify', 'refs/heads/fix/dirty',
                                      cwd=remote, accept=False).returncode)


class GitFixtureIsolationTests(unittest.TestCase):
    def test_root_fixtures_preserve_an_inherited_callers_repository(self):
        import shutil
        if not shutil.which('git'):
            self.skipTest('Git executable not available')
        with tempfile.TemporaryDirectory() as tmp:
            observer = Path(tmp) / 'observer'
            observer.mkdir()
            # Independent setup: every poisoned selector below points only to
            # this disposable observer, never to the real source checkout.
            environment = {name: value for name, value in os.environ.items()
                           if not name.startswith('GIT_')}
            self.assertFalse(any(name.startswith('GIT_') for name in environment))
            def git(*args):
                result = subprocess.run(['git', '-C', str(observer), *args],
                                        env=environment, capture_output=True,
                                        text=True, timeout=30)
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)
                return result.stdout
            git('init', '--initial-branch=fixture-owner')
            self.assertEqual(observer.resolve(), Path(git('rev-parse', '--show-toplevel').strip()).resolve())
            git('config', 'user.name', 'Observer fixture')
            git('config', 'user.email', 'observer@example.invalid')
            git('config', 'commit.gpgsign', 'false')
            sentinel = observer / 'observer.txt'
            sentinel.write_bytes(b'owned observer sentinel\n')
            git('add', 'observer.txt')
            git('commit', '-m', 'Seed disposable observer')
            metadata = observer / '.git'
            def snapshot():
                return {
                    'head': git('rev-parse', 'HEAD'),
                    'refs': git('show-ref'),
                    'worktrees': git('worktree', 'list', '--porcelain'),
                    'config': (metadata / 'config').read_bytes(),
                    'index': (metadata / 'index').read_bytes(),
                    'objects': {p.relative_to(metadata).as_posix(): p.read_bytes()
                                for p in (metadata / 'objects').rglob('*') if p.is_file()},
                }
            before = snapshot()
            poisoned = dict(environment, GIT_DIR=str(metadata),
                            GIT_WORK_TREE=str(observer), GIT_COMMON_DIR=str(metadata),
                            GIT_INDEX_FILE=str(metadata / 'index'),
                            GIT_OBJECT_DIRECTORY=str(metadata / 'objects'))
            result = subprocess.run(
                [sys.executable, '-B', str(ROOT / 'tests/infrastructure/test_root_ownership.py'),
                 'RepositoryRootOwnership.test_real_git_worktree_gitfile_is_allowed',
                 'RepositoryRootOwnership.test_borrowed_valid_worktree_redirect_does_not_claim_another_root'],
                cwd=ROOT, env=poisoned, capture_output=True, text=True, timeout=45)
            # Check preservation before checking the child result: even a
            # rejected fixture run must never mutate the caller's repository.
            self.assertEqual(before, snapshot())
            self.assertEqual(b'owned observer sentinel\n', sentinel.read_bytes())
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            self.assertIn('Ran 2 tests', result.stderr)
            self.assertNotIn('skipped', result.stderr)


class RuntimeBenchmarkTests(unittest.TestCase):
    @staticmethod
    def diagnostic():
        import importlib.util
        path = ROOT / 'tests/diagnostics/runtime_load/run.py'
        spec = importlib.util.spec_from_file_location('runtime_load_diagnostic', path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return module

    @staticmethod
    def row(workers=2, workload='echo', **changes):
        row = dict(workers=workers, clients=1, requests_per_client=2, pipeline=1,
                   payload_bytes=1024, workload=workload, completed=2, errors=0,
                   elapsed_ns=100000 // max(1, workers), cpu_ns=10000, cpu_cycles=1000,
                   startup_ns=1000, shutdown_ns=1000,
                   peak_working_set_bytes=4096, private_bytes=2048,
                   profile='pool-' + str(workers) if workers else 'thread-per-message',
                   created_message_threads=0 if workers else 2,
                   observed_worker_threads=min(workers or 2, 2), peak_active_handlers=1,
                   rtt_ns=[2000, 3000], queue_ns=[10, 20], handler_ns=[100, 200])
        if workload.startswith('app_'):
            row.update(payload_bytes=0, application_routed=True, verified_responses=2,
                       handler_lock_calls=2, lock_wait_ns=[10, 20], lock_hold_ns=[30, 40], lock_calls=[1, 1],
                       persistence_verified=workload == 'app_achievement_encrypted')
        row.update(changes)
        return row

    def test_generated_candidates_retain_every_other_production_byte(self):
        tool = self.diagnostic()
        original = (ROOT / 'src/net/asio_service.cpp').read_text()
        for workers in tool.WORKERS:
            generated = tool.generated_service(original, workers)
            restored = generated.removeprefix('#include "probe.hpp"\n').replace(
                'jobs.push_back(citrine_eclipse_mirage::benchmark::timed_job(std::move(fn)));', tool.PUSH)
            restored = restored.replace('\n    citrine_eclipse_mirage::benchmark::PerMessageJobs message_jobs;', '')
            for needle, insertion in tool.CONTROL_INSERTIONS:
                restored = restored.replace(needle + insertion, needle)
            if workers not in (0, 2):
                restored = restored.replace(tool.MARKER.replace('handler_policy::workers', str(workers)), tool.MARKER)
            self.assertEqual(original, restored)
        for source, count in [(original, 3), (original.replace(tool.PUSH, ''), 2),
                              (original + tool.MARKER, 2), ('#include "probe.hpp"\n' + original, 2),
                              (original.replace(tool.THREAD_ALIAS, ''), 2),
                              (original.replace('OwnedThread=std::thread;', 'OwnedThread=std::jthread;'), 2)]:
            with self.subTest(workers=count), self.assertRaises(ValueError):
                tool.generated_service(source, count)

    def test_application_copy_preserves_dispatch_and_lock_ownership(self):
        tool = self.diagnostic()
        original = (ROOT / 'src/runtime/application.cpp').read_text()
        generated = tool.generated_application(original)
        restored = generated.removeprefix('#include "application_probe.hpp"\n')
        for before, after in tool.APPLICATION_INSERTIONS:
            restored = restored.replace(after, before)
        self.assertEqual(original, restored)
        for bad in (original.replace(tool.APPLICATION_INSERTIONS[0][0], ''),
                    original + tool.APPLICATION_INSERTIONS[1][0], generated):
            with self.assertRaises(ValueError):
                tool.generated_application(bad)

    def test_generated_application_identity_is_bound_to_the_source_and_disk_copy(self):
        tool = self.diagnostic()
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            (build / 'generated').mkdir()
            source = (ROOT / 'src/net/asio_service.cpp').read_text()
            hashes = {}
            for workers in tool.WORKERS:
                path = build / 'generated' / f'asio_{workers}.cpp'
                path.write_bytes(tool.generated_service(source, workers).encode())
                hashes[str(workers)] = tool.digest(path)
            path = build / 'generated/application.cpp'
            path.write_bytes(tool.generated_application((ROOT / 'src/runtime/application.cpp').read_text()).encode())
            inputs = dict(generated_service_sha256=hashes, generated_application_sha256=tool.digest(path))
            tool.verify_generated(build, inputs)
            path.write_bytes(path.read_bytes() + b'\n// unreviewed copy\n')
            with self.assertRaises(ValueError):
                tool.verify_generated(build, inputs)
            inputs['generated_application_sha256'] = tool.digest(path)
            with self.assertRaises(ValueError):
                tool.verify_generated(build, inputs)

    def test_generated_set_rejects_aliases_before_any_output_changes(self):
        tool = self.diagnostic()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, first, last = root / 'source.cpp', root / 'asio_1.cpp', root / 'asio_2.cpp'
            source.write_bytes(b'protected source')
            first.write_bytes(b'old first copy')
            os.link(source, last)
            with self.assertRaises(ValueError):
                tool.write_generated({first: b'new first copy', last: b'new second copy'})
            self.assertEqual(b'protected source', source.read_bytes())
            self.assertEqual(b'old first copy', first.read_bytes())
            self.assertFalse(list(root.glob('*.partial')))
            last.unlink()
            tool.write_generated({first: b'new first copy', last: b'new second copy'})
            self.assertEqual(b'new first copy', first.read_bytes())
            self.assertEqual(b'new second copy', last.read_bytes())
            self.assertEqual(b'protected source', source.read_bytes())

    def test_bad_or_incomplete_measurements_cannot_pass(self):
        tool = self.diagnostic()
        good = self.row()
        fields = ('workers', 'clients', 'requests_per_client', 'pipeline', 'payload_bytes', 'workload')
        expected = {key: good[key] for key in fields}
        self.assertIs(good, tool.validate(good, expected))
        for change in [dict(errors=1), dict(errors=False), dict(completed=1),
                       dict(rtt_ns=[1]), dict(queue_ns=[float('nan'), 1]),
                       dict(handler_ns=[-1, 0]), dict(elapsed_ns=0), dict(workers=4),
                       dict(observed_worker_threads=3), dict(peak_active_handlers=2),
                       dict(rtt_ns=[1, 1000000])]:
            with self.subTest(change=change), self.assertRaises(ValueError):
                tool.validate({**good, **change}, expected)

    def test_matrix_requires_every_candidate_round_and_unique_execution_order(self):
        tool = self.diagnostic()
        rows = [self.row(n, w, repeat=0, order=i)
                for w in tool.WORKLOADS for i, n in enumerate(tool.WORKERS)]
        summary = tool.summarize(rows, 1, (1,), (1,))
        self.assertEqual(40, len(summary))
        for row in summary:
            self.assertEqual(max(1, row['workers']) / 2, row['throughput_ratio_to_2_workers'])
            self.assertIsNone(row['throughput_ratio_95pct_bootstrap'])
        for bad in (rows[:-1], rows + [rows[0]],
                    [{**row, 'order': 0} for row in rows]):
            with self.assertRaises(ValueError):
                tool.summarize(bad, 1, (1,), (1,))
        components = [r for r in rows if r['workload'] in tool.COMPONENT_WORKLOADS]
        with self.assertRaises(ValueError):
            tool.summarize(components, 1, (1,), (1,))
        self.assertEqual(20, len(tool.summarize(components, 1, (1,), (1,), tool.COMPONENT_WORKLOADS)))

    def test_application_measurements_require_responses_lock_samples_and_persistence(self):
        tool = self.diagnostic()
        good = self.row(workload='app_achievement_encrypted')
        expected = {key: good[key] for key in
                    ('workers', 'clients', 'requests_per_client', 'pipeline', 'payload_bytes', 'workload')}
        self.assertIs(good, tool.validate(good, expected))
        for change in (dict(application_routed=False), dict(verified_responses=1),
                       dict(handler_lock_calls=True), dict(lock_wait_ns=[1]),
                       dict(lock_calls=[0, 2]), dict(lock_calls=[True, 1]),
                       dict(lock_hold_ns=[-1, 1]), dict(lock_hold_ns=[100, 200]),
                       dict(persistence_verified=False)):
            with self.subTest(change=change), self.assertRaises(ValueError):
                tool.validate({**good, **change}, expected)

    def test_percentiles_and_paired_intervals_have_known_answers(self):
        tool = self.diagnostic()
        self.assertEqual(95, tool.percentile(list(range(1, 101)), .95))
        self.assertEqual([2.0, 2.0], tool.ratio_interval([2.0] * 5, 1))
        for bad in ([], [float('nan')], [float('inf')], [-1], [0]):
            with self.assertRaises(ValueError):
                tool.ratio_interval(bad, 1)

    def test_failed_child_and_ineffective_corruption_control_are_rejected(self):
        tool = self.diagnostic()
        row = self.row()
        expected = {key: row[key] for key in
                    ('workers', 'clients', 'requests_per_client', 'pipeline', 'payload_bytes', 'workload')}
        with tempfile.TemporaryDirectory() as directory:
            bad = subprocess.CompletedProcess([], 1, '', 'fixture failure')
            with patch.object(tool.subprocess, 'run', return_value=bad), self.assertRaises(ValueError):
                tool.execute(Path('not-executed'), expected, directory)
            good = subprocess.CompletedProcess([], 0, json.dumps(row), '')
            with patch.object(tool.subprocess, 'run', return_value=good), self.assertRaises(ValueError):
                tool.execute(Path('not-executed'), expected, directory, fault=True)

    def test_application_controls_require_the_specific_response_or_persistence_failure(self):
        tool = self.diagnostic()
        row = self.row(workload='app_achievement_encrypted')
        expected = {key: row[key] for key in
                    ('workers', 'clients', 'requests_per_client', 'pipeline', 'payload_bytes', 'workload')}
        with tempfile.TemporaryDirectory() as directory:
            for fault, reason in ((True, 'application response validation failed'),
                                  ('corrupt-persistence', 'Application persistence validation failed'),
                                  ('corrupt-ini', 'Application persistence validation failed')):
                bad = subprocess.CompletedProcess([], 1, '', reason)
                with patch.object(tool.subprocess, 'run', return_value=bad) as child:
                    tool.execute(Path('not-executed'), expected, directory, fault=fault)
                    self.assertEqual('corrupt-response' if fault is True else fault,
                                     child.call_args.kwargs['env']['RUNTIME_BENCHMARK_FAULT'])
                with patch.object(tool.subprocess, 'run', return_value=subprocess.CompletedProcess([], 1, '', 'unrelated startup failure')):
                    with self.assertRaises(ValueError):
                        tool.execute(Path('not-executed'), expected, directory, fault=fault)

    def test_failure_retains_partial_measurements_without_success_receipt(self):
        tool = self.diagnostic()
        with tempfile.TemporaryDirectory() as directory:
            # Match run.py's resolved ROOT/output identity, including Windows
            # temporary directories exposed through an 8.3 path or junction.
            root = Path(directory).resolve()
            build = root / 'build/binaries'
            (build / 'generated').mkdir(parents=True)
            (build / 'Release').mkdir()
            (build / 'generated/inputs.json').write_text('{}')
            for n in tool.WORKERS:
                (build / 'Release' / ('runtime_load_' + str(n) + '.exe')).write_bytes(b'not executed')
            calls = []
            def execute(binary, params, output, fault=False):
                if fault:
                    return
                calls.append(params)
                if len(calls) > 1:
                    raise ValueError('injected child failure')
                count = params['clients'] * params['requests_per_client']
                return {**self.row(params['workers']), **params, 'completed': count,
                        'created_message_threads': count if not params['workers'] else 0,
                        'rtt_ns': [2000] * count, 'queue_ns': [10] * count, 'handler_ns': [100] * count}
            output = root / 'build/results'
            with patch.object(tool, 'ROOT', root), patch.object(tool, 'verify_source'), \
                 patch.object(tool, 'identity', return_value={}), \
                 patch.object(tool, 'verify_generated'), \
                 patch.object(tool, 'verified_builds', return_value={}), \
                 patch.object(tool.platform, 'system', return_value='Windows'), \
                 patch.object(tool, 'execute', side_effect=execute):
                with self.assertRaisesRegex(ValueError, 'injected child failure'):
                    tool.run(build, output, quick=True, suite='component')
            receipt = json.loads((output / 'results.json').read_text())
            self.assertEqual('FAIL', receipt['result'])
            self.assertEqual(1, len(receipt['rows']))
            self.assertNotIn('summary', receipt)


class RuntimeEvidenceTests(unittest.TestCase):
    @staticmethod
    def auditor():
        import importlib.util
        spec = importlib.util.spec_from_file_location('runtime_evidence_audit', ROOT / 'tests/diagnostics/runtime_load/audit.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return module

    def fixture(self):
        audit = self.auditor()
        tool = audit.benchmark
        identity = dict(commit='a' * 40, manifest_sha256='b' * 64, vendor_lock_sha256='c' * 64,
                        asio_source_sha256='d' * 64, application_source_sha256='e' * 64)
        hashes = tool.generated_hashes()
        inputs = dict(schema='citrine.runtime-benchmark-inputs.v2', **identity,
                      production_worker_policy=2, production_queue_limit=128,
                      generated_service_sha256={str(n): hashes[f'asio_{n}.cpp'] for n in tool.WORKERS},
                      generated_application_sha256=hashes['application.cpp'])
        files = {'binaries/generated/inputs.json': json.dumps(inputs).encode()}
        data = dict(schema='citrine.runtime-load.v3', result='PASS_CORRECTNESS_WITH_MEASUREMENTS',
                    run_id='1' * 32,
                    evidence_class='WINDOWS_NATIVE_COMPONENT_BENCHMARK', source=inputs, system='Windows',
                    logical_cpus=4, binaries={}, builds={}, repeats=1, suite='all', workloads=list(tool.WORKLOADS),
                    quick=True, performance_thresholds=False, statistical_comparison='SMOKE_ONLY',
                    response_corruption_control='EXPECTED_FAILURE', persistence_corruption_control='EXPECTED_FAILURE_XML_AND_INI', rows=[])
        for kind in ('runtime_load', 'application_load'):
            for n in tool.WORKERS:
                name = f'{kind}_{n}'
                raw = ('synthetic non-executable ' + name).encode()
                sha = hashlib.sha256(raw).hexdigest()
                build = dict(schema='citrine.runtime-benchmark-build.v1', source=inputs,
                             binary=name + '.exe', sha256=sha, kind=kind, workers=n,
                             configuration='Release', compiler_id='MSVC', compiler_version='19.44', cxx_standard=20)
                files[f'binaries/Release/{name}.exe'] = raw
                files[f'binaries/Release/{name}.exe.build.json'] = json.dumps(build).encode()
                data['binaries'][name], data['builds'][name] = sha, build
        rows = []
        for w in tool.WORKLOADS:
            for clients in (1, 8):
                for order, n in enumerate(tool.WORKERS):
                    params = tool.parameters(w, clients, 1, n, True)
                    total = clients * 8
                    row = {**RuntimeBenchmarkTests.row(n, w), **params, 'repeat': 0, 'order': order,
                           'run_id': data['run_id'], 'binary_sha256': data['binaries'][f'{tool.binary_kind(w)}_{n}'],
                           'completed': total, 'created_message_threads': total if n == 0 else 0,
                           'rtt_ns': [2000] * total, 'queue_ns': [10] * total, 'handler_ns': [100] * total}
                    if w in tool.APPLICATION_WORKLOADS:
                        row.update(verified_responses=total, handler_lock_calls=total, lock_calls=[1] * total,
                                   lock_wait_ns=[10] * total, lock_hold_ns=[20] * total)
                    path = f'samples/{len(rows):04d}.json'
                    raw = json.dumps(row).encode()
                    files['results/' + path] = raw
                    data['rows'].append({**{k: v for k, v in row.items() if k not in tool.SAMPLE_FIELDS},
                                         'samples_file': path, 'samples_sha256': hashlib.sha256(raw).hexdigest()})
                    rows.append(row)
        data['summary'] = tool.summarize(rows, 1, (1, 8), (1,))
        output = io.StringIO()
        writer = csv.DictWriter(output, fieldnames=list(data['summary'][0])); writer.writeheader(); writer.writerows(data['summary'])
        files['results/summary.csv'] = output.getvalue().encode()
        files['results/results.json'] = json.dumps(data).encode()
        return audit, identity, data, files

    def check(self, audit, identity, files):
        class Evidence:
            def read(self, name):
                return files[name]
        with patch.object(audit.benchmark, 'identity', return_value=identity):
            return audit.audit(Evidence())

    def test_complete_artifact_recomputes_json_csv_and_links_binaries(self):
        audit, identity, data, files = self.fixture()
        result = self.check(audit, identity, files)
        self.assertEqual('PASS_INTEGRITY_NOT_EXECUTION_ATTESTATION', result['result'])
        self.assertEqual((10, 80, 80), (result['binary_count'], result['measurements'], result['summary_cells']))
        self.assertTrue(result['quick'])

    def test_same_complete_evidence_is_auditable_from_a_directory_or_zip(self):
        audit, identity, data, files = self.fixture()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name, raw in files.items():
                path = root / name; path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(raw)
            archive = root / 'evidence.zip'
            with zipfile.ZipFile(archive, 'w') as zipped:
                for name, raw in files.items(): zipped.writestr(name, raw)
            with patch.object(audit.benchmark, 'identity', return_value=identity):
                direct = audit.audit(audit.DirectoryEvidence(root / 'results', root / 'binaries'))
                zipped = audit.ZipEvidence(archive)
                try: self.assertEqual(direct, audit.audit(zipped))
                finally: zipped.zip.close()

    def test_paired_ratios_and_round_percentiles_have_independent_known_answers(self):
        tool = RuntimeBenchmarkTests.diagnostic()
        base, candidate = (10000, 1000000, 1000000), (100000, 100000, 10000000)
        latencies = ([1000, 2000], [10000, 20000], [100000, 200000])
        rows = [RuntimeBenchmarkTests.row(n, 'echo', repeat=r, order=i,
                                        elapsed_ns=(candidate if n == 4 else base)[r], rtt_ns=latencies[r])
                for r in range(3) for i, n in enumerate(tool.WORKERS)]
        result = next(r for r in tool.summarize(rows, 3, (1,), (1,), ('echo',)) if r['workers'] == 4)
        # Median paired ratios is median(0.1, 10, 0.1), not the quotient
        # of median rates (20,000 / 2,000 == 10 in these scaled inputs).
        self.assertEqual(.1, result['throughput_ratio_to_2_workers'])
        self.assertEqual(20, result['median_round_p99_us'])
        self.assertEqual(2, result['p99_rank_per_round'])
        self.assertEqual(1, result['samples_at_or_above_p99_rank'])
        self.assertIsNone(result['throughput_ratio_95pct_bootstrap'])

    def test_tampered_or_missing_evidence_cannot_inherit_a_pass(self):
        mutations = ('raw_hash', 'binary', 'build_source', 'summary', 'csv', 'missing_row', 'duplicate_sample', 'control', 'source', 'recipe', 'run_identity', 'sample_binary')
        for mutation in mutations:
            with self.subTest(mutation=mutation):
                audit, identity, data, files = self.fixture()
                if mutation == 'raw_hash': files['results/samples/0000.json'] += b' '
                elif mutation == 'binary': files['binaries/Release/runtime_load_2.exe'] += b'changed binary'
                elif mutation == 'build_source':
                    build = audit.benchmark.load_json(files['binaries/Release/runtime_load_2.exe.build.json'])
                    build['configuration'] = 'Debug'; files['binaries/Release/runtime_load_2.exe.build.json'] = json.dumps(build).encode()
                elif mutation == 'summary': data['summary'][0]['median_requests_per_second'] += 1
                elif mutation == 'csv': files['results/summary.csv'] += files['results/summary.csv'].splitlines(keepends=True)[1]
                elif mutation == 'missing_row': data['rows'].pop()
                elif mutation == 'duplicate_sample': data['rows'][1] = data['rows'][0]
                elif mutation == 'control': data['persistence_corruption_control'] = 'EXPECTED_FAILURE'
                elif mutation == 'source': identity = {**identity, 'commit': 'f' * 40}
                elif mutation in ('recipe', 'run_identity', 'sample_binary'):
                    row = audit.benchmark.load_json(files['results/samples/0000.json'])
                    if mutation == 'recipe': row['payload_bytes'] = 1000
                    elif mutation == 'run_identity': row['run_id'] = '2' * 32
                    else: row['binary_sha256'] = 'f' * 64
                    raw = json.dumps(row).encode(); files['results/samples/0000.json'] = raw
                    data['rows'][0] = {**{k: v for k, v in row.items() if k not in audit.benchmark.SAMPLE_FIELDS},
                                      'samples_file': 'samples/0000.json', 'samples_sha256': hashlib.sha256(raw).hexdigest()}
                files['results/results.json'] = json.dumps(data).encode()
                with self.assertRaises(ValueError): self.check(audit, identity, files)

    def test_strict_json_refuses_duplicate_keys_and_nonfinite_values(self):
        tool = RuntimeBenchmarkTests.diagnostic()
        for raw in ('{"result": "FAIL", "result": "PASS"}', '{"a":{"x":1,"x":2}}', '[NaN]', '[Infinity]'):
            with self.subTest(raw=raw), self.assertRaises(ValueError): tool.load_json(raw)

    def test_zip_and_directory_paths_cannot_redirect_evidence(self):
        audit = self.auditor()
        for path in ('../escape', '/absolute', 'a/../b', 'C:/file', 'a\\file', 'a//file'):
            with self.subTest(path=path), self.assertRaises(ValueError): audit.member(path)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for extra in ('../escape', 'RESULTS/results.json'):
                archive = root / 'bad.zip'
                with zipfile.ZipFile(archive, 'w') as z:
                    z.writestr('results/results.json', '{}'); z.writestr(extra, '{}')
                with self.assertRaises(ValueError): audit.ZipEvidence(archive)
            archive = root / 'link.zip'
            with zipfile.ZipFile(archive, 'w') as z:
                z.writestr('results/results.json', '{}')
                entry = zipfile.ZipInfo('results/link'); entry.create_system = 3; entry.external_attr = 0o120777 << 16
                z.writestr(entry, '../outside')
            with self.assertRaises(ValueError): audit.ZipEvidence(archive)

    def test_stale_or_replaced_binary_does_not_match_link_receipt(self):
        audit, identity, data, files = self.fixture()
        tool = audit.benchmark
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            (build / 'Release').mkdir()
            name = 'runtime_load_2'
            binary = build / 'Release' / (name + '.exe')
            binary.write_bytes(files[f'binaries/Release/{name}.exe'])
            Path(str(binary) + '.build.json').write_bytes(files[f'binaries/Release/{name}.exe.build.json'])
            tool.verified_builds({('runtime_load', 2): binary}, data['source'])
            binary.write_bytes(b'stale candidate from another build')
            with self.assertRaises(ValueError): tool.verified_builds({('runtime_load', 2): binary}, data['source'])
            with self.assertRaises(ValueError): tool.validate_build(data['builds'][name], name, data['binaries'][name], {**data['source'], 'commit': 'f' * 40})

    def test_source_archive_does_not_claim_parent_git_identity(self):
        tool = RuntimeBenchmarkTests.diagnostic()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / 'parent/archive'
            (root.parent / '.git').mkdir(parents=True)
            for name in ('metadata/ACTIVE_SOURCE_MANIFEST.json', 'metadata/VENDOR_LOCK.json',
                         'src/net/asio_service.cpp', 'src/runtime/application.cpp'):
                path = root / name; path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(b'fixture')
            with patch.object(tool, 'ROOT', root), patch.object(tool.subprocess, 'run', side_effect=AssertionError('Archive must not query parent Git')):
                self.assertIsNone(tool.identity()['commit'])

    def test_inherited_git_selectors_cannot_replace_the_benchmark_commit(self):
        tool = RuntimeBenchmarkTests.diagnostic()
        environment = os.environ.copy()
        local_names = subprocess.run(['git', 'rev-parse', '--local-env-vars'],
                                     cwd=ROOT, capture_output=True, text=True,
                                     check=True, timeout=30).stdout.splitlines()
        for name in local_names:
            environment.pop(name, None)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            for name in ('metadata/ACTIVE_SOURCE_MANIFEST.json', 'metadata/VENDOR_LOCK.json',
                         'src/net/asio_service.cpp', 'src/runtime/application.cpp'):
                path = root / name; path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(b'fixture')
            for args in (['init', '-q'], ['add', '.'], ['-c', 'user.name=Fixture', '-c', 'user.email=fixture@example.invalid', 'commit', '-qm', 'fixture']):
                subprocess.run(['git', '-C', str(root), *args], capture_output=True, check=True,
                               env=environment, timeout=30)
            expected = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'],
                                               text=True, env=environment, timeout=30).strip()
            with patch.object(tool, 'ROOT', root), patch.dict(os.environ, {'GIT_DIR': str(root / 'absent-git'), 'GIT_WORK_TREE': str(root / 'absent-tree')}):
                self.assertEqual(expected, tool.identity()['commit'])


class DocumentationContractTests(unittest.TestCase):
    @staticmethod
    def catalog():
        text = (ROOT / 'docs/INDEX.md').read_text(encoding='utf-8')
        rows = re.findall(r'^\| \[[^\]]+\]\(([^)]+)\) \| (Current|Checkpoint|Mixed|Review) \|', text, re.M)
        return [((ROOT / 'docs' / name).resolve().relative_to(ROOT).as_posix(), role)
                for name, role in rows]

    def test_catalog_covers_all_first_party_markdown_and_companion_csvs(self):
        manifest = json.loads((ROOT / 'metadata/ACTIVE_SOURCE_MANIFEST.json').read_text())
        expected = {name for name in manifest['files']
                    if name.endswith('.md') and not name.startswith(
                        ('third_party/', 'metadata/evidence/', 'docs/evidence/'))}
        expected |= {name for name in manifest['files']
                     if name.startswith('docs/') and name.endswith('.csv')}
        rows = self.catalog()
        self.assertEqual(len(rows), len({name for name, _ in rows}), 'Duplicate catalog path')
        self.assertEqual(expected, {name for name, _ in rows})
        for name, role in rows:
            with self.subTest(document=name):
                self.assertTrue((ROOT / name).is_file())
                if role == 'Checkpoint' and name.endswith('.md'):
                    self.assertIn('historical', (ROOT / name).read_text(encoding='utf-8')[:1200].lower())

    def test_document_file_links_resolve_without_network_or_git_checkout(self):
        # Checkpoint links must remain navigable too; no external-link request
        # or rewriting of retained evidence is needed for this file-level check.
        from urllib.parse import unquote, urlsplit
        for name, _ in self.catalog():
            if not name.endswith('.md'):
                continue
            path = ROOT / name
            text = path.read_text(encoding='utf-8')
            for destination in re.findall(r'\]\(([^)\n]+)\)', text):
                target = urlsplit(destination.strip('<>'))
                if target.scheme or target.netloc or not target.path:
                    continue
                with self.subTest(document=name, target=destination):
                    self.assertTrue((path.parent / unquote(target.path)).is_file())

    def test_current_documented_facts_match_live_contracts(self):
        text = (ROOT / 'docs/INDEX.md').read_text(encoding='utf-8')
        facts = dict(re.findall(r'^\| ([^|]+?) \| `([^`]+)` \|', text, re.M))
        partition = json.loads((ROOT / 'metadata/BUILD_PARTITION_POLICY.json').read_text())
        production = sum(len(partition[key]) for key in
                         ('production_object_sources', 'vdf_sources', 'dll_entry_sources'))
        reference = len(partition['reference_sources'])
        self.assertEqual(str(production), facts['Production C++ TUs'])
        self.assertEqual(str(reference), facts['Reference C++ TUs'])
        summary = json.loads((ROOT / 'metadata/RUNTIME_CURRENT_STATUS.json').read_text())
        self.assertEqual(production, summary['production_first_party_tus'])
        self.assertEqual(reference, summary['retained_reference_tus'])
        abi = json.loads((ROOT / 'metadata/PUBLIC_DLL_ABI.json').read_text())['exports']
        ordered = sorted(abi, key=lambda name: abi[name]['ordinal'])
        self.assertEqual(', '.join(name + '@' + str(abi[name]['ordinal']) for name in ordered),
                         facts['Production C exports'])
        header = (ROOT / 'include/citrine_eclipse_mirage/platform/crypt_query.hpp').read_text()
        suffix = re.search(r'target_suffix\s*=\s*u"([^"]+)"', header)[1]
        self.assertEqual(suffix, facts['CryptQuery compatibility suffix'])
        resource = json.loads((ROOT / 'metadata/RESOURCE_METADATA_CONTRACT.json').read_text())
        self.assertEqual(str(len(resource['required']['additional_resource_assets'])),
                         facts['Additional PE resource assets'])
        avatar = json.loads((ROOT / 'metadata/AVATAR_DATA_CONTRACT.json').read_text())
        self.assertEqual('READ_ONLY_CPP_DATA_NOT_PE_RESOURCE', avatar['storage'])

    def test_current_operator_fixture_command_selects_its_real_manual_workflow(self):
        text = (ROOT / 'docs/DEVELOPMENT.md').read_text(encoding='utf-8')
        commands = re.findall(r'^gh workflow run (.+)$', text, re.M)
        self.assertEqual(['operator-output.yml --ref <branch>'], commands)
        workflow_name = commands[0].split()[0]
        workflow = (ROOT / '.github/workflows' / workflow_name).read_text()
        trigger = workflow.split('\non:\n', 1)[1].split('\npermissions:', 1)[0]
        self.assertIn('  workflow_dispatch:', trigger)
        self.assertNotIn('pull_request:', trigger)
        self.assertNotIn('push:', trigger)
        self.assertIn('OperatorOutputTests', workflow)
        self.assertNotIn('operator_output', (ROOT / '.github/workflows/ci.yml').read_text())
        facts = dict(re.findall(r'^\| ([^|]+?) \| `([^`]+)` \|',
                                (ROOT / 'docs/INDEX.md').read_text(), re.M))
        self.assertEqual(workflow_name, facts['Manual operator fixture workflow'])


class GovernanceTests(unittest.TestCase):
    def test_resource_assets_require_exact_hash_without_dropping_versioninfo(self):
        import hashlib
        policy=json.loads((ROOT/'tests/support/golden/metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text())['required']
        payload=b'owner asset fixture'
        policy['additional_resource_assets']=[{'path':[10,101,1033],'sha256':hashlib.sha256(payload).hexdigest()}]
        # Invalid VERSIONINFO remains an error; a new asset never bypasses it.
        leaves=[([16,1,1033],b'',0),([10,101,1033],payload,0)]
        errors=mustavex.validate_resource_leaves(leaves,policy)
        self.assertIn('invalid VERSIONINFO root',errors)
        self.assertFalse(any('allowlist' in e or 'byte pin' in e for e in errors))
        bad=[leaves[0],([10,101,1033],b'tampered',0)]
        self.assertTrue(any('byte pin' in e for e in mustavex.validate_resource_leaves(bad,policy)))
        self.assertTrue(any('allowlist' in e for e in mustavex.validate_resource_leaves(leaves[1:],policy)))
        self.assertTrue(any('allowlist' in e for e in mustavex.validate_resource_leaves(leaves+[leaves[1]],policy)))
        # MSVC's automatic DLL manifest is not an approved third product asset.
        manifest=([24,2,1033],b'<assembly/>',0)
        self.assertTrue(any('allowlist' in e for e in mustavex.validate_resource_leaves(leaves+[manifest],policy)))

    def test_pending_product_target_cannot_mutate_binary(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'original.dll'
            path.write_bytes(b'unchanged input; not a PE fixture')
            before = path.read_bytes()
            cp = subprocess.run([sys.executable, '-B', str(ROOT / 'scripts/ursynex_golden_finalizer.py'), str(path)], capture_output=True)
            self.assertNotEqual(0, cp.returncode)
            self.assertEqual(before, path.read_bytes())
            pending_root = Path(tmp) / 'pending-source'
            (pending_root / 'metadata').mkdir(parents=True)
            for name in golden.POLICIES:
                (pending_root / 'metadata' / name).write_text(json.dumps({'activation':'PENDING_PRODUCT_CONTRACT'}))
            report = golden.pipeline(pending_root, path, Path(tmp), pending_root / 'build/test-golden-blocked')
            self.assertEqual('BLOCKED_TARGET_SEMANTICS', report['result'])
            self.assertEqual([], report['steps'])
            self.assertEqual(before, path.read_bytes())

    def test_activation_cannot_hide_missing_product_fields(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            for name in golden.POLICIES:
                (root / 'metadata' / name).write_text(json.dumps({'activation':'READY_TARGET'}))
            self.assertEqual(4, len(golden.pending_targets(root)))

    def test_product_targets_are_configuration_ready_not_native_acceptance(self):
        self.assertEqual([], golden.pending_targets(ROOT))
        corvexon = load_target(ROOT, 'CORVEXON_GOLDEN_TRAIL.json')
        abi = json.loads((ROOT / 'metadata/PUBLIC_DLL_ABI.json').read_text())['exports']
        self.assertEqual({n:v['ordinal'] for n,v in abi.items()}, {v['name']:v['ordinal'] for v in corvexon['shipping_exports']})
        self.assertNotIn('OLE32.DLL', corvexon['import_policy']['allowed_modules'])
        self.assertIn('CRYPT32.DLL', corvexon['import_policy']['required_modules'])
        self.assertEqual(['CryptQueryObject'],corvexon['import_policy']['restricted_named_imports']['CRYPT32.DLL'])
        resource = load_target(ROOT, 'MUSTAVEX_GOLDEN_METADATA.json')
        self.assertEqual([], resource['required']['additional_resource_assets'])
        png_leaves=[([16,1,1033],b'',0),([10,101,1033],b'png',0)]
        self.assertTrue(any('allowlist' in e for e in mustavex.validate_resource_leaves(png_leaves,resource['required'])))

    def test_avatar_generator_refuses_tampered_input_before_output(self):
        import shutil
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            contract = json.loads((ROOT / 'metadata/AVATAR_DATA_CONTRACT.json').read_text())
            (root / 'metadata/AVATAR_DATA_CONTRACT.json').write_text(json.dumps(contract))
            for spec in contract['assets'].values():
                target = root / spec['source']
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(ROOT / spec['source'], target)
            output=root / 'generated/avatar_data.hpp'
            avatar_data.generate(root,output)
            before=output.read_bytes()
            profile=root / contract['assets']['profile']['source']
            profile.write_bytes(profile.read_bytes()+b'tampered')
            with self.assertRaises(ValueError):
                avatar_data.generate(root,output)
            self.assertEqual(before,output.read_bytes())

    def make_avatar_fixture(self, root):
        import hashlib
        (root / 'metadata').mkdir(parents=True)
        assets = {}
        for name in ('profile', 'fallback'):
            payload = b'\x89PNG\r\n\x1a\n' + name.encode('ascii') + b' fixture'
            path = root / 'resources' / (name + '.png')
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(payload)
            assets[name] = {'source': 'resources/' + name + '.png',
                            'sha256': hashlib.sha256(payload).hexdigest()}
        (root / 'metadata/AVATAR_DATA_CONTRACT.json').write_text(
            json.dumps({'assets': assets}), encoding='utf-8')

    def test_avatar_generator_rejects_shared_output_hardlink(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / 'source'
            self.make_avatar_fixture(root)
            victim = Path(tmp) / 'owned.txt'
            victim.write_text('original bytes')
            output = root / 'generated/avatar_data.hpp'
            output.parent.mkdir()
            os.link(victim, output)
            with self.assertRaisesRegex(ValueError, 'unshared regular'):
                avatar_data.generate(root, output)
            self.assertEqual('original bytes', victim.read_text())
            output.unlink()
            avatar_data.generate(root, output)
            expected = output.read_bytes()
            avatar_data.generate(root, output)
            self.assertEqual(expected, output.read_bytes())

    def test_avatar_generator_rejects_symlinked_output_and_parent(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / 'source'
            self.make_avatar_fixture(root)
            victim = Path(tmp) / 'owned.txt'
            victim.write_text('original bytes')
            output = root / 'generated/avatar_data.hpp'
            output.parent.mkdir()
            try:
                output.symlink_to(victim)
            except (OSError, NotImplementedError) as error:
                self.skipTest('Symlink privileges unavailable: ' + str(error))
            with self.assertRaisesRegex(ValueError, 'symlink or junction'):
                avatar_data.generate(root, output)
            self.assertEqual('original bytes', victim.read_text())
            output.unlink()
            output.parent.rmdir()
            outside = Path(tmp) / 'external'
            outside.mkdir()
            output.parent.symlink_to(outside, target_is_directory=True)
            with self.assertRaisesRegex(ValueError, 'symlink or junction'):
                avatar_data.generate(root, output)
            self.assertFalse((outside / output.name).exists())
            # A nested missing directory must not be created via an ancestor link.
            nested = output.parent / 'nested' / output.name
            with self.assertRaisesRegex(ValueError, 'symlink or junction'):
                avatar_data.generate(root, nested)
            self.assertFalse((outside / 'nested').exists())

    def test_fixture_is_never_a_product_target(self):
        with self.assertRaises(PolicyPending):
            load_target(ROOT / 'tests/support/golden', 'URSYNEX_GOLDEN_FINALIZER.json')

    def test_pipeline_order_and_failure_exit(self):
        for failed_stage in range(6):
            seen = []
            def run(command):
                index = len(seen)
                seen.append(command)
                return {'exit_code': 37 if index == failed_stage else 0, 'output': ''}
            result = golden.execute_steps([(name, i) for i, name in enumerate(golden.ORDER)], run)
            self.assertEqual(list(range(failed_stage+1)), seen)
            self.assertEqual(37, result[-1]['exit_code'])

    def test_finalizer_failed_write_keeps_original_and_cleans_temp(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'original.dll'
            path.write_bytes(b'original bytes')
            with patch.object(finalizer, '_write_file_fully', side_effect=OSError('injected disk write failure')):
                with self.assertRaises(OSError):
                    finalizer.atomic_replace_bytes(path, b'changed bytes')
            self.assertEqual(b'original bytes', path.read_bytes())
            self.assertEqual([path], list(Path(tmp).iterdir()))

    def test_unowned_metadata_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            (root / 'metadata/METADATA_OWNERSHIP.json').write_text(json.dumps({'files': {
                'METADATA_OWNERSHIP.json': {'owner': 'qa', 'purpose': 'registry', 'role': 'CURRENT_CONTRACT'},
                'SCRIPT_OWNERSHIP.json': {'owner': 'qa', 'purpose': 'script registry', 'role': 'CURRENT_CONTRACT'}},
                'schema': ownership.METADATA_SCHEMA}))
            (root / 'metadata/SCRIPT_OWNERSHIP.json').write_text(json.dumps({'schema': ownership.SCRIPT_SCHEMA, 'files': {}}))
            self.assertEqual([], ownership.check(root))
            (root / 'metadata/unowned.json').write_text('{}')
            self.assertTrue(any('Unowned' in e for e in ownership.check(root)))


    def test_uppercase_ownership_extensions_are_not_skipped(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for folder in ('metadata', 'scripts', 'tools'):
                (root / folder).mkdir()
            entry = lambda role: {'owner': 'qa', 'purpose': 'fixture', 'role': role}
            metadata = {'schema': ownership.METADATA_SCHEMA, 'files': {
                'METADATA_OWNERSHIP.json': entry('CURRENT_CONTRACT'),
                'SCRIPT_OWNERSHIP.json': entry('CURRENT_CONTRACT')}}
            scripts = {'schema': ownership.SCRIPT_SCHEMA, 'files': {}}
            def save():
                (root / 'metadata/METADATA_OWNERSHIP.json').write_text(json.dumps(metadata))
                (root / 'metadata/SCRIPT_OWNERSHIP.json').write_text(json.dumps(scripts))
            save()
            self.assertEqual([], ownership.check(root))
            cases = [
                ('metadata/unlisted.JSON', 'Unowned metadata: unlisted.JSON', 'CURRENT_CONTRACT'),
                ('scripts/unlisted.PY', 'Script ownership file-set mismatch', 'PYTHON_TOOL_OR_MODULE'),
                ('tools/unlisted.PS1', 'Script ownership file-set mismatch', 'AUTOMATION'),
                ('scripts/unlisted.CMD', 'Script ownership file-set mismatch', 'HUMAN_LAUNCHER'),
            ]
            for name, message, role in cases:
                with self.subTest(name=name):
                    path = root / name
                    path.write_text('fixture\n')
                    self.assertIn(message, ownership.check(root))
                    ledger = metadata['files'] if name.startswith('metadata/') else scripts['files']
                    key = path.name if name.startswith('metadata/') else name
                    ledger[key] = entry(role)
                    save()
                    self.assertEqual([], ownership.check(root))
                    path.unlink()
                    del ledger[key]
                    save()
                    self.assertEqual([], ownership.check(root))

    def test_ownership_schemas_and_allowed_roles_fail_closed(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            (root / 'scripts').mkdir()
            (root / 'scripts/audit_tool.py').write_text('pass\n')
            metadata = {
                'schema': ownership.METADATA_SCHEMA,
                'files': {
                    'METADATA_OWNERSHIP.json': {'owner': 'qa', 'purpose': 'metadata registry',
                                                'role': 'CURRENT_CONTRACT'},
                    'SCRIPT_OWNERSHIP.json': {'owner': 'qa', 'purpose': 'script registry',
                                              'role': 'CURRENT_CONTRACT'},
                },
            }
            scripts = {
                'schema': ownership.SCRIPT_SCHEMA,
                'files': {
                    'scripts/audit_tool.py': {'owner': 'qa', 'purpose': 'fixture script',
                                              'role': 'PYTHON_TOOL_OR_MODULE'},
                },
            }
            metadata_path = root / 'metadata/METADATA_OWNERSHIP.json'
            script_path = root / 'metadata/SCRIPT_OWNERSHIP.json'
            def write_records():
                metadata_path.write_text(json.dumps(metadata))
                script_path.write_text(json.dumps(scripts))
            write_records()
            self.assertEqual([], ownership.check(root))
            for ledger, key in ((metadata, 'metadata'), (scripts, 'script')):
                ledger['schema'] = 'unexpected.audit.schema'
                write_records()
                self.assertTrue(any('Unrecognized ' + key + ' ownership schema' in error
                                    for error in ownership.check(root)))
                ledger['schema'] = (ownership.METADATA_SCHEMA if key == 'metadata'
                                    else ownership.SCRIPT_SCHEMA)
                write_records()
            for ledger, name in ((metadata, 'METADATA_OWNERSHIP.json'),
                                 (scripts, 'scripts/audit_tool.py')):
                record = ledger['files'][name]
                role = record.pop('role')
                write_records()
                self.assertTrue(any('invalid' in error and name in error
                                    for error in ownership.check(root)))
                record['role'] = 'UNKNOWN_ROLE'
                write_records()
                self.assertTrue(any('invalid' in error and name in error
                                    for error in ownership.check(root)))
                record['role'] = role
                write_records()
                self.assertEqual([], ownership.check(root))

            for ledger, name, kind in ((metadata, 'METADATA_OWNERSHIP.json', 'metadata'),
                                       (scripts, 'scripts/audit_tool.py', 'script')):
                record = ledger['files'][name]
                for field, invalid in (('owner', True), ('owner', ['qa']),
                                       ('owner', ' \t '), ('purpose', {'text': 'qa'}),
                                       ('purpose', '   '), ('purpose', 42),
                                       ('role', []), ('role', {}), ('role', True)):
                    with self.subTest(kind=kind, field=field, value=invalid):
                        previous = record[field]
                        try:
                            record[field] = invalid
                            write_records()
                            self.assertIn('Incomplete or invalid ' + kind + ' ownership: ' + name,
                                          ownership.check(root))
                        finally:
                            record[field] = previous
                            write_records()
                        self.assertEqual([], ownership.check(root))

    def test_duplicate_ownership_keys_fail_closed(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for folder in ('metadata', 'scripts', 'tools'):
                (root / folder).mkdir()
            (root / 'scripts/audit_tool.py').write_text('pass\n')
            entry = lambda role: {'owner': 'qa', 'purpose': 'fixture', 'role': role}
            ledgers = {
                'METADATA_OWNERSHIP.json': {
                    'schema': ownership.METADATA_SCHEMA, 'files': {
                        'METADATA_OWNERSHIP.json': entry('CURRENT_CONTRACT'),
                        'SCRIPT_OWNERSHIP.json': entry('CURRENT_CONTRACT')}},
                'SCRIPT_OWNERSHIP.json': {
                    'schema': ownership.SCRIPT_SCHEMA, 'files': {
                        'scripts/audit_tool.py': entry('PYTHON_TOOL_OR_MODULE')}},
            }
            for name, document in ledgers.items():
                (root / 'metadata' / name).write_text(json.dumps(document))
            self.assertEqual([], ownership.check(root))
            for name, document in ledgers.items():
                target = root / 'metadata' / name
                original = target.read_text()
                record_name, record = next(iter(document['files'].items()))
                fields = [('schema', document['schema'], 'unexpected.schema'),
                          ('files', document['files'], {}),
                          (record_name, record, {}),
                          ('owner', record['owner'], 'Ambiguous'),
                          ('purpose', record['purpose'], 'Ambiguous'),
                          ('role', record['role'], 'UNKNOWN_ROLE')]
                for key, valid, invalid in fields:
                    needle = json.dumps(key) + ': ' + json.dumps(valid)
                    self.assertIn(needle, original)
                    for first in (invalid, valid):
                        with self.subTest(ledger=name, key=key, identical=first == valid):
                            duplicate = json.dumps(key) + ': ' + json.dumps(first) + ', ' + needle
                            try:
                                target.write_text(original.replace(needle, duplicate, 1))
                                with self.assertRaisesRegex(ValueError, 'Duplicate JSON object key'):
                                    ownership.check(root)
                            finally:
                                target.write_text(original)
                            self.assertEqual([], ownership.check(root))

    def test_duplicate_ownership_cli_fails_and_recovers(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for folder in ('metadata', 'scripts', 'tools'):
                (root / folder).mkdir()
            checker = root / 'scripts/check_metadata_ownership.py'
            helper = root / 'scripts/project_files.py'
            checker.write_bytes(Path(ownership.__file__).read_bytes())
            helper.write_bytes(Path(inventory.__file__).read_bytes())
            entry = lambda role: {'owner': 'qa', 'purpose': 'fixture', 'role': role}
            ledgers = {
                'METADATA_OWNERSHIP.json': {'schema': ownership.METADATA_SCHEMA, 'files': {
                    'METADATA_OWNERSHIP.json': entry('CURRENT_CONTRACT'),
                    'SCRIPT_OWNERSHIP.json': entry('CURRENT_CONTRACT')}},
                'SCRIPT_OWNERSHIP.json': {'schema': ownership.SCRIPT_SCHEMA, 'files': {
                    'scripts/check_metadata_ownership.py': entry('PYTHON_TOOL_OR_MODULE'),
                    'scripts/project_files.py': entry('PYTHON_TOOL_OR_MODULE')}},
            }
            for name, document in ledgers.items():
                (root / 'metadata' / name).write_text(json.dumps(document))
            def run():
                return subprocess.run([sys.executable, '-B', str(checker)],
                                      capture_output=True, text=True, timeout=15)
            self.assertEqual(0, run().returncode)
            for name in ledgers:
                target = root / 'metadata' / name
                original = target.read_text()
                try:
                    target.write_text(original.replace('"owner": "qa"',
                                      '"owner": "Ambiguous", "owner": "qa"', 1))
                    result = run()
                    self.assertEqual(1, result.returncode, result.stdout + result.stderr)
                    self.assertIn('METADATA_OWNERSHIP: FAIL', result.stdout)
                    self.assertIn('Duplicate JSON object key', result.stdout)
                    self.assertNotIn('Traceback', result.stderr)
                finally:
                    target.write_text(original)
                self.assertEqual(0, run().returncode)

    def test_launcher_result_and_argument_mutations(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            (root / 'scripts/build_orchestration').mkdir(parents=True)
            registry = json.loads((ROOT / 'metadata/HUMAN_ENTRYPOINT_REGISTRY.json').read_text())
            registry['entrypoints'] = [registry['entrypoints'][0]]
            (root / 'metadata/HUMAN_ENTRYPOINT_REGISTRY.json').write_text(json.dumps(registry))
            entry = registry['entrypoints'][0]
            original = (ROOT / entry['launcher']).read_text()
            (root / entry['automation']).write_text((ROOT / entry['automation']).read_text())
            path = root / entry['launcher']
            path.write_text(original)
            self.assertEqual([], launchers.check(root))
            for token in ['%*', '-LauncherOwnsSummary %*', 'exit /b %RC%',
                          'ForegroundColor Red', 'Exit code: %RC%', 'pause >nul']:
                path.write_text(original.replace(token, 'REMOVED'))
                self.assertTrue(launchers.check(root), token)
            path.write_text(original)
            # The old suffix check accepted comments and echo as executable exit.
            for fake in ('REM exit /b %RC%', 'echo exit /b %RC%'):
                with self.subTest(fake_cmd=fake):
                    path.write_text(original.replace('exit /b %RC%', fake))
                    self.assertTrue(any('Launcher masks child exit' in e for e in
                                        launchers.check(root)))
            path.write_text(original)
            automation=root/entry['automation']
            original_automation = automation.read_text()
            for fake in ('# exit $LASTEXITCODE', 'Write-Host exit $LASTEXITCODE'):
                with self.subTest(fake_ps=fake):
                    automation.write_text(original_automation.replace('exit $LASTEXITCODE', fake))
                    self.assertTrue(any('Automation masks child exit' in e for e in
                                        launchers.check(root)))
            automation.write_text(original_automation.replace('exit $LASTEXITCODE','exit 0'))
            self.assertTrue(any('masks child exit' in e for e in launchers.check(root)))

    def test_launcher_scan_rejects_unregistered_uppercase_cmd_and_accepts_owned_entry(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            (root / 'scripts').mkdir()
            registry = root / 'metadata/HUMAN_ENTRYPOINT_REGISTRY.json'
            launcher = root / 'scripts/review.CMD'
            automation = root / 'scripts/review.ps1'
            registry.write_text(json.dumps({'entrypoints': []}))
            (root / 'scripts/ignored.CMD').mkdir()
            self.assertEqual([], launchers.check(root))
            launcher.write_text('REM unregistered fixture\n')
            self.assertIn('Registered launcher set differs from actual CMD set',
                          launchers.check(root))
            automation.write_text('param([switch]$LauncherOwnsSummary)\n'
                                  '& ./fixture_profile.ps1 -Mode "Native" '
                                  '-LauncherOwnsSummary:$LauncherOwnsSummary\nexit $LASTEXITCODE\n')
            launcher.write_text('\n'.join([
                '@echo off', 'powershell.exe -NoProfile -File "%~dp0review.ps1" -LauncherOwnsSummary %*',
                'set "RC=%ERRORLEVEL%"', 'echo Exit code: %RC%',
                'REM ForegroundColor Green', 'REM ForegroundColor Red',
                'REM SURGE - %LABEL%', 'pause >nul', 'exit /b %RC%',
            ]) + '\n')
            registry.write_text(json.dumps({'entrypoints': [{
                'launcher': 'scripts/review.CMD',
                'automation': 'scripts/review.ps1',
                'mode': 'Native',
            }]}))
            self.assertEqual([], launchers.check(root))
            registered = json.loads(registry.read_text())
            registered['entrypoints'][0]['launcher'] = 'scripts/review.cmd'
            registry.write_text(json.dumps(registered))
            self.assertIn('Registered launcher set differs from actual CMD set', launchers.check(root))
            registered['entrypoints'][0]['launcher'] = 'scripts/review.CMD'
            registry.write_text(json.dumps(registered))
            self.assertEqual([], launchers.check(root))

    def test_launcher_summary_ownership_must_reach_the_profile(self):
        entries = json.loads((ROOT / 'metadata/HUMAN_ENTRYPOINT_REGISTRY.json').read_text())['entrypoints']
        for entry in entries:
            with self.subTest(launcher=entry['launcher']), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                (root / 'metadata').mkdir()
                (root / 'metadata/HUMAN_ENTRYPOINT_REGISTRY.json').write_text(
                    json.dumps({'entrypoints': [entry]}))
                for name in (entry['launcher'], entry['automation']):
                    path = root / name
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_bytes((ROOT / name).read_bytes())
                self.assertEqual([], launchers.check(root))
                target = root / entry['automation']
                original = target.read_text()
                for token in ('[switch]$LauncherOwnsSummary',
                              '-LauncherOwnsSummary:$LauncherOwnsSummary'):
                    target.write_text(original.replace(token, ''))
                    self.assertTrue(any('summary ownership' in e
                                        for e in launchers.check(root)), token)


class OperatorOutputTests(unittest.TestCase):
    """Exercise actual wrappers/formatting; fake only the compiler/validator work."""

    @unittest.skipUnless(shutil.which('powershell.exe') or shutil.which('pwsh'),
                         'PowerShell is required for the provenance fixture')
    def test_provenance_keeps_tool_hashes_and_versions_without_install_paths(self):
        shell = shutil.which('powershell.exe') or shutil.which('pwsh')
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / 'private toolchain path (1)'
            root.mkdir()
            names = ['cmake.exe', 'python.exe', 'cl.exe', 'link.exe', 'nmake.exe']
            for name in names:
                (root / name).write_bytes(('synthetic tool identity: ' + name).encode('ascii'))
            harness = root / 'provenance.ps1'
            harness.write_text(r'''
param([string]$FixtureRoot, [string]$Common)
$ErrorActionPreference = 'Stop'
# A Windows PowerShell child can inherit a Core-only PSModulePath from CTest.
Import-Module (Join-Path $PSHOME 'Modules/Microsoft.PowerShell.Utility/Microsoft.PowerShell.Utility.psd1') -ErrorAction Stop
. $Common
Write-ToolchainProvenance (Join-Path $FixtureRoot 'provenance.log') `
    (Join-Path $FixtureRoot 'Visual Studio') (Join-Path $FixtureRoot 'vcvars64.bat') `
    (Join-Path $FixtureRoot 'cmake.exe') (Join-Path $FixtureRoot 'python.exe') `
    (Join-Path $FixtureRoot 'cl.exe') (Join-Path $FixtureRoot 'link.exe') `
    (Join-Path $FixtureRoot 'nmake.exe')
''', encoding='ascii')
            result = subprocess.run([shell, '-NoLogo', '-NoProfile', '-NonInteractive',
                                     '-File', str(harness), str(root),
                                     str(ROOT / 'scripts/build_orchestration/build_common.ps1')],
                                    capture_output=True, text=True, timeout=20)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            text = (root / 'provenance.log').read_text(encoding='utf-8-sig')
            lines = text.splitlines()
            self.assertEqual(['CITRINE_ECLIPSE_MIRAGE_TOOLCHAIN_PROVENANCE_V1',
                              'VS_INSTALLATION=<redacted>', 'VCVARS64=<redacted>'], lines[:3])
            self.assertEqual(8, len(lines))
            self.assertNotIn(str(root), text)
            self.assertNotIn('|path=', text)
            for label, name, line in zip(['cmake', 'python', 'cl', 'link', 'nmake'], names, lines[3:]):
                self.assertEqual(f'{label}|tool={name}|file_version=<none>|sha256={msvc_evidence.sha(root / name)}', line)

    def make_fixture(self, tmp):
        root = Path(tmp) / 'operator fixture (1)'
        root.mkdir()
        entries = json.loads((ROOT / 'metadata/HUMAN_ENTRYPOINT_REGISTRY.json').read_text())['entrypoints']
        for entry in entries:
            for name in (entry['launcher'], entry['automation']):
                target = root / name
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes((ROOT / name).read_bytes())
        directory = root / 'scripts/build_orchestration'
        shutil.copyfile(ROOT / 'scripts/build_orchestration/build_common.ps1',
                        directory / 'build_common.ps1')
        # No MSVC build or Golden acceptance is performed by this output fixture.
        (directory / 'build_profile.ps1').write_text(r'''
param([string]$Mode,[string]$BuildDir,[string]$Config,[switch]$LauncherOwnsSummary)
. (Join-Path $PSScriptRoot 'build_common.ps1')
$code = [int]$env:SURGE_OUTPUT_FIXTURE_EXIT
$labels = @{Compile='COMPILE ONLY';Debug='DEBUG BUILD';Native='NATIVE ACCEPTANCE';Crt='STATIC CRT ACCEPTANCE';Golden='PRODUCTION BUILD'}
$record = @{mode=$Mode;build_dir=$BuildDir;configuration=$Config;owns_summary=[bool]$LauncherOwnsSummary}
Write-Host ('FIXTURE_ARGS:' + ($record | ConvertTo-Json -Compress))
if ($code -eq 0 -or ($Mode -eq 'Golden' -and $code -eq 3)) {
    Write-SelectedProfileResult -ProfileLabel $labels[$Mode] -Dll 'fixture/citrine_eclipse_mirage.dll' -Sha256 ('a' * 64) -GoldenBlocked:($code -eq 3) -LauncherOwnsSummary:$LauncherOwnsSummary
} else {
    Write-Host 'ERROR: injected validation failure'
}
exit $code
''', encoding='ascii')
        (root / 'entries.json').write_text(json.dumps(entries), encoding='ascii')
        return root, entries

    @unittest.skipUnless(shutil.which('powershell.exe') or shutil.which('pwsh'),
                         'PowerShell is required for the output-only fixture')
    def test_powershell_routes_summary_and_preserves_arguments_and_exit_codes(self):
        shell = shutil.which('powershell.exe') or shutil.which('pwsh')
        with tempfile.TemporaryDirectory() as tmp:
            root, entries = self.make_fixture(tmp)
            harness = root / 'harness.ps1'
            harness.write_text(r'''
param([string]$FixtureRoot)
$ErrorActionPreference = 'Stop'
$entries = Get-Content -LiteralPath (Join-Path $FixtureRoot 'entries.json') -Raw | ConvertFrom-Json
foreach ($entry in $entries) {
    $codes = @(0,105)
    if ($entry.mode -eq 'Golden') { $codes += 3 }
    foreach ($code in $codes) {
        foreach ($owned in @($false,$true)) {
            $env:SURGE_OUTPUT_FIXTURE_EXIT = [string]$code
            $lines = @(& (Join-Path $FixtureRoot $entry.automation) -BuildDir 'build\fixture path (1)' -Config RelWithDebInfo -LauncherOwnsSummary:$owned 6>&1 | ForEach-Object { [string]$_ })
            $actualCode = $LASTEXITCODE
            @{mode=$entry.mode;expected_code=$code;actual_code=$actualCode;owned=$owned;lines=$lines} | ConvertTo-Json -Compress -Depth 5
        }
    }
}
exit 0
''', encoding='ascii')
            result = subprocess.run([shell, '-NoLogo', '-NoProfile', '-NonInteractive',
                                     '-ExecutionPolicy', 'Bypass', '-File', str(harness), str(root)],
                                    capture_output=True, text=True, timeout=45)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            records = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
            self.assertEqual(22, len(records), result.stdout)
            labels = {entry['mode']: entry['label'] for entry in entries}
            for record in records:
                with self.subTest(mode=record['mode'], code=record['expected_code'], owned=record['owned']):
                    self.assertEqual(record['expected_code'], record['actual_code'])
                    lines = record['lines']
                    args = json.loads(next(line.removeprefix('FIXTURE_ARGS:')
                                           for line in lines if line.startswith('FIXTURE_ARGS:')))
                    self.assertEqual(record['mode'], args['mode'])
                    self.assertEqual(r'build\fixture path (1)', args['build_dir'])
                    self.assertEqual('RelWithDebInfo', args['configuration'])
                    self.assertEqual(record['owned'], args['owns_summary'])
                    if record['expected_code'] == 0:
                        self.assertEqual(0 if record['owned'] else 1,
                                         lines.count(labels[record['mode']] + ': PASS'))
                    elif record['expected_code'] == 3:
                        self.assertEqual(0 if record['owned'] else 1,
                                         sum('Golden: BLOCKED' in line for line in lines))
                        self.assertEqual(0 if record['owned'] else 1, lines.count('Exit code 3.'))
                        self.assertIn('See golden/GOLDEN_REPORT.json. This is not Golden acceptance.', lines)
                    else:
                        self.assertIn('ERROR: injected validation failure', lines)
                        self.assertFalse(any(': PASS' in line for line in lines))
                    if record['expected_code'] in (0, 3):
                        self.assertEqual(1, lines.count('DLL: citrine_eclipse_mirage.dll'))
                        self.assertEqual(1, lines.count('SHA256: ' + 'a' * 64))

    def run_cmd(self, launcher, code):
        env = dict(os.environ, SURGE_OUTPUT_FIXTURE_EXIT=str(code))
        # CMD parses its own quotes; list2cmdline's CRT-style backslash escapes
        # become literal characters here. /s removes just the outer quote pair.
        command = ('"' + os.environ.get('COMSPEC', 'cmd.exe') + '" /d /s /c ""' +
                   str(launcher) + r'" -BuildDir "build\fixture path (1)" -Config RelWithDebInfo"')
        return subprocess.run(command,
                              input='\n', capture_output=True, text=True, env=env, timeout=25)

    @unittest.skipUnless(os.name == 'nt', 'Requires real Windows CMD and Windows PowerShell')
    def test_cmd_outputs_one_verdict_with_color_and_with_renderer_failure(self):
        with tempfile.TemporaryDirectory() as tmp:
            root, entries = self.make_fixture(tmp)
            for fallback in (False, True):
                for entry in entries:
                    launcher = root / entry['launcher']
                    if fallback:
                        text = launcher.read_text(encoding='ascii')
                        text, replacements = re.subn(
                            r'^  powershell\.exe -NoLogo -NoProfile -Command .*$',
                            '  cmd.exe /d /c exit 1', text, flags=re.MULTILINE)
                        self.assertGreaterEqual(replacements, 2, 'Renderer fault was not injected')
                        launcher.write_text(text, encoding='ascii')
                    codes = (0, 105, 3) if entry['mode'] == 'Golden' else (0, 105)
                    for code in codes:
                        with self.subTest(mode=entry['mode'], code=code, fallback=fallback):
                            result = self.run_cmd(launcher, code)
                            output = result.stdout + result.stderr
                            self.assertEqual(code, result.returncode, output)
                            verdict = 'PASS' if code == 0 else ('BLOCKED' if code == 3 else 'FAIL')
                            self.assertEqual(1, output.count(entry['label'] + ': ' + verdict), output)
                            self.assertEqual(1, output.count('Exit code: ' + str(code)), output)
                            self.assertEqual(1, output.count('Press any key to close this operator window.'), output)
                            args_line = next(line for line in output.splitlines() if line.startswith('FIXTURE_ARGS:'))
                            args = json.loads(args_line.removeprefix('FIXTURE_ARGS:'))
                            self.assertTrue(args['owns_summary'])
                            self.assertEqual(r'build\fixture path (1)', args['build_dir'])
                            if code in (0, 3):
                                self.assertEqual(1, output.count('DLL: citrine_eclipse_mirage.dll'), output)
                                self.assertEqual(1, output.count('SHA256: ' + 'a' * 64), output)
                            if code == 3:
                                self.assertNotIn('Golden: BLOCKED', output)
                                self.assertIn('golden/GOLDEN_REPORT.json', output)
                            if code == 105:
                                self.assertIn('ERROR: injected validation failure', output)

    @unittest.skipUnless(os.name == 'nt', 'Requires real Windows CMD and Windows PowerShell')
    def test_cmd_missing_automation_retains_one_failure_and_exit_two(self):
        with tempfile.TemporaryDirectory() as tmp:
            root, entries = self.make_fixture(tmp)
            for entry in entries:
                with self.subTest(mode=entry['mode']):
                    (root / entry['automation']).unlink()
                    result = self.run_cmd(root / entry['launcher'], 0)
                    output = result.stdout + result.stderr
                    self.assertEqual(2, result.returncode, output)
                    self.assertEqual(1, output.count(entry['label'] + ': FAIL'), output)
                    self.assertEqual(1, output.count('Exit code: 2'), output)
                    self.assertIn('ERROR: Missing target', output)
                    self.assertNotIn('FIXTURE_ARGS:', output)

    @unittest.skipUnless(os.name == 'nt', 'Requires real Windows CMD and Windows PowerShell')
    def test_cmd_negative_control_reproduces_the_previous_duplicate(self):
        with tempfile.TemporaryDirectory() as tmp:
            root, entries = self.make_fixture(tmp)
            entry = next(e for e in entries if e['mode'] == 'Golden')
            launcher = root / entry['launcher']
            text = launcher.read_text(encoding='ascii')
            self.assertIn('-LauncherOwnsSummary %*', text)
            # Restore the former CMD call: both layers now publish a verdict.
            launcher.write_text(text.replace('-LauncherOwnsSummary %*', '%*'), encoding='ascii')
            result = self.run_cmd(launcher, 0)
            output = result.stdout + result.stderr
            self.assertEqual(0, result.returncode, output)
            self.assertEqual(2, output.count('PRODUCTION BUILD: PASS'), output)


class BuildWordingTests(unittest.TestCase):
    def test_uppercase_metadata_json_is_included_in_build_wording_scan(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            policy = root / 'metadata/contract.JSON'
            policy.write_text('LOCAL_GOLDEN_PASS\n')
            self.assertEqual(['Obsolete build wording: metadata/contract.JSON:1'],
                             launchers.check_build_wording(root))
            policy.write_text('GOLDEN_PASS\n')
            self.assertEqual([], launchers.check_build_wording(root))

    def test_active_operator_tools_and_contracts_reject_retired_labels(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            for name in ('scripts/build_orchestration/build.ps1','tools/inspect.py',
                         'cmake/Policy.cmake','.github/workflows/ci.yml',
                         'metadata/POLICY.json','src/platform/module.cpp',
                         'include/citrine_eclipse_mirage/platform/module.hpp','CMakeLists.txt'):
                path=root/name
                path.parent.mkdir(parents=True,exist_ok=True)
                for label in ('NOT_RELEASE','LOCAL_GOLDEN_PASS','LOCAL_BUILD_RECEIPT.json',
                              'LOCAL_DIST_CONTRACT.json','Local Golden','local build',
                              'golden-local','READY_LOCAL_TARGET'):
                    path.write_text(label,encoding='utf-8')
                    self.assertEqual(['Obsolete build wording: '+name+':1'],
                                     launchers.check_build_wording(root))
                path.write_text('GOLDEN_PASS\nBUILD_RECEIPT.json\nDIST_RECEIPT.json\n',encoding='utf-8')
                self.assertEqual([],launchers.check_build_wording(root))

    def test_historical_evidence_and_vendor_bytes_are_not_rewritten(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            for name in ('metadata/evidence/OLD.json','docs/evidence/OLD.log',
                         'third_party/vendor/tool.py','tests/fixtures/old.txt'):
                path=root/name
                path.parent.mkdir(parents=True,exist_ok=True)
                path.write_text('NOT_RELEASE; LOCAL_GOLDEN_PASS',encoding='utf-8')
            before={path:path.read_bytes() for path in root.rglob('*') if path.is_file()}
            self.assertEqual([],launchers.check_build_wording(root))
            self.assertEqual(before,{path:path.read_bytes() for path in before})


class DistTests(unittest.TestCase):
    """Synthetic process results test orchestration only; never stand in for MSVC evidence."""
    def setUp(self):
        import shutil
        self.temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)/'source'
        (self.root/'metadata').mkdir(parents=True)
        (self.root/'metadata/ACTIVE_SOURCE_MANIFEST.json').write_text('{"fixture_only":true,"files":{}}')
        shutil.copyfile(ROOT/'metadata/DIST_CONTRACT.json',self.root/'metadata/DIST_CONTRACT.json')
        self.build=self.root/'build/golden'
        self.build.mkdir(parents=True)
        self.raw=self.build/'citrine_eclipse_mirage.unsigned.dll'
        self.raw.write_bytes(b'synthetic orchestration fixture, not a PE or MSVC result')
        self.output=self.build/'golden'

    def run_pipeline(self,failed_stage=None,runtime_failure=False,corrupt_runtime=False):
        calls=[]
        def run(command,**kwargs):
            self.assertEqual('1',kwargs['env']['PYTHONDONTWRITEBYTECODE'])
            index=len(calls);calls.append(command)
            if index==6:
                self.assertTrue(command[0].endswith('citrine_eclipse_mirage_runtime_host.exe'))
                self.assertEqual('citrine_eclipse_mirage.dll',Path(command[-2]).name)
                self.assertEqual('--xml-contracts',command[-1])
                if corrupt_runtime:Path(command[-2]).write_bytes(b'changed during runtime')
            return subprocess.CompletedProcess(command,37 if index==failed_stage or (index==6 and runtime_failure) else 0,'synthetic process result\n','')
        with patch.object(golden,'pending_targets',return_value=[]),patch.object(golden.subprocess,'run',side_effect=run):
            report=golden.pipeline(self.root,self.raw,self.build,self.output)
        return report,calls

    def dist_bytes(self):
        return {p.name:p.read_bytes() for p in (self.root/'dist').iterdir() if p.is_file()}

    def test_cli_keeps_stage_and_runtime_diagnostics_in_report_only(self):
        diagnostic = 'private diagnostic: C:\\operator\\workspace\\candidate.dll'
        for failed_stage, runtime_failure in [(None, False), *[(i, False) for i in range(6)], (None, True)]:
            with self.subTest(failed_stage=failed_stage, runtime_failure=runtime_failure):
                calls = []
                def run(command, **kwargs):
                    index = len(calls)
                    calls.append(command)
                    code = 37 if index == failed_stage or (index == 6 and runtime_failure) else 0
                    return subprocess.CompletedProcess(command, code, diagnostic + '\n', 'stderr: ' + diagnostic)
                stdout, stderr = io.StringIO(), io.StringIO()
                argv = ['run_golden_pipeline.py', '--dll', str(self.raw),
                        '--build-dir', str(self.build), '--output-dir', str(self.output)]
                with patch.object(golden, 'ROOT', self.root), patch.object(sys, 'argv', argv), \
                     patch.object(golden, 'pending_targets', return_value=[]), \
                     patch.object(golden.subprocess, 'run', side_effect=run), \
                     contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
                    code = golden.main()
                report = json.loads((self.output / 'GOLDEN_REPORT.json').read_text(encoding='utf-8'))
                success = failed_stage is None and not runtime_failure
                count = 6 if failed_stage is None else failed_stage + 1
                state = 'GOLDEN_PASS' if success else 'FAIL'
                runtime_exit = 37 if runtime_failure else 0 if failed_stage is None else 'NOT_RUN'
                self.assertEqual(0 if success else 1, code)
                self.assertEqual(state, report['result'])
                self.assertEqual(golden.ORDER[:count], [step['stage'] for step in report['steps']])
                self.assertEqual(count + (failed_stage is None), len(calls))
                for step in report['steps']:
                    self.assertEqual(diagnostic + '\nstderr: ' + diagnostic, step['output'])
                if failed_stage is None:
                    self.assertEqual(runtime_exit, report['runtime_check']['exit_code'])
                    self.assertEqual(diagnostic + '\nstderr: ' + diagnostic, report['runtime_check']['output'])
                    self.assertIn('Final-name runtime check: ' + ('PASS' if success else 'FAIL'), stdout.getvalue())
                else:
                    self.assertNotIn('runtime_check', report)
                for index, stage in enumerate(golden.ORDER[:count]):
                    status = 'FAIL' if index == failed_stage else 'PASS'
                    self.assertIn(f'[{index+1}/6] {stage}: {status}', stdout.getvalue())
                self.assertEqual(f'GOLDEN_PIPELINE: {state} stages={count} runtime_exit={runtime_exit}',
                                 stdout.getvalue().splitlines()[-1])
                self.assertNotIn(diagnostic, stdout.getvalue() + stderr.getvalue())
                self.assertNotIn('"output":', stdout.getvalue())
                self.assertEqual('', stderr.getvalue())

    def test_cli_blocked_target_keeps_pending_details_in_report(self):
        pending = ['policy unresolved at C:\\operator\\workspace\\policy.json']
        stdout = io.StringIO()
        argv = ['run_golden_pipeline.py', '--dll', str(self.raw),
                '--build-dir', str(self.build), '--output-dir', str(self.output)]
        with patch.object(golden, 'ROOT', self.root), patch.object(sys, 'argv', argv), \
             patch.object(golden, 'pending_targets', return_value=pending), \
             patch.object(golden.subprocess, 'run') as child, contextlib.redirect_stdout(stdout):
            code = golden.main()
        self.assertEqual(3, code)
        child.assert_not_called()
        report = json.loads((self.output / 'GOLDEN_REPORT.json').read_text(encoding='utf-8'))
        self.assertEqual(pending, report['pending'])
        self.assertEqual([], report['steps'])
        self.assertEqual('BLOCKED_TARGET_SEMANTICS', report['result'])
        self.assertEqual('GOLDEN_PIPELINE: BLOCKED_TARGET_SEMANTICS stages=0 runtime_exit=NOT_RUN\n',
                         stdout.getvalue())

    def test_success_stages_exact_final_name_hash_receipt_and_original_bytes(self):
        before=self.raw.read_bytes()
        report,calls=self.run_pipeline()
        self.assertEqual('GOLDEN_PASS',report['result'])
        self.assertEqual(7,len(calls))
        target=self.root/'dist/citrine_eclipse_mirage.dll'
        self.assertEqual(before,target.read_bytes())
        self.assertEqual(before,self.raw.read_bytes())
        self.assertEqual(golden.sha(target)+'  citrine_eclipse_mirage.dll\n',Path(report['dist_sha256_file']).read_text())
        receipt=json.loads(Path(report['dist_receipt']).read_text())
        self.assertEqual(report['sha256'],receipt['sha256'])
        self.assertEqual(golden.sha(self.root/'metadata/ACTIVE_SOURCE_MANIFEST.json'),receipt['source_manifest_sha256'])
        self.assertEqual('surge.dist-receipt.v1',receipt['schema'])
        self.assertEqual('surge.golden.v1',report['schema'])
        for value in (receipt,report):
            self.assertNotIn('classification',value)
            self.assertNotIn('official_release',value)
            self.assertNotIn('library_publication',value)


    def test_golden_output_symlink_cannot_overwrite_external_file(self):
        self.output.mkdir(parents=True, exist_ok=True)
        victim = self.root.parent / 'outside-golden-victim.bin'
        victim.write_bytes(b'original external fixture bytes')
        candidate_hash = golden.sha(self.raw)[:16]
        output_name = 'citrine_eclipse_mirage.' + candidate_hash + '.dll'
        artifact = self.output / output_name
        sidecar = Path(str(artifact) + '.sha256')
        for unsafe in (artifact, sidecar):
            with self.subTest(unsafe=unsafe.name):
                try:
                    unsafe.symlink_to(victim)
                except (OSError, NotImplementedError):
                    self.skipTest('Symlink fixtures are unavailable')
                try:
                    with self.assertRaisesRegex(ValueError, 'Unsafe existing Golden output member'):
                        self.run_pipeline()
                    self.assertEqual(b'original external fixture bytes', victim.read_bytes())
                    self.assertTrue(unsafe.is_symlink())
                    self.assertFalse((self.root / 'dist').exists())
                    self.assertFalse(list(self.output.glob('.golden-*.partial')))
                finally:
                    unsafe.unlink()

    def test_golden_output_staging_leaves_no_partial_files(self):
        report, calls = self.run_pipeline()
        self.assertEqual('GOLDEN_PASS', report['result'])
        self.assertEqual(7, len(calls))
        artifact = Path(report['artifact'])
        self.assertEqual(self.raw.read_bytes(), artifact.read_bytes())
        self.assertEqual(golden.sha(artifact) + '  ' + artifact.name + '\n',
                         Path(report['sha256_file']).read_text())
        self.assertFalse(list(self.output.glob('.golden-*.partial')))

    def test_every_golden_failure_preserves_previous_dist_and_skips_runtime(self):
        self.run_pipeline()
        before=self.dist_bytes()
        for stage in range(6):
            with self.subTest(stage=stage):
                report,calls=self.run_pipeline(failed_stage=stage)
                self.assertEqual('FAIL',report['result'])
                self.assertEqual(stage+1,len(calls))
                self.assertNotIn('runtime_check',report)
                self.assertEqual(before,self.dist_bytes())

    def test_runtime_failure_or_changed_candidate_cannot_create_dist(self):
        for corrupt in (False,True):
            with self.subTest(corrupt=corrupt):
                report,_=self.run_pipeline(runtime_failure=not corrupt,corrupt_runtime=corrupt)
                self.assertEqual('FAIL',report['result'])
                self.assertNotEqual(0,report['runtime_check']['exit_code'])
                self.assertFalse((self.root/'dist').exists())

    def test_incomplete_result_cannot_stage_dist(self):
        with self.assertRaises(ValueError):
            golden.stage_dist(self.root,self.raw,{'result':'GOLDEN_PASS','steps':[]})
        self.assertFalse((self.root/'dist').exists())

    def test_replace_failure_restores_previous_bundle(self):
        report,_=self.run_pipeline()
        before=self.dist_bytes()
        artifact=Path(report['artifact']);artifact.write_bytes(b'new synthetic candidate')
        report['sha256']=golden.sha(artifact)
        report['runtime_check'].update(sha256_before=report['sha256'],sha256_after=report['sha256'])
        replace=golden.os.replace
        def fail_sidecar(source,target):
            if Path(source).name=='citrine_eclipse_mirage.dll.sha256' and '.golden-transaction' in Path(source).parts:
                raise OSError('injected sidecar commit failure')
            return replace(source,target)
        with patch.object(golden.os,'replace',side_effect=fail_sidecar):
            with self.assertRaises(OSError):golden.stage_dist(self.root,artifact,report)
        self.assertEqual(before,self.dist_bytes())
        self.assertTrue((self.root/'dist/.golden.lock').is_file())
        self.assertFalse(list((self.root/'dist').glob('.candidate-*')))


    def _crash_dist_operation(self, artifact, report, replace_number, operation="promote"):
        """Terminate a real child immediately after one public file replacement."""
        report_path = self.build / 'crash-report.json'
        report_path.write_text(json.dumps(report), encoding='utf-8')
        script = """
import json
import os
from pathlib import Path
import sys
sys.path.insert(0, sys.argv[1])
import run_golden_pipeline as golden
root = Path(sys.argv[2]).resolve()
artifact = Path(sys.argv[3]).resolve()
report = json.loads(Path(sys.argv[4]).read_text(encoding='utf-8'))
stop_after = int(sys.argv[5])
original_replace = golden.os.replace
count = 0
def replace_and_exit(source, target):
    global count
    original_replace(source, target)
    if Path(target).parent == root / 'dist' and Path(target).name in golden.DIST_NAMES:
        count += 1
        if count == stop_after:
            os._exit(79)
golden.os.replace = replace_and_exit
if sys.argv[6] == "recover":
    golden.recover_dist(root)
else:
    golden.stage_dist(root, artifact, report)
"""
        result = subprocess.run(
            [sys.executable, '-B', '-c', script, str(ROOT / 'scripts'),
             str(self.root), str(artifact), str(report_path), str(replace_number), operation],
            capture_output=True, text=True, timeout=25)
        self.assertEqual(79, result.returncode, result.stdout + result.stderr)

    def _changed_candidate(self, report):
        artifact = Path(report['artifact'])
        artifact.write_bytes(b'synthetic replacement candidate; not a PE')
        report['sha256'] = golden.sha(artifact)
        report['runtime_check'].update(sha256_before=report['sha256'], sha256_after=report['sha256'])
        return artifact

    def test_process_exit_after_each_replace_replays_previous_bundle(self):
        for count in (1, 2, 3):
            with self.subTest(replace_count=count):
                report, _ = self.run_pipeline()
                before = self.dist_bytes()
                artifact = self._changed_candidate(report)
                self._crash_dist_operation(artifact, report, count)
                self.assertTrue((self.root / 'dist' / golden.DIST_TRANSACTION).is_dir())
                self.assertTrue(golden.recover_dist(self.root))
                self.assertEqual(before, self.dist_bytes())
                self.assertFalse((self.root / 'dist' / golden.DIST_TRANSACTION).exists())
                self.assertFalse(golden.recover_dist(self.root))

    def test_interrupted_recovery_replays_original_snapshot_again(self):
        report, _ = self.run_pipeline()
        before = self.dist_bytes()
        artifact = self._changed_candidate(report)
        self._crash_dist_operation(artifact, report, 2)
        self._crash_dist_operation(artifact, report, 1, operation="recover")
        self.assertTrue((self.root / 'dist' / golden.DIST_TRANSACTION).is_dir())
        self.assertTrue(golden.recover_dist(self.root))
        self.assertEqual(before, self.dist_bytes())
        self.assertFalse(golden.recover_dist(self.root))

    def test_journal_retirement_failure_restores_previous_bundle(self):
        report, _ = self.run_pipeline()
        before = self.dist_bytes()
        artifact = self._changed_candidate(report)
        transaction = (self.root / 'dist' / golden.DIST_TRANSACTION).resolve()
        replace = golden.os.replace
        failed = False
        def fail_retirement_once(source, target):
            nonlocal failed
            if Path(source).resolve() == transaction and Path(target).name.startswith('.golden-retired-') and not failed:
                failed = True
                raise OSError('injected journal retirement failure')
            return replace(source, target)
        with patch.object(golden.os, 'replace', side_effect=fail_retirement_once):
            with self.assertRaisesRegex(OSError, 'journal retirement failure'):
                golden.stage_dist(self.root, artifact, report)
        self.assertTrue(failed)
        self.assertEqual(before, self.dist_bytes())
        self.assertFalse(transaction.exists())

    def test_failed_golden_run_recovers_old_bundle_before_testing(self):
        report, _ = self.run_pipeline()
        before = self.dist_bytes()
        artifact = self._changed_candidate(report)
        self._crash_dist_operation(artifact, report, 2)
        failure, calls = self.run_pipeline(failed_stage=0)
        self.assertEqual('FAIL', failure['result'])
        self.assertEqual(1, len(calls))
        self.assertEqual(before, self.dist_bytes())

    def test_tampered_recovery_snapshot_fails_closed(self):
        report, _ = self.run_pipeline()
        artifact = self._changed_candidate(report)
        self._crash_dist_operation(artifact, report, 1)
        transaction = self.root / 'dist' / golden.DIST_TRANSACTION
        (transaction / 'previous' / golden.DIST_NAMES[0]).write_bytes(b'corrupted rollback data')
        before = self.dist_bytes()
        with self.assertRaisesRegex(ValueError, 'recovery snapshot'):
            golden.recover_dist(self.root)
        self.assertEqual(before, self.dist_bytes())
        self.assertTrue(transaction.is_dir())

    def test_first_publish_interruption_reverts_to_no_bundle(self):
        report, _ = self.run_pipeline()
        for name in golden.DIST_NAMES:
            (self.root / 'dist' / name).unlink()
        artifact = self._changed_candidate(report)
        self._crash_dist_operation(artifact, report, 1)
        self.assertTrue(golden.recover_dist(self.root))
        for name in golden.DIST_NAMES:
            self.assertFalse((self.root / 'dist' / name).exists())

    def test_second_writer_cannot_acquire_live_dist_lock(self):
        self.run_pipeline()
        dist = self.root / 'dist'
        with golden._dist_lock(dist):
            with self.assertRaises(OSError):
                with golden._dist_lock(dist):
                    self.fail('Second writer entered a live dist lock')

    def test_project_filename_is_separate_from_cryptquery_compatibility_filter(self):
        contract=json.loads((ROOT/'metadata/DIST_CONTRACT.json').read_text())
        header=(ROOT/'include/citrine_eclipse_mirage/platform/crypt_query.hpp').read_text()
        source=(ROOT/'src/platform/crypt_query.cpp').read_text()
        declaration=re.search(r'\btarget_suffix\s*=\s*u"([^"\n]+)"\s*;',header)
        self.assertIsNotNone(declaration)
        self.assertEqual(declaration.group(1),contract['hook11']['existing_filename_filter'])
        self.assertNotEqual(declaration.group(1),contract['runtime_dll_name'])
        self.assertEqual('citrine_eclipse_mirage.dll',contract['runtime_dll_name'])
        self.assertIn('target_suffix[j]',source)


class ToolCacheTests(unittest.TestCase):
    def cleanup_fixture(self, base):
        root = base / 'source'
        (root / 'scripts').mkdir(parents=True)
        (root / 'metadata').mkdir()
        (root / 'build').mkdir()
        (root / 'scripts/helper.py').write_text('value=1\n')
        victim = root / 'metadata/keep.json'
        victim.write_bytes(b'owned source sentinel')
        inventory.write_manifest(root)
        payload = root / 'scripts/__pycache__/helper.cpython-313.pyc'
        payload.parent.mkdir()
        payload.write_bytes(b'owned cache')
        command = [sys.executable, '-B', str(ROOT / 'scripts/clear_tool_python_cache.py'),
                   '--root', str(root), '--quiet', '--report']
        return root, victim, payload, command

    def test_cleanup_report_invalid_targets_preserve_sources_and_cache(self):
        with tempfile.TemporaryDirectory() as tmp:
            root, victim, payload, command = self.cleanup_fixture(Path(tmp))
            directory = root / 'build/directory'
            directory.mkdir()
            protected = {p: p.read_bytes() for p in
                         [victim, root / inventory.MANIFEST, root / 'scripts/helper.py']}
            for target in [victim, root / inventory.MANIFEST, root / 'scripts/helper.py',
                           root / 'build', directory, root / 'build/missing/report.json',
                           root.parent / 'outside.json', root / 'build/../metadata/keep.json',
                           Path('metadata/keep.json')]:
                with self.subTest(target=str(target)):
                    result = subprocess.run(command + [str(target)], cwd=root,
                                            capture_output=True, text=True)
                    self.assertNotEqual(0, result.returncode)
                    self.assertEqual('', result.stdout)
                    self.assertIn('TOOL_CACHE_CLEANUP: FAIL:', result.stderr)
                    self.assertTrue(payload.exists())
                    for path, before in protected.items():
                        self.assertEqual(before, path.read_bytes())
                    self.assertFalse(list((root / 'build').rglob('*.partial')))
            self.assertFalse((root.parent / 'outside.json').exists())

    def test_cleanup_report_linked_directories_cannot_reach_outside(self):
        import os
        for link_root in (True, False):
            with self.subTest(link_root=link_root), tempfile.TemporaryDirectory() as tmp:
                base = Path(tmp)
                root, victim, payload, command = self.cleanup_fixture(base)
                outside = base / 'outside'
                outside.mkdir()
                sentinel = outside / 'report.json'
                sentinel.write_bytes(b'outside sentinel')
                link = root / 'build' if link_root else root / 'build/redirect'
                if link_root:
                    link.rmdir()
                if os.name == 'nt':
                    made = subprocess.run(['cmd.exe', '/d', '/c', 'mklink', '/J',
                                           str(link), str(outside)], capture_output=True, text=True)
                    self.assertEqual(0, made.returncode, made.stdout + made.stderr)
                else:
                    link.symlink_to(outside, target_is_directory=True)
                try:
                    result = subprocess.run(command + [str(link / 'report.json')],
                                            capture_output=True, text=True)
                    self.assertNotEqual(0, result.returncode)
                    self.assertIn('TOOL_CACHE_CLEANUP: FAIL:', result.stderr)
                    self.assertTrue(payload.exists())
                    self.assertEqual(b'owned source sentinel', victim.read_bytes())
                    self.assertEqual(b'outside sentinel', sentinel.read_bytes())
                    self.assertEqual(['report.json'], sorted(p.name for p in outside.iterdir()))
                finally:
                    if os.name == 'nt':
                        os.rmdir(link)  # Remove the owned junction entry, never its target.
                    else:
                        link.unlink()
                self.assertEqual(b'outside sentinel', sentinel.read_bytes())

    def test_cleanup_report_atomic_replace_preserves_hardlinked_source(self):
        import os
        with tempfile.TemporaryDirectory() as tmp:
            root, victim, payload, command = self.cleanup_fixture(Path(tmp))
            report = root / 'build/report.json'
            os.link(victim, report)
            self.assertTrue(os.path.samefile(victim, report))
            result = subprocess.run(command + [str(report)], capture_output=True, text=True)
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual('', result.stdout)
            self.assertEqual(b'owned source sentinel', victim.read_bytes())
            self.assertFalse(os.path.samefile(victim, report))
            self.assertFalse(payload.exists())
            self.assertEqual(['scripts/__pycache__/helper.cpython-313.pyc'],
                             json.loads(report.read_text())['removed'])
            self.assertFalse(list(report.parent.glob('.*.partial')))

    def test_cleanup_report_replace_failure_preserves_previous_report(self):
        with tempfile.TemporaryDirectory() as tmp:
            report = Path(tmp) / 'report.json'
            report.write_bytes(b'previous complete report')
            output = '{"removed": []}\n'
            def fail_replace(source, target):
                self.assertEqual(report, target)
                self.assertEqual(b'previous complete report', report.read_bytes())
                self.assertEqual(output, Path(source).read_text(encoding='utf-8'))
                raise OSError('injected report publication failure')
            with patch.object(tool_cache.os, 'replace', side_effect=fail_replace):
                with self.assertRaisesRegex(OSError, 'injected report publication failure'):
                    tool_cache.write_report(report, output)
            self.assertEqual(b'previous complete report', report.read_bytes())
            self.assertEqual(['report.json'], sorted(p.name for p in report.parent.iterdir()))
            tool_cache.write_report(report, output)
            self.assertEqual(output, report.read_text())
            self.assertEqual(['report.json'], sorted(p.name for p in report.parent.iterdir()))

    def test_quiet_cleanup_keeps_report_and_propagates_failure(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            (root/'scripts').mkdir()
            (root/'metadata').mkdir()
            (root/'scripts/helper.py').write_text('value=1\n')
            inventory.write_manifest(root)
            cache=root/'scripts/__pycache__';cache.mkdir()
            (cache/'helper.cpython-313.pyc').write_bytes(b'fixture')
            report=root/'build/PYTHON_CACHE_CLEANUP.json';report.parent.mkdir()
            command=[sys.executable,'-B',str(ROOT/'scripts/clear_tool_python_cache.py'),
                     '--root',str(root),'--quiet','--report',str(report)]
            cp=subprocess.run(command,capture_output=True,text=True)
            self.assertEqual(0,cp.returncode,cp.stderr)
            self.assertEqual('',cp.stdout)
            self.assertEqual(['scripts/__pycache__/helper.cpython-313.pyc'],json.loads(report.read_text())['removed'])
            cp=subprocess.run(command,capture_output=True,text=True)
            self.assertEqual(0,cp.returncode,cp.stderr)
            self.assertEqual('',cp.stdout)
            self.assertEqual([],json.loads(report.read_text())['removed'])
            (root/inventory.MANIFEST).write_text('invalid json')
            cp=subprocess.run(command,capture_output=True,text=True)
            self.assertNotEqual(0,cp.returncode)
            self.assertIn('TOOL_CACHE_CLEANUP: FAIL:',cp.stderr)


    def test_cleanup_report_rejects_source_destinations_before_mutation(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'scripts').mkdir()
            (root / 'metadata').mkdir()
            (root / 'scripts/helper.py').write_text('value=1\n')
            victim = root / 'metadata/METADATA_OWNERSHIP.json'
            victim.write_bytes(b'original active record')
            inventory.write_manifest(root)
            baseline = victim.read_bytes()
            cache = root / 'scripts/__pycache__'
            cache.mkdir()
            payload = cache / 'helper.cpython-313.pyc'
            payload.write_bytes(b'fixture')

            command = [sys.executable, '-B', str(ROOT / 'scripts/clear_tool_python_cache.py'),
                       '--root', str(root), '--quiet', '--report', str(victim)]
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertNotEqual(0, result.returncode)
            self.assertIn('beneath source build/', result.stderr)
            self.assertEqual(baseline, victim.read_bytes())
            self.assertTrue(payload.exists())

            build = root / 'build'
            build.mkdir()
            linked = build / 'unsafe.json'
            try:
                linked.symlink_to(victim)
            except (OSError, NotImplementedError):
                pass
            else:
                command[-1] = str(linked)
                result = subprocess.run(command, capture_output=True, text=True)
                self.assertNotEqual(0, result.returncode)
                self.assertEqual(baseline, victim.read_bytes())
                self.assertTrue(payload.exists())

            report = build / 'PYTHON_CACHE_CLEANUP.json'
            command[-1] = str(report)
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(['scripts/__pycache__/helper.cpython-313.pyc'],
                             json.loads(report.read_text())['removed'])
            self.assertEqual(baseline, victim.read_bytes())
            self.assertFalse(list(build.glob('.*.partial')))

    def test_cleanup_removes_only_owned_generated_caches_without_refresh(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            (root/'metadata').mkdir()
            (root/'scripts').mkdir()
            (root/'scripts/helper.py').write_text('value=1\n')
            inventory.write_manifest(root)
            manifest=(root/inventory.MANIFEST).read_bytes()
            cache=root/'scripts/__pycache__';cache.mkdir()
            for name in ['helper.cpython-313.pyc','helper.cpython-312.opt-1.pyc','unowned.cpython-313.pyc','notes.txt']:
                (cache/name).write_bytes(b'fixture')
            removed=tool_cache.clear(root)
            self.assertEqual(['scripts/__pycache__/helper.cpython-312.opt-1.pyc','scripts/__pycache__/helper.cpython-313.pyc'],removed)
            self.assertEqual({'unowned.cpython-313.pyc','notes.txt'},{p.name for p in cache.iterdir()})
            self.assertEqual(manifest,(root/inventory.MANIFEST).read_bytes())
            with self.assertRaises(ValueError):inventory.verify_manifest(root)

    def test_corvexon_nested_children_reproduce_old_cache_and_keep_new_tree_clean(self):
        import os,shutil
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);scripts=root/'scripts';scripts.mkdir();(root/'metadata').mkdir()
            shutil.copyfile(ROOT/'scripts/golden_policy.py',scripts/'golden_policy.py')
            shutil.copyfile(ROOT/'scripts/project_version.py',scripts/'project_version.py')
            shutil.copyfile(ROOT/'metadata/CORVEXON_GOLDEN_TRAIL.json',root/'metadata/CORVEXON_GOLDEN_TRAIL.json')
            children=['verify_aryra_golden_layout.py','verify_mustavex_golden_metadata.py','check_msvc_shipping_link_command.py']
            for name in children:
                (scripts/name).write_text('import golden_policy\nfrom pathlib import Path\nPath(__file__).with_suffix(".ran").write_text("ran")\n')
            parent=scripts/'verify_corvexon_golden_trail.py'
            current=(ROOT/'scripts/verify_corvexon_golden_trail.py').read_text()
            self.assertEqual(3,current.count("[sys.executable,'-B',"))
            legacy=current.replace("[sys.executable,'-B',","[sys.executable,")
            dll=root/'invalid-synthetic.dll';dll.write_bytes(b'not a PE; failure after the three child processes is expected')
            env=dict(os.environ)
            env.pop('PYTHONDONTWRITEBYTECODE',None);env.pop('PYTHONPYCACHEPREFIX',None)
            for source,expect_cache in [(legacy,True),(current,False)]:
                parent.write_text(source)
                cache=scripts/'__pycache__'
                if cache.exists():shutil.rmtree(cache)
                cp=subprocess.run([sys.executable,'-B',str(parent),str(dll),'--build-dir',str(root/'build')],env=env,capture_output=True,text=True)
                self.assertEqual(1,cp.returncode,cp.stdout+cp.stderr)
                for name in children:self.assertTrue((scripts/name).with_suffix('.ran').is_file())
                self.assertEqual(expect_cache,cache.exists())
                for name in children:(scripts/name).with_suffix('.ran').unlink()


if __name__ == '__main__':
    unittest.main(verbosity=2)
