"""Original SURGE directive guards adapted to the current authorized workstream."""
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
from check_integration_requirements import check


class RequirementsTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        for name in ['metadata/INTEGRATION_REQUIREMENTS.json','metadata/BRANDING_AND_IDENTITY.json',
                     'metadata/LOCAL_DATA_POLICY.json','metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json',
                     'metadata/RUNTIME_CURRENT_STATUS.json','metadata/PUBLIC_DLL_ABI.json',
                     'resources/branding/surge.png']:
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT / name, path)

    def alter(self, name, edit):
        path = self.root / name
        data = json.loads(path.read_text())
        edit(data)
        path.write_text(json.dumps(data))

    def test_current_target_contract(self):
        self.assertEqual([], check(self.root))

    def test_current_runtime_contract_must_exist(self):
        (self.root / 'metadata/RUNTIME_CURRENT_STATUS.json').unlink()
        self.assertIn('CURRENT_RUNTIME_STATE_MISSING', check(self.root))

    def test_current_exports_match_authoritative_abi(self):
        self.alter('metadata/RUNTIME_CURRENT_STATUS.json',lambda d:d['exports'].update(DllWaitReady=4))
        self.assertIn('CURRENT_RUNTIME_ABI_MISMATCH',check(self.root))

    def test_current_vendor_bindings_match_owner_requirements(self):
        self.alter('metadata/RUNTIME_CURRENT_STATUS.json',lambda d:d['active_vendors'].update(filesystem='ghc'))
        self.assertIn('CURRENT_RUNTIME_VENDOR_MISMATCH',check(self.root))

    def test_current_log_policy_cannot_contradict_release_policy(self):
        self.alter('metadata/RUNTIME_CURRENT_STATUS.json',lambda d:d['logging'].update(release_file_logging='ALLOWED'))
        self.assertIn('CURRENT_RUNTIME_LOG_POLICY_MISMATCH',check(self.root))

    def test_infrastructure_scope_cannot_shrink_to_build_helpers(self):
        self.alter('metadata/INTEGRATION_REQUIREMENTS.json', lambda d: d['infrastructure'].pop('golden_tools'))
        self.assertIn('INFRASTRUCTURE_SCOPE_CHANGED', check(self.root))

    def test_vendor_mandate_cannot_disappear_while_integration_is_pending(self):
        self.alter('metadata/INTEGRATION_REQUIREMENTS.json', lambda d: d['third_party'].update(pegtl='OPTIONAL'))
        self.assertIn('MANDATORY_VENDOR_POLICY_WEAKENED', check(self.root))

    def test_comparison_role_cannot_become_code_donor(self):
        self.alter('metadata/INTEGRATION_REQUIREMENTS.json', lambda d: d.update(redfield='IMPLEMENTATION_SOURCE'))
        self.assertIn('FROZEN_COMPARISON_ROLE_CHANGED redfield', check(self.root))

    def test_library_sync_does_not_follow_build(self):
        self.alter('metadata/INTEGRATION_REQUIREMENTS.json', lambda d: d.update(library_synchronization='AUTOMATIC'))
        self.assertIn('LIBRARY_SCOPE_CHANGED', check(self.root))

    def test_golden_order_is_preserved(self):
        self.alter('metadata/INTEGRATION_REQUIREMENTS.json', lambda d: d['golden_pipeline_order'].reverse())
        self.assertIn('GOLDEN_ORDER_CHANGED', check(self.root))

    def test_surge_identity_and_exact_asset(self):
        (self.root / 'resources/branding/surge.png').write_bytes(b'tampered')
        self.assertIn('OWNER_AVATAR_CHANGED', check(self.root))

    def test_namespace_cannot_be_renamed_with_display_identity(self):
        self.alter('metadata/BRANDING_AND_IDENTITY.json', lambda d: d.update(cpp_root_namespace='SURGE'))
        self.assertIn('SURGE_IDENTITY_CHANGED', check(self.root))

    def test_historical_runtime_snapshot_cannot_replace_current_contract(self):
        self.alter('metadata/RUNTIME_CURRENT_STATUS.json', lambda d: d.update(role='HISTORICAL_SNAPSHOT'))
        self.assertIn('CURRENT_RUNTIME_SNAPSHOT_STALE', check(self.root))

    def test_release_log_remains_forbidden(self):
        self.alter('metadata/LOCAL_DATA_POLICY.json', lambda d: d.update(release_file_logging='ALLOWED'))
        self.assertIn('RELEASE_LOG_POLICY_WEAKENED', check(self.root))

    def test_transfer_matrix_cannot_drop_a_category(self):
        self.alter('metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json', lambda d: d['requirements'].pop())
        self.assertIn('INFRASTRUCTURE_TRANSFER_SCOPE_LOST', check(self.root))


if __name__ == '__main__':
    unittest.main(verbosity=2)
