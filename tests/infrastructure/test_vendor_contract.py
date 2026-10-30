"""Archive-pin, tamper and symlink-preserving vendoring/package regressions."""
from pathlib import Path
import json
import shutil
import stat
import sys
import tempfile
import unittest
import zipfile
from unittest.mock import patch
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'scripts'))
import vendor_sync
import project_files
import package_source
import extract_source
import restore_vendor_links
import prepare_hook_engine


class VendorTests(unittest.TestCase):
    def test_archive_comment_commit_is_bound_to_the_pinned_payload(self):
        record=vendor_sync.load_lock(ROOT)['vendors']['minhook'].copy()
        expected=vendor_sync.expected_files(ROOT,record)
        with tempfile.TemporaryDirectory() as tmp:
            path=Path(tmp);archive=path/Path(record['archive']).name
            with zipfile.ZipFile(archive,'w') as out:
                out.comment=record['upstream_commit'].encode('ascii')
                for rel in expected:
                    info=zipfile.ZipInfo(record['archive_folder']+rel)
                    info.external_attr=0o100644<<16
                    out.writestr(info,(ROOT/record['source_install_path']/rel).read_bytes())
            record.update(sha256=vendor_sync.hash_file(archive),bytes=archive.stat().st_size)
            self.assertEqual(expected,vendor_sync.inspect_archive(ROOT,'minhook',record,path)[0])
            record['upstream_commit']='0'*40
            with self.assertRaisesRegex(ValueError,'archive commit comment mismatch'):
                vendor_sync.inspect_archive(ROOT,'minhook',record,path)

    def test_build_copy_patch_is_hash_pinned_and_original_tree_unchanged(self):
        cmake=shutil.which('cmake')
        if not cmake:self.skipTest('CMake needed to execute the real patch script')
        record=vendor_sync.load_lock(ROOT)['vendors']['minhook']
        original=ROOT/record['source_install_path']/'src/hook.c'
        before=original.read_bytes()
        parent=ROOT/'build/qa';parent.mkdir(parents=True,exist_ok=True)
        with tempfile.TemporaryDirectory(dir=parent,prefix='hook-copy-') as tmp:
            stage=Path(tmp)/'engine'
            prepare_hook_engine.prepare(ROOT,stage,cmake)
            policy=prepare_hook_engine.verify(ROOT,stage)
            self.assertEqual(policy['output_hook_sha256'],vendor_sync.hash_file(stage/'src/hook.c'))
            self.assertNotEqual(before,(stage/'src/hook.c').read_bytes())
            for rel in ['src/hook.c','include/MinHook.h']:
                target=stage/rel;data=target.read_bytes();target.write_bytes(data+b'X')
                with self.assertRaisesRegex(ValueError,'build-copy member/hash mismatch'):
                    prepare_hook_engine.verify(ROOT,stage)
                target.write_bytes(data)
            self.assertEqual(before,original.read_bytes())
            stamp=(stage/'src/hook.c').stat().st_mtime_ns
            prepare_hook_engine.prepare(ROOT,stage,cmake)
            self.assertEqual(stamp,(stage/'src/hook.c').stat().st_mtime_ns)

    def test_windows_cmake_crlf_output_is_pinned_lf_before_full_verification(self):
        cmake=shutil.which('cmake')
        if not cmake:self.skipTest('CMake needed to execute the real patch script')
        real_run=prepare_hook_engine.subprocess.run
        record=vendor_sync.load_lock(ROOT)['vendors']['minhook']
        original=ROOT/record['source_install_path']/'src/hook.c'
        before=original.read_bytes()
        def windows_write(command, **kwargs):
            result=real_run(command, **kwargs)
            target=Path(next(v.split('=',1)[1] for v in command if v.startswith('-DMINHOOK_HOOK_C=')))
            target.write_bytes(target.read_bytes().replace(b'\r\n',b'\n').replace(b'\n',b'\r\n'))
            return result
        parent=ROOT/'build/qa';parent.mkdir(parents=True,exist_ok=True)
        with tempfile.TemporaryDirectory(dir=parent,prefix='hook-crlf-') as tmp:
            stage=Path(tmp)/'engine'
            with patch.object(prepare_hook_engine.subprocess,'run',side_effect=windows_write):
                prepare_hook_engine.prepare(ROOT,stage,cmake)
            policy=prepare_hook_engine.verify(ROOT,stage)
            target=stage/'src/hook.c';generated=target.read_bytes()
            self.assertNotIn(b'\r',generated)
            self.assertEqual(policy['output_hook_sha256'],vendor_sync.hash_file(target))
            self.assertEqual(before,original.read_bytes())
            # Read-only verification never accepts CRLF as an alternative hash.
            target.write_bytes(generated.replace(b'\n',b'\r\n'))
            with self.assertRaisesRegex(ValueError,'build-copy member/hash mismatch') as error:
                prepare_hook_engine.verify(ROOT,stage)
            self.assertIn('src/hook.c',str(error.exception))
            self.assertIn('expected_sha256',str(error.exception))
            target.write_bytes(generated)
            def changed_windows_write(command, **kwargs):
                result=windows_write(command, **kwargs)
                changed=Path(next(v.split('=',1)[1] for v in command if v.startswith('-DMINHOOK_HOOK_C=')))
                changed.write_bytes(changed.read_bytes()+b'X')
                return result
            with patch.object(prepare_hook_engine.subprocess,'run',side_effect=changed_windows_write):
                with self.assertRaisesRegex(ValueError,'build-copy member/hash mismatch'):
                    prepare_hook_engine.prepare(ROOT,stage,cmake)
            self.assertEqual(generated,target.read_bytes())
            self.assertEqual(before,original.read_bytes())

    def test_newline_canonicalization_cannot_accept_changed_content(self):
        policy=json.loads((ROOT/'metadata/HOOK_ENGINE_PATCH.json').read_text())
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/'src').mkdir()
            target=root/'src/hook.c'
            for bad in [b'changed\r\n',b'changed\n',b'changed\r\r\n']:
                target.write_bytes(bad)
                prepare_hook_engine.canonicalize_generated_hook(root,policy['output_hook_sha256'])
                self.assertEqual(bad,target.read_bytes())

    def test_all_active_vendors_match_owner_pins_and_member_hashes(self):
        lock=vendor_sync.load_lock(ROOT)
        inputs=json.loads((ROOT/'metadata/SOURCE_INPUTS.json').read_text())['next_phase_vendor_archives']
        for name,record in lock['vendors'].items():
            self.assertEqual(inputs[Path(record['archive']).name]['sha256'],record['sha256'])
            files=vendor_sync.expected_files(ROOT,record)
            self.assertTrue(vendor_sync.validate_installed(ROOT,name,record,files,record.get('symlinks',{})))

    def test_wrong_archive_is_rejected_before_install(self):
        lock=vendor_sync.load_lock(ROOT);record=lock['vendors']['minhook']
        with tempfile.TemporaryDirectory() as tmp:
            path=Path(tmp);(path/Path(record['archive']).name).write_bytes(b'wrong archive')
            with self.assertRaisesRegex(ValueError,'archive SHA/size mismatch'):
                vendor_sync.inspect_archive(ROOT,'minhook',record,path)

    def test_member_manifest_mutation_is_rejected(self):
        record=vendor_sync.load_lock(ROOT)['vendors']['minhook']
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);manifest=root/record['member_manifest'];manifest.parent.mkdir(parents=True)
            manifest.write_text('mutated manifest')
            with self.assertRaisesRegex(ValueError,'member manifest hash mismatch'):
                vendor_sync.expected_files(root,record)

    def test_installed_single_byte_mutation_is_rejected(self):
        record=vendor_sync.load_lock(ROOT)['vendors']['minhook']
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);destination=root/record['source_install_path']
            shutil.copytree(ROOT/record['source_install_path'],destination)
            files=vendor_sync.expected_files(ROOT,record)
            header=destination/'include/MinHook.h';header.write_bytes(header.read_bytes()+b'X')
            with self.assertRaisesRegex(ValueError,'member set/hash mismatch'):
                vendor_sync.validate_installed(root,'minhook',record,files,{})

    def test_installer_refuses_overwrite(self):
        record=vendor_sync.load_lock(ROOT)['vendors']['minhook']
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);destination=root/record['source_install_path'];destination.mkdir(parents=True)
            marker=destination/'keep';marker.write_bytes(b'owned input')
            with self.assertRaisesRegex(ValueError,'refusing overwrite'):
                vendor_sync.install(root,'minhook',record,{}, {})
            self.assertEqual(b'owned input',marker.read_bytes())

    def test_asio_same_count_damage_identifies_exact_alias_paths(self):
        record=vendor_sync.load_lock(ROOT)['vendors']['asio']
        expected=vendor_sync.expected_files(ROOT,record);links=record['symlinks']
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);destination=root/record['source_install_path']
            shutil.copytree(ROOT/record['source_install_path'],destination,
                            ignore=lambda folder,names:[n for n in names if (Path(folder)/n).is_symlink()])
            for rel,target in links.items():
                path=destination/rel;path.parent.mkdir(parents=True,exist_ok=True)
                path.write_bytes(target.encode('utf8'))
            self.assertTrue(vendor_sync.validate_installed(root,'asio',record,expected,links))
            for rel in links:(destination/rel).write_bytes(b'')
            with self.assertRaises(ValueError) as error:
                vendor_sync.validate_installed(root,'asio',record,expected,links)
            detail=str(error.exception)
            self.assertIn('1607/1607',detail)
            self.assertIn('changed=2',detail)
            for rel in links:self.assertIn('changed: '+rel,detail)

    def test_member_and_link_escape_rejected(self):
        for name in ['../escape','/absolute','C:/drive','a/../b','a\\b','a//b']:
            with self.subTest(name=name),self.assertRaises(ValueError):vendor_sync.safe_rel(name)
        for target in ['../../../escape','/absolute','C:drive','a\\b','bad\nname']:
            with self.subTest(target=target),self.assertRaises(ValueError):vendor_sync.validate_symlink('asio/include',target)
        self.assertTrue(vendor_sync.validate_symlink('asio/include','../include'))


class LinkPackagingTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.base=Path(self.temp.name);self.root=self.base/'source'
        (self.root/'metadata').mkdir(parents=True)
        (self.root/'third_party/asio/asio').mkdir(parents=True)
        (self.root/'third_party/asio/include').mkdir()
        (self.root/'third_party/asio/include/header.hpp').write_bytes(b'unchanged vendor fixture')
        (self.root/'CMakeLists.txt').write_text('source fixture')
        lock={'vendors':{'asio':{'source_install_path':'third_party/asio','symlinks':{'asio/include':'../include'}}}}
        (self.root/'metadata/VENDOR_LOCK.json').write_text(json.dumps(lock))
        contract={'required_files':['CMakeLists.txt'],'source_directories':['metadata','third_party']}
        (self.root/'metadata/REPOSITORY_CONTRACT.json').write_text(json.dumps(contract))
        self.link=self.root/'third_party/asio/asio/include'
        self.link.write_bytes(b'../include')
        project_files.write_manifest(self.root)
        self.archive=self.base/'source.zip'

    def test_registered_alias_roundtrip_uses_portable_exact_descriptor(self):
        package_source.package(self.root,self.archive)
        with zipfile.ZipFile(self.archive) as archive:
            info=archive.getinfo('third_party/asio/asio/include')
            self.assertTrue(stat.S_ISREG(info.external_attr>>16))
            self.assertEqual(b'../include',archive.read(info))
        destination=self.base/'restored'
        with patch.object(extract_source.os,'symlink',side_effect=OSError('No Windows symlink privilege')) as create_link:
            extract_source.extract(self.archive,destination)
            create_link.assert_not_called()
        restored=destination/'third_party/asio/asio/include'
        self.assertFalse(restored.is_symlink())
        self.assertEqual(b'../include',restored.read_bytes())
        self.assertEqual(b'unchanged vendor fixture',(destination/'third_party/asio/include/header.hpp').read_bytes())
        self.assertEqual([],project_files.verify_manifest(destination))

    def test_unexpected_symlink_type_in_portable_archive_is_rejected(self):
        package_source.package(self.root,self.archive)
        with zipfile.ZipFile(self.archive) as archive:
            members=[(i,archive.read(i)) for i in archive.infolist()]
        with zipfile.ZipFile(self.archive,'w') as archive:
            for info,data in members:
                if info.filename=='third_party/asio/asio/include':info.external_attr=0o120777<<16
                archive.writestr(info,data)
        destination=self.base/'rejected'
        with self.assertRaisesRegex(ValueError,'source/link type mismatch'):
            extract_source.extract(self.archive,destination)
        self.assertFalse(destination.exists())

    def test_restore_only_exact_descriptors(self):
        self.link.unlink();self.link.write_bytes(b'../include')
        self.assertEqual(b'../include',project_files.source_files(self.root)['third_party/asio/asio/include'])
        try:restore_vendor_links.restore(self.root)
        except (OSError,NotImplementedError):self.skipTest('Optional restoration to a real symlink requires OS privilege')
        self.assertTrue(self.link.is_symlink())
        self.link.unlink();self.link.write_bytes(b'actual user file')
        with self.assertRaisesRegex(ValueError,'non-descriptor'):restore_vendor_links.restore(self.root)
        self.assertEqual(b'actual user file',self.link.read_bytes())

    def test_standard_zip_extraction_needs_no_link_support(self):
        package_source.package(self.root,self.archive)
        destination=self.base/'standard-unzip'
        with zipfile.ZipFile(self.archive) as archive:archive.extractall(destination)
        self.assertEqual([],project_files.verify_manifest(destination))
        self.assertFalse((destination/'third_party/asio/asio/include').is_symlink())

    def test_descriptor_single_byte_mutation_is_rejected(self):
        self.link.unlink();self.link.write_bytes(b'../include\n')
        with self.assertRaisesRegex(ValueError,'altered source symlink/descriptor'):
            project_files.source_files(self.root)

    def test_descriptor_directory_cannot_replace_locked_alias(self):
        self.link.unlink();self.link.mkdir()
        with self.assertRaisesRegex(ValueError,'invalid locked vendor alias'):
            project_files.source_files(self.root)

    def test_changed_link_target_is_rejected(self):
        self.link.unlink()
        try:self.link.symlink_to('../../../../outside',target_is_directory=True)
        except (OSError,NotImplementedError):self.skipTest('Host cannot create altered symlink fixture')
        with self.assertRaisesRegex(ValueError,'altered source symlink'):project_files.source_files(self.root)

    def test_extractor_never_overwrites_existing_destination(self):
        destination=self.base/'existing';destination.mkdir()
        marker=destination/'keep';marker.write_bytes(b'keep')
        with self.assertRaisesRegex(ValueError,'refusing overwrite'):
            extract_source.extract(self.archive,destination)
        self.assertEqual(b'keep',marker.read_bytes())


if __name__=='__main__':unittest.main(verbosity=2)
