# SPDX-License-Identifier: GPL-3.0-only

"""Tamper, failure-propagation and source/package identity regressions."""
from pathlib import Path
import json
import os
import re
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
from golden_policy import load_target, PolicyPending


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
        (self.build / 'LOCAL_BUILD_RECEIPT.json').write_text(json.dumps(self.receipt), encoding='utf-8')
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

    def test_missing_ambiguous_relative_and_changed_cache_are_rejected(self):
        original = self.cache.read_text(encoding='utf-8')
        for text in ('', original + original, 'CMAKE_CXX_COMPILER:FILEPATH=cl.exe\n'):
            with self.subTest(cache=text):
                self.cache.write_text(text, encoding='utf-8')
                self.receipt['cmake_cache_sha256'] = msvc_evidence.sha(self.cache)
                self.assertTrue(self.errors())
        self.cache.write_text(original, encoding='utf-8')
        self.assertIn('Build identity mismatch: cmake_cache_sha256', self.errors())


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
        for namespace,accepted in [('preloader_l::module',True),('upstream::module',False)]:
            payload=('namespace '+namespace+' {}\n').encode('utf-8');path.write_bytes(payload)
            (self.root/'metadata/SOURCE_FILES.json').write_text(json.dumps({'files':{'src/module.cpp':{'sha256':inventory.digest(payload)}}}))
            self.assertEqual(accepted,source_verification.verify(self.root)['status']=='PASS')


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

    def test_source_drift_is_not_automatically_legitimized(self):
        (self.root / 'src/value.cpp').write_text('changed')
        with self.assertRaises(ValueError):
            packaging.package(self.root, self.zip)
        self.assertFalse(self.zip.exists())

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

    def test_duplicate_package_member(self):
        packaging.package(self.root, self.zip)
        with zipfile.ZipFile(self.zip, 'a') as archive:
            import warnings
            with warnings.catch_warnings():
                warnings.simplefilter('ignore', UserWarning)
                archive.writestr('src/value.cpp', 'duplicate')
        self.assertTrue(any('Duplicate' in e for e in falcion.check(self.root, self.zip)))


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
                (root / 'metadata' / name).write_text(json.dumps({'activation':'READY_LOCAL_TARGET'}))
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
            for token in ['%*', 'exit /b %RC%', 'ForegroundColor Red', 'Exit code: %RC%', 'pause >nul']:
                path.write_text(original.replace(token, 'REMOVED'))
                self.assertTrue(launchers.check(root), token)
            path.write_text(original)
            automation=root/entry['automation']
            automation.write_text(automation.read_text().replace('exit $LASTEXITCODE','exit 0'))
            self.assertTrue(any('masks child exit' in e for e in launchers.check(root)))


class LocalDistTests(unittest.TestCase):
    """Synthetic process results test orchestration only; never stand in for MSVC evidence."""
    def setUp(self):
        import shutil
        self.temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)/'source'
        (self.root/'metadata').mkdir(parents=True)
        (self.root/'metadata/ACTIVE_SOURCE_MANIFEST.json').write_text('{"fixture_only":true,"files":{}}')
        shutil.copyfile(ROOT/'metadata/LOCAL_DIST_CONTRACT.json',self.root/'metadata/LOCAL_DIST_CONTRACT.json')
        self.build=self.root/'build/golden'
        self.build.mkdir(parents=True)
        self.raw=self.build/'preloader.unsigned.dll'
        self.raw.write_bytes(b'synthetic orchestration fixture, not a PE or MSVC result')
        self.output=self.build/'golden-local'

    def run_pipeline(self,failed_stage=None,runtime_failure=False,corrupt_runtime=False):
        calls=[]
        def run(command,**kwargs):
            self.assertEqual('1',kwargs['env']['PYTHONDONTWRITEBYTECODE'])
            index=len(calls);calls.append(command)
            if index==6:
                self.assertTrue(command[0].endswith('preloader_l_runtime_host.exe'))
                self.assertEqual('preloader_l.dll',Path(command[-1]).name)
                if corrupt_runtime:Path(command[-1]).write_bytes(b'changed during runtime')
            return subprocess.CompletedProcess(command,37 if index==failed_stage or (index==6 and runtime_failure) else 0,'synthetic process result\n','')
        with patch.object(golden,'pending_targets',return_value=[]),patch.object(golden.subprocess,'run',side_effect=run):
            report=golden.pipeline(self.root,self.raw,self.build,self.output)
        return report,calls

    def dist_bytes(self):
        return {p.name:p.read_bytes() for p in (self.root/'dist').iterdir() if p.is_file()}

    def test_success_stages_exact_final_name_hash_receipt_and_original_bytes(self):
        before=self.raw.read_bytes()
        report,calls=self.run_pipeline()
        self.assertEqual('LOCAL_GOLDEN_PASS',report['result'])
        self.assertEqual(7,len(calls))
        target=self.root/'dist/preloader_l.dll'
        self.assertEqual(before,target.read_bytes())
        self.assertEqual(before,self.raw.read_bytes())
        self.assertEqual(golden.sha(target)+'  preloader_l.dll\n',Path(report['dist_sha256_file']).read_text())
        receipt=json.loads(Path(report['dist_receipt']).read_text())
        self.assertEqual(report['sha256'],receipt['sha256'])
        self.assertEqual(golden.sha(self.root/'metadata/ACTIVE_SOURCE_MANIFEST.json'),receipt['source_manifest_sha256'])
        self.assertFalse(receipt['official_release'])
        self.assertFalse(receipt['library_publication'])

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
            golden.stage_local_dist(self.root,self.raw,{'result':'LOCAL_GOLDEN_PASS','steps':[]})
        self.assertFalse((self.root/'dist').exists())

    def test_replace_failure_restores_previous_bundle(self):
        report,_=self.run_pipeline()
        before=self.dist_bytes()
        artifact=Path(report['artifact']);artifact.write_bytes(b'new synthetic candidate')
        report['sha256']=golden.sha(artifact)
        report['runtime_check'].update(sha256_before=report['sha256'],sha256_after=report['sha256'])
        replace=golden.os.replace
        def fail_sidecar(source,target):
            if Path(source).name=='preloader_l.dll.sha256' and Path(source).parent.name.startswith('.candidate-'):
                raise OSError('injected sidecar commit failure')
            return replace(source,target)
        with patch.object(golden.os,'replace',side_effect=fail_sidecar):
            with self.assertRaises(OSError):golden.stage_local_dist(self.root,artifact,report)
        self.assertEqual(before,self.dist_bytes())
        self.assertFalse((self.root/'dist/.local-golden.lock').exists())
        self.assertFalse(list((self.root/'dist').glob('.candidate-*')))

    def test_final_filename_matches_unchanged_hook_filter(self):
        contract=json.loads((ROOT/'metadata/LOCAL_DIST_CONTRACT.json').read_text())
        header=(ROOT/'include/preloader_l/platform/crypt_query.hpp').read_text()
        source=(ROOT/'src/platform/crypt_query.cpp').read_text()
        declaration=re.search(r'\btarget_suffix\s*=\s*u"([^"\n]+)"\s*;',header)
        self.assertIsNotNone(declaration)
        self.assertEqual(declaration.group(1),contract['runtime_dll_name'])
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
