# SPDX-License-Identifier: GPL-3.0-only

"""Exact vanilla-engine staging, tamper and native-test configuration regressions.

Host configuration proves input/layout/target ownership, not native execution.
"""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import vendor_sync
import stage_mithrilhook as stage


class MithrilHookIdentity(unittest.TestCase):
    def temporary(self):
        parent = ROOT / 'build/qa'
        parent.mkdir(parents=True, exist_ok=True)
        return tempfile.TemporaryDirectory(prefix='mithril-stage-', dir=parent)

    def test_exact_unpatched_package_and_selection_agree(self):
        policy = json.loads((ROOT / 'metadata/HOOK_ENGINE.json').read_text())
        record = vendor_sync.load_lock(ROOT)['vendors']['mithrilhook']
        members = vendor_sync.expected_files(ROOT, record)
        self.assertTrue(vendor_sync.validate_installed(ROOT, 'mithrilhook', record, members, {}))
        self.assertEqual(policy['archive_sha256'], record['sha256'])
        self.assertEqual(policy['source_commit'], record['upstream_commit'])
        self.assertEqual(policy['hook_sha256'], members['src/hook.c'])
        self.assertFalse(policy['application_source_patches'])

    def test_actual_package_verifier_accepts_immutable_payload(self):
        completed = subprocess.run(
            [sys.executable, '-B', str(ROOT / 'third_party/mithrilhook/scripts/verify_sources.py')],
            capture_output=True, text=True, timeout=30)
        self.assertEqual(completed.returncode, 0, completed.stdout + completed.stderr)

    def test_staging_preserves_bytes_and_rejects_tamper_without_silent_repair(self):
        original = (ROOT / 'third_party/mithrilhook/src/hook.c').read_bytes()
        with self.temporary() as tmp:
            output = Path(tmp) / 'engine'
            stage.prepare(ROOT, output)
            self.assertEqual(len(stage.verify(ROOT, output)), 104)
            target = output / 'src/hook.c'
            before = target.stat().st_mtime_ns
            stage.prepare(ROOT, output)
            self.assertEqual(target.stat().st_mtime_ns, before)
            (output / 'build').mkdir()
            (output / 'build/generated.txt').write_text('generated output only')
            stage.verify(ROOT, output)
            target.write_bytes(original + b'X')
            for action in [stage.verify, stage.prepare]:
                with self.assertRaisesRegex(ValueError, 'member/hash mismatch'):
                    action(ROOT, output)
            self.assertEqual(target.read_bytes(), original + b'X')
            self.assertEqual((ROOT / 'third_party/mithrilhook/src/hook.c').read_bytes(), original)

    def test_stage_rejects_extra_source_and_outside_publication(self):
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp) / 'outside'
            with self.assertRaises(ValueError):
                stage.prepare(ROOT, output)
            self.assertFalse(output.exists())
        with self.temporary() as tmp:
            output = Path(tmp) / 'engine'
            stage.prepare(ROOT, output)
            (output / 'extra.c').write_text('unregistered source')
            with self.assertRaisesRegex(ValueError, 'member/hash mismatch'):
                stage.verify(ROOT, output)

    @unittest.skipUnless(shutil.which('cmake'), 'CMake required for exact native test inventory')
    def test_unmodified_native_test_project_configures_inside_its_own_build(self):
        # Configure the actual immutable native test CMake on the host without
        # compiling Windows code. This independently checks guard-compatible
        # layout and all executable/helper declarations against our strict policy.
        with self.temporary() as tmp:
            output = Path(tmp) / 'engine'
            stage.prepare(ROOT, output)
            harness = Path(tmp) / 'harness'
            harness.mkdir()
            (harness / 'CMakeLists.txt').write_text(
                'cmake_minimum_required(VERSION 3.20)\n'
                'project(engine_test_inventory LANGUAGES C CXX)\n'
                f'set(PROJECT_SOURCE_DIR "{output.as_posix()}")\n'
                f'set(Python3_EXECUTABLE "{Path(sys.executable).as_posix()}")\n'
                'set(MTH_HDE src/hde/hde64.c)\n'
                'add_library(mithrilhook STATIC "${PROJECT_SOURCE_DIR}/src/hook.c")\n'
                'target_include_directories(mithrilhook PUBLIC "${PROJECT_SOURCE_DIR}/include")\n'
                'enable_testing()\n'
                'add_subdirectory("${PROJECT_SOURCE_DIR}/tests" "${PROJECT_SOURCE_DIR}/build/tests")\n'
                'get_property(_targets DIRECTORY "${PROJECT_SOURCE_DIR}/tests" PROPERTY BUILDSYSTEM_TARGETS)\n'
                'set(_inventory "")\nforeach(_target IN LISTS _targets)\n'
                'get_target_property(_type "${_target}" TYPE)\n'
                'string(APPEND _inventory "${_target}\\t${_type}\\n")\nendforeach()\n'
                'file(WRITE "${CMAKE_BINARY_DIR}/inventory.txt" "${_inventory}")\n')
            command = ['cmake', '-S', str(harness), '-B', str(output / 'build')]
            if os.name == 'nt' and 'Visual Studio' in os.environ.get('CMAKE_GENERATOR', 'Visual Studio'):
                command += ['-A', 'x64']
            result = subprocess.run(command, capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            actual = dict(line.split('\t', 1) for line in
                          (output / 'build/inventory.txt').read_text().splitlines())
            policy = json.loads((ROOT / 'metadata/BUILD_PARTITION_POLICY.json').read_text())
            self.assertEqual(set(actual), set(policy['vendor_engine_test_targets']))
            self.assertEqual(actual['mithrilhook_control_unwind_engine'], 'STATIC_LIBRARY')
            stage.verify(ROOT, output)
            # The actual vendor generator also preserves the exact real-worker
            # stale-IP counterexample, rather than the obsolete Freeze argument.
            original = (output / 'src/hook.c').read_text()
            mutated = (output / 'build/tests/controls/stale.c').read_text()
            anchor = 'ip = hook->isEnabled ? FindNewIP(hook, desired) : FindOldIP(hook, desired);'
            self.assertEqual(original.count(anchor), 1)
            self.assertEqual(mutated, original.replace(anchor,
                'ip = hook->isEnabled ? FindNewIP(hook, desired) : 0;', 1))


if __name__ == '__main__':
    unittest.main()
