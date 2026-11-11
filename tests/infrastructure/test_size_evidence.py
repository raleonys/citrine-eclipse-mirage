# SPDX-License-Identifier: GPL-3.0-only

"""COFF evidence reader rejects malformed input and preserves object-size meaning."""
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import collect_size_evidence as size_evidence
from collect_size_evidence import coff_sections, collect, current_objects, vs_object_paths


def object_bytes(big=False, name=b'.text', payload=b'\x90\x90\xc3', flags=0x60001020):
    header = bytearray(56 if big else 20)
    if big:
        struct.pack_into('<HHHHI', header, 0, 0, 0xffff, 2, 0x8664, 0)
        header[12:28] = bytes.fromhex('c7a1bad1eebaa94baf20faf66aa4dcb8')
        struct.pack_into('<I', header, 44, 1)
    else:
        struct.pack_into('<HH', header, 0, 0x8664, 1)
    section = bytearray(40)
    section[:len(name)] = name
    struct.pack_into('<II', section, 16, len(payload), len(header) + 40)
    struct.pack_into('<I', section, 36, flags)
    return header + section + payload


class CoffEvidenceTests(unittest.TestCase):
    def test_real_code_bytes_and_comdat_are_distinct_from_file_size(self):
        data = object_bytes()
        result = coff_sections(data)
        self.assertEqual(result['machine'], '0x8664')
        self.assertEqual(result['sections'], [{'name': '.text', 'raw_bytes': 3, 'flags': 0x60001020, 'comdat': True}])
        self.assertGreater(len(data), result['sections'][0]['raw_bytes'])

    def test_bigobj_header_and_twenty_byte_symbol_layout(self):
        self.assertEqual(coff_sections(object_bytes(big=True))['sections'][0]['raw_bytes'], 3)

    def test_long_section_name_uses_coff_string_table(self):
        data = object_bytes(name=b'/4')
        name = b'.rdata$long-owned-name\0'
        struct.pack_into('<I', data, 8, len(data))
        data += struct.pack('<I', len(name) + 4) + name
        self.assertEqual(coff_sections(data)['sections'][0]['name'], '.rdata$long-owned-name')

    def test_uninitialized_storage_is_not_raw_file_contribution(self):
        data = object_bytes(name=b'.bss', payload=b'', flags=0xc0000080)
        struct.pack_into('<II', data, 20 + 16, 4096, 0)
        self.assertEqual(coff_sections(data)['sections'][0]['raw_bytes'], 0)

    def test_truncated_headers_and_tables(self):
        for data in [b'', b'\0' * 19, object_bytes()[:40], object_bytes(big=True)[:55]]:
            with self.subTest(size=len(data)), self.assertRaises(ValueError):
                coff_sections(data)

    def test_unknown_bigobj_class_is_not_treated_as_valid(self):
        data = object_bytes(big=True)
        data[12] ^= 1
        with self.assertRaisesRegex(ValueError, 'class'):
            coff_sections(data)

    def test_pe_optional_header_is_not_an_object(self):
        data = object_bytes()
        struct.pack_into('<H', data, 16, 240)
        with self.assertRaisesRegex(ValueError, 'optional header'):
            coff_sections(data)

    def test_section_payload_outside_file_is_rejected(self):
        data = object_bytes()
        struct.pack_into('<I', data, 20 + 20, len(data) + 1)
        with self.assertRaisesRegex(ValueError, 'payload'):
            coff_sections(data)

    def test_invalid_symbol_string_table_is_rejected(self):
        data = object_bytes(name=b'/4')
        struct.pack_into('<II', data, 8, len(data) - 2, 50)
        with self.assertRaisesRegex(ValueError, 'symbol/string'):
            coff_sections(data)

    def test_nonzero_symbol_count_requires_a_table_pointer(self):
        for big in (False, True):
            for count in (1, 0xffffffff):
                with self.subTest(bigobj=big, symbols=count):
                    data = object_bytes(big=big)
                    struct.pack_into('<I', data, 52 if big else 12, count)
                    with self.assertRaisesRegex(ValueError, 'symbol.*table'):
                        coff_sections(data)

    def test_symbol_string_table_cannot_overlap_section_headers(self):
        for big in (False, True):
            with self.subTest(bigobj=big):
                data = object_bytes(big=big)
                pointer = (56 if big else 20) + 8
                struct.pack_into('<I', data, 48 if big else 8, pointer)
                # A plausible four-byte empty string table in the section's
                # otherwise ignored VirtualSize field must not count as data.
                struct.pack_into('<I', data, pointer, 4)
                with self.assertRaisesRegex(ValueError, 'overlap'):
                    coff_sections(data)

    def test_valid_zero_and_nonzero_symbol_table_extents(self):
        for big in (False, True):
            for count in (0, 1):
                with self.subTest(bigobj=big, symbols=count):
                    data = object_bytes(big=big)
                    expected = coff_sections(data)
                    symbol = struct.pack('<8sIiHBB' if big else '<8sIhHBB',
                                         b'owned', 0, 1, 0, 2, 0)
                    struct.pack_into('<II', data, 48 if big else 8, len(data), count)
                    data += symbol * count + struct.pack('<I', 4)
                    self.assertEqual(expected, coff_sections(data))

    def test_unknown_machine_is_rejected(self):
        data = object_bytes()
        struct.pack_into('<H', data, 0, 0xbeef)
        with self.assertRaisesRegex(ValueError, 'machine'):
            coff_sections(data)

    def test_output_cannot_overwrite_source_or_existing_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaisesRegex(ValueError, 'build/'):
                collect(root, root / 'build', 'Release', None, root / 'receipt.zip')
            previous = root / 'build/qa/previous.zip'
            previous.parent.mkdir(parents=True)
            previous.write_bytes(b'preserved')
            with self.assertRaisesRegex(ValueError, 'exists'):
                collect(root, root / 'build', 'Release', None, previous)
            self.assertEqual(previous.read_bytes(), b'preserved')

    def test_current_target_inputs_exclude_stale_objects_on_disk(self):
        with tempfile.TemporaryDirectory() as directory:
            # collect() canonicalizes its roots before invoking these helpers.
            # Windows TEMP may contain an 8.3 alias for the same directory.
            root = Path(directory).resolve()
            build = root / 'build'
            source = root / 'src/current.cpp'
            source.parent.mkdir()
            source.write_text('int current();')
            current = build / 'CMakeFiles/owned.dir/src/current.cpp.obj'
            stale = current.with_name('obsolete.cpp.obj')
            current.parent.mkdir(parents=True)
            current.write_bytes(object_bytes())
            stale.write_bytes(object_bytes())
            graph = ('TARGET\towned\nTYPE\tOBJECT_LIBRARY\nSOURCES\tsrc/current.cpp\nEND\n'
                     'TARGET\tshipping\nTYPE\tSHARED_LIBRARY\nSOURCES\t' + str(current) + '\nEND\n')
            (build / 'partition.Release.targets').write_text(graph)
            self.assertEqual(current_objects(root, build, {'owned', 'shipping'}, 'Release'), [current])

    def test_vs_object_mapping_resolves_configuration_and_explicit_name(self):
        with tempfile.TemporaryDirectory() as directory:
            # Match collect()'s canonical input boundary, including 8.3 TEMP.
            root = Path(directory).resolve()
            build = root / 'build'
            build.mkdir()
            source = root / 'src/entry.cpp'
            project = build / 'shipping.vcxproj'
            project.write_text('''<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
<IntDir>shipping.dir\\Release\\</IntDir></PropertyGroup>
<ItemGroup><ClCompile Include="../src/entry.cpp"><ObjectFileName>$(IntDir)explicit.obj</ObjectFileName></ClCompile></ItemGroup>
</Project>''')
            self.assertEqual(vs_object_paths(project, 'Release', {source}),
                             {source: build / 'shipping.dir/Release/explicit.obj'})

    def test_unknown_vs_path_macros_are_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            project = Path(directory) / 'shipping.vcxproj'
            project.write_text('<Project><PropertyGroup><IntDir>$(Unknown)/</IntDir></PropertyGroup></Project>')
            with self.assertRaisesRegex(ValueError, 'Unresolved'):
                vs_object_paths(project, 'Release', set())



class EvidencePublicationTests(unittest.TestCase):
    def test_new_bundle_preserves_sorted_members_and_sidecar_digest(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / 'size.zip'
            payload = {'z/last.txt': b'last', 'a/first.txt': b'first'}
            digest = size_evidence.publish_evidence(output, payload)
            with zipfile.ZipFile(output) as archive:
                self.assertEqual(['a/first.txt', 'z/last.txt'], archive.namelist())
                for name, data in payload.items():
                    self.assertEqual(data, archive.read(name))
            self.assertEqual(digest, size_evidence.sha(output.read_bytes()))
            self.assertEqual(digest + '  size.zip\n',
                             Path(str(output) + '.sha256').read_text(encoding='utf-8'))
            self.assertEqual(['size.zip', 'size.zip.sha256'],
                             sorted(path.name for path in output.parent.iterdir()))

    def test_preexisting_outputs_and_legacy_partials_are_not_overwritten(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / 'size.zip'
            victim = root / 'unrelated-source'
            victim.write_bytes(b'preserved source bytes')
            for name in ('size.zip', 'size.zip.sha256',
                         'size.zip.partial', 'size.zip.sha256.partial'):
                target = root / name
                with self.subTest(name=name):
                    os.link(victim, target)
                    try:
                        with self.assertRaisesRegex(ValueError, 'Existing or unsafe'):
                            size_evidence.publish_evidence(output, {'test': b'data'})
                        self.assertEqual(b'preserved source bytes', victim.read_bytes())
                    finally:
                        target.unlink()
                    self.assertEqual(['unrelated-source'],
                                     sorted(path.name for path in root.iterdir()))

    def test_symlinked_legacy_partial_cannot_redirect_archive_write(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / 'size.zip'
            victim = root / 'unrelated-source'
            victim.write_bytes(b'preserved source bytes')
            legacy = root / 'size.zip.partial'
            try:
                legacy.symlink_to(victim)
            except (OSError, NotImplementedError):
                self.skipTest('Symlink creation unavailable on this host')
            with self.assertRaisesRegex(ValueError, 'Existing or unsafe'):
                size_evidence.publish_evidence(output, {'test': b'data'})
            self.assertEqual(b'preserved source bytes', victim.read_bytes())
            self.assertTrue(legacy.is_symlink())
            self.assertFalse(output.exists())

    def test_checksum_publication_failure_removes_new_zip_and_staging(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / 'size.zip'
            original_replace = os.replace
            calls = []

            def inject(source, destination):
                calls.append(Path(destination))
                if len(calls) == 2:
                    self.assertEqual(Path(str(output) + '.sha256'), Path(destination))
                    self.assertTrue(output.is_file())
                    raise OSError('injected checksum publication failure')
                return original_replace(source, destination)

            with patch.object(size_evidence.os, 'replace', side_effect=inject):
                with self.assertRaisesRegex(OSError, 'injected checksum publication failure'):
                    size_evidence.publish_evidence(output, {'test': b'data'})
            self.assertEqual(2, len(calls))
            self.assertEqual([], list(root.iterdir()))

    def test_collect_rejects_dangling_output_symlink_before_any_build_reads(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'build').mkdir()
            output = root / 'build/size.zip'
            try:
                output.symlink_to(root / 'build/target-not-yet-created.zip')
            except (OSError, NotImplementedError):
                self.skipTest('Symlink creation unavailable on this host')
            with self.assertRaisesRegex(ValueError, 'symlink'):
                size_evidence.collect(root, root / 'build', 'Release', None, output)
            self.assertFalse((root / 'build/target-not-yet-created.zip').exists())


if __name__ == '__main__':
    unittest.main(verbosity=2)
