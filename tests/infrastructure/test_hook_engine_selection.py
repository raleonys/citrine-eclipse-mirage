# SPDX-License-Identifier: GPL-3.0-only

"""Exact selected-engine identity and single-disable counterfactual regressions.

Native suspension/completion/retirement tests live in the immutable package.
These host checks reject source drift; they do not claim native acceptance.
"""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import vendor_sync
import hook_real_peer_control as control


class MithrilHookIdentity(unittest.TestCase):
    def test_exact_unpatched_package_and_selection_agree(self):
        policy = json.loads((ROOT / 'metadata/HOOK_ENGINE.json').read_text())
        record = vendor_sync.load_lock(ROOT)['vendors']['mithrilhook']
        members = vendor_sync.expected_files(ROOT, record)
        self.assertTrue(vendor_sync.validate_installed(ROOT, 'mithrilhook', record, members, {}))
        self.assertEqual(policy['archive_sha256'], record['sha256'])
        self.assertEqual(policy['source_commit'], record['upstream_commit'])
        self.assertEqual(policy['hook_sha256'], members['src/hook.c'])
        self.assertEqual(policy['hook_sha256'], control.CURRENT_SHA256)
        self.assertFalse(policy['application_source_patches'])

    def test_actual_package_verifier_accepts_immutable_payload(self):
        completed = subprocess.run(
            [sys.executable, '-B', str(ROOT / 'third_party/mithrilhook/scripts/verify_sources.py')],
            capture_output=True, text=True, timeout=30)
        self.assertEqual(completed.returncode, 0, completed.stdout + completed.stderr)

    def test_stale_disable_requires_exact_input_and_one_known_action_change(self):
        source = (ROOT / 'third_party/mithrilhook/src/hook.c').read_bytes()
        stale = control.stale_source(source)
        self.assertEqual(hashlib.sha256(stale).hexdigest(), control.STALE_SHA256)
        self.assertEqual(stale, source.replace(control.ANCHOR, control.STALE, 1))
        for bad in (source + b'X', stale, source.replace(control.ANCHOR, control.STALE)):
            with self.subTest(digest=hashlib.sha256(bad).hexdigest()):
                with self.assertRaises(ValueError):
                    control.stale_source(bad)

    def test_negative_control_cannot_publish_outside_owned_build(self):
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp) / 'hook.c'
            with self.assertRaises(ValueError):
                control.prepare(ROOT / 'third_party/mithrilhook/src/hook.c', output)
            self.assertFalse(output.exists())


if __name__ == '__main__':
    unittest.main()
