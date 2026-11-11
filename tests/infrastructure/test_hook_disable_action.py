# SPDX-License-Identifier: GPL-3.0-only

"""Source-linked single-hook disable action regression for the exact owned MinHook copy.

The test runs the actual CMake build-copy transform on the immutable vendored
hook.c. It does not simulate Windows peer suspension or assert native acceptance.
"""
import hashlib
import json
import re
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
POLICY = ROOT / 'metadata/HOOK_ENGINE_PATCH.json'
PATCH = ROOT / 'cmake/PatchHookFreeze.cmake'
DONOR = ROOT / 'third_party/minhook/src/hook.c'
PUBLIC_ENTRIES = (
    b'MH_STATUS WINAPI MH_CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal)',
    b'MH_STATUS WINAPI MH_RemoveHook(LPVOID pTarget)',
    b'static MH_STATUS EnableHook(LPVOID pTarget, BOOL enable)',
    b'static MH_STATUS QueueHook(LPVOID pTarget, BOOL queueEnable)',
    b'MH_STATUS WINAPI MH_ApplyQueued(VOID)',
)


class SingleHookDisableIP(unittest.TestCase):
    def test_missing_or_duplicate_public_cleanup_entry_fails_without_writing(self):
        source = DONOR.read_bytes()
        for signature in PUBLIC_ENTRIES:
            with self.subTest(signature=signature):
                self.assertEqual(source.count(signature), 1)
                self.assert_rejected_anchor(source.replace(signature, signature.replace(b'(', b'_missing(', 1), 1))
                self.assert_rejected_anchor(source + b'\n/* ' + signature + b' */\n')

    def test_missing_or_duplicate_public_cleanup_lock_fails_without_writing(self):
        source = DONOR.read_bytes()
        lock = b'    EnterSpinLock();'
        for signature in PUBLIC_ENTRIES:
            start = source.index(signature)
            end = source.index(b'\n}', start)
            body = source[start:end]
            self.assertEqual(body.count(lock), 1)
            with self.subTest(signature=signature):
                for changed in (body.replace(lock, b'    EnterSpinLock( );', 1),
                                body.replace(lock, lock + b'\n' + lock, 1)):
                    self.assert_rejected_anchor(source[:start] + changed + source[end:])

    def assert_rejected_anchor(self, source):
        executable = shutil.which('cmake')
        self.assertIsNotNone(executable, 'CMake is required for the source QA gate')
        with tempfile.TemporaryDirectory() as temp:
            target = Path(temp) / 'hook.c'
            target.write_bytes(source)
            result = subprocess.run([executable, '-DMINHOOK_HOOK_C=' + str(target),
                                     '-P', str(PATCH)], capture_output=True, text=True, timeout=30)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('anchor must occur exactly once', result.stderr)
            self.assertEqual(target.read_bytes(), source, 'Rejected transform must not write partial output')

    def test_missing_single_hook_anchor_fails_closed(self):
        source = DONOR.read_bytes()
        anchor = b'status = Freeze(&threads, pos, ACTION_ENABLE);'
        self.assertEqual(source.count(anchor), 1)
        self.assert_rejected_anchor(source.replace(anchor, b'status = Freeze( &threads, pos, ACTION_ENABLE);'))

    def test_duplicate_single_hook_anchor_fails_closed(self):
        source = DONOR.read_bytes()
        self.assert_rejected_anchor(source + b'\n/* status = Freeze(&threads, pos, ACTION_ENABLE); */\n')

    def test_exact_pinned_hook_copy_preserves_disable_ip_action(self):
        policy = json.loads(POLICY.read_text(encoding='utf-8'))
        original = DONOR.read_bytes()
        self.assertEqual(hashlib.sha256(original).hexdigest(), policy['input_hook_sha256'])
        self.assertEqual(hashlib.sha256(PATCH.read_bytes()).hexdigest(),
                         policy['patch_script_sha256'])
        executable = shutil.which('cmake')
        self.assertIsNotNone(executable, 'CMake is required for the source QA gate')
        with tempfile.TemporaryDirectory() as temp:
            target = Path(temp) / 'hook.c'
            target.write_bytes(original)
            subprocess.run([executable, '-DMINHOOK_HOOK_C=' + str(target),
                            '-P', str(PATCH)], check=True, capture_output=True, timeout=30)
            result = target.read_bytes().replace(b'\r\n', b'\n')
        self.assertEqual(hashlib.sha256(result).hexdigest(), policy['output_hook_sha256'])
        self.assertEqual(DONOR.read_bytes(), original, 'Vendored source must not change')
        rendered = result.decode('utf-8-sig')
        entry = rendered.split('static MH_STATUS EnableHook(LPVOID pTarget, BOOL enable)', 1)[1]
        entry = entry.split('MH_STATUS WINAPI MH_EnableHook(LPVOID pTarget)', 1)[0]
        self.assertEqual(entry.count('status = Freeze(&threads, pos, enable ? ACTION_ENABLE : ACTION_DISABLE);'), 1)
        self.assertNotIn('status = Freeze(&threads, pos, ACTION_ENABLE);', entry)
        self.assertIn('return EnableHook(pTarget, FALSE);',
                      rendered.split('MH_STATUS WINAPI MH_DisableHook(LPVOID pTarget)', 1)[1])
        mapping = rendered.split('static PEER_PREPARATION_RESULT ProcessThreadIPs', 1)[1]
        mapping = mapping.split('static BOOL EnumerateThreads', 1)[0]
        self.assertRegex(mapping, r'case ACTION_DISABLE:\s+enable = FALSE;')
        self.assertIn('ip = FindOldIP(pHook, *pIP);', mapping)
        # A reverted always-enable call must fail exact output identity.
        bad = result.replace(
            b'status = Freeze(&threads, pos, enable ? ACTION_ENABLE : ACTION_DISABLE);',
            b'status = Freeze(&threads, pos, ACTION_ENABLE);')
        self.assertNotEqual(bad, result)
        self.assertNotEqual(hashlib.sha256(bad).hexdigest(), policy['output_hook_sha256'])


if __name__ == '__main__':
    unittest.main()
