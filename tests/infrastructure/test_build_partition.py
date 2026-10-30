"""Negative regressions for production/reference object and target isolation."""
from pathlib import Path
import copy
import json
import sys
import tempfile
import unittest
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'scripts'))
import check_build_partition as boundary
import audit_translation_units as unit_audit

POLICY = {
    'production_targets': ['preloader_l', 'preloader_l_core', 'preloader_l_objects', 'preloader_l_vdf_objects'],
    'reference_targets': ['preloader_l_reference_models', 'preloader_l_reference_objects'],
    'production_test_targets': ['application_contract'],
    'reference_test_targets': ['reference_contract'],
    'source_api_targets': ['preloader_l_source_api', 'preloader_l_source_host'],
    'production_object_sources': ['src/runtime/application.cpp', 'src/platform/crypt_query_host.cpp'],
    'vdf_sources': ['src/config/parser.cpp'],
    'dll_entry_sources': ['src/platform/dll_entry.cpp'],
    'reference_sources': ['src/net/transport_operations.cpp', 'src/xml/native_document.cpp'],
}
LLVM_MAP = '''Address  Size Align Out In Symbol
  1000 100 16 objects/src/platform/dll_entry.cpp.obj:(.text)
  1100 100 16 objects/src/platform/crypt_query_host.cpp.obj:(.text)
'''
MSVC_MAP = '''Preferred load address is 0000000180000000
 Start Length Name Class
 Address Publics by Value Rva+Base Lib:Object
 0001:00000000 DllMain 180001000 f dll_entry.obj
 0001:00000100 inspect_path 180001100 f crypt_query_host.obj
'''


def target(name, sources=(), deps=()):
    return {'id': name, 'name': name, 'sources': [{'path': p} for p in sources],
            'dependencies': [{'id': d} for d in deps]}


def graph():
    return {t['name']: t for t in [
        target('preloader_l', POLICY['dll_entry_sources'], ['preloader_l_core']),
        target('preloader_l_core', deps=['preloader_l_objects', 'preloader_l_vdf_objects']),
        target('preloader_l_objects', POLICY['production_object_sources']),
        target('preloader_l_vdf_objects', POLICY['vdf_sources']),
        target('preloader_l_reference_objects', POLICY['reference_sources']),
        target('preloader_l_reference_models', deps=['preloader_l_reference_objects', 'preloader_l_core']),
        target('preloader_l_source_api', deps=['preloader_l_reference_objects', 'preloader_l_core']),
        target('application_contract', deps=['preloader_l_core']),
        target('reference_contract', deps=['preloader_l_reference_models']),
    ]}


class MapBoundaryTests(unittest.TestCase):
    def test_current_model_members_cannot_hide_in_production_named_archives(self):
        policy = json.loads((Path(__file__).resolve().parents[2] / 'metadata/BUILD_PARTITION_POLICY.json').read_text())
        for name in ['src/config/order.cpp', 'src/config/config_startup.cpp',
                     'src/net/session_routing.cpp', 'src/platform/windows_effects.cpp']:
            self.assertIn(name, policy['reference_sources'])
            stem = Path(name).stem
            for text in [LLVM_MAP + ' preloader_l_core.a(' + stem + '.cpp.obj):(.text)',
                         MSVC_MAP + ' preloader_l_core.lib:' + stem + '.obj']:
                with self.subTest(source=name, text=text):
                    self.assertTrue(boundary.map_errors(text, policy)[0])

    def test_runtime_startup_is_distinct_from_retained_config_startup(self):
        policy = json.loads((Path(__file__).resolve().parents[2] / 'metadata/BUILD_PARTITION_POLICY.json').read_text())
        for text in [LLVM_MAP + ' objects/src/runtime/startup.cpp.obj:(.text)',
                     MSVC_MAP + ' preloader_l_core.lib:startup.obj']:
            self.assertEqual(boundary.map_errors(text, policy)[0], [])

    def test_supported_linker_maps(self):
        self.assertEqual(boundary.map_errors(LLVM_MAP, POLICY), ([], 'LLVM_MINGW'))
        self.assertEqual(boundary.map_errors(MSVC_MAP, POLICY), ([], 'MSVC'))

    def test_stale_archive_member_cannot_return_reference_code(self):
        for extra in [' oldcore.a(transport_operations.cpp.obj):(.text)',
                      ' oldcore.lib:transport_operations.obj',
                      ' C:\\build path\\NATIVE_DOCUMENT.cpp.obj:(.text)']:
            self.assertTrue(boundary.map_errors(LLVM_MAP + extra, POLICY)[0])

    def test_reference_archive_rejected_even_without_known_member_name(self):
        self.assertTrue(boundary.map_errors(LLVM_MAP + ' preloader_l_reference_models.lib:other.obj', POLICY)[0])

    def test_empty_unknown_and_wrong_target_maps_are_rejected(self):
        for text in ['', 'success', 'Address Size Align Out In Symbol\n']:
            self.assertTrue(boundary.map_errors(text, POLICY)[0])

    def test_utf16_msvc_map(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'link.map'
            path.write_text(MSVC_MAP, encoding='utf-16')
            self.assertEqual(boundary.map_errors(boundary.read_map(path), POLICY), ([], 'MSVC'))


class InitializerInventoryTests(unittest.TestCase):
    def test_compiler_coff_header_includes_ctors_and_retains_crt_tls(self):
        # Captured section table shape from a compiled x64 COFF object; the .ctors
        # section holds 23 pointer-sized constructor slots (0xB8 = 184 bytes).
        output = (
            'Sections:\nIdx Name          Size     VMA              Type\n'
            '  0 .text         00000000 0000000000000000 TEXT\n'
            '  1 .data         00000000 0000000000000000 DATA\n'
            '  2 .bss          00000000 0000000000000000 BSS\n'
            '  3 .ctors        000000b8 0000000000000000 DATA\n'
            '  4 .CRT$XCU      00000008 0000000000000000 DATA\n'
            '  5 .tls$         00000008 0000000000000000 DATA\n'
        )
        self.assertEqual(['.ctors', '.CRT$XCU', '.tls$'],
                         unit_audit.initializer_sections(output))

    def test_ctors_subsections_and_zero_sized_exclusions(self):
        output = (
            '  0 .ctors.65535 00000008 0000000000000000 DATA\n'
            '  1 .ctors       00000000 0000000000000000 DATA\n'
            '  2 .ctors_fake  00000008 0000000000000000 DATA\n'
            '  3 .tls$        00000000 0000000000000000 DATA\n'
            '  4 .CRT$XCU     00000000 0000000000000000 DATA\n'
            '  5 .text        00000008 0000000000000000 TEXT\n'
        )
        self.assertEqual(['.ctors.65535'], unit_audit.initializer_sections(output))
        self.assertEqual([], unit_audit.initializer_sections('unexpected/unsupported output'))


class TargetBoundaryTests(unittest.TestCase):
    def test_reference_and_source_api_coexist_outside_production_closure(self):
        self.assertEqual(boundary.codemodel_errors(graph(), POLICY), [])

    def test_case_insensitive_cpp_suffix_in_cmake_declaration(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'cmake').mkdir()
            (root / 'cmake/Sources.cmake').write_text(
                'set(PRELOADER_L_APPLICATION_SOURCES src/runtime/app.cpp src/runtime/unlisted.CPP)\n')
            self.assertIn('src/runtime/unlisted.CPP',
                          boundary.declared_sources(root, 'PRELOADER_L_APPLICATION_SOURCES'))

    def test_uppercase_source_in_generated_target_is_not_ignored(self):
        g = graph()
        g['preloader_l_objects']['sources'].append({'path': 'src/runtime/unlisted.CPP'})
        self.assertIn('Generated target source set differs: preloader_l_objects',
                      boundary.codemodel_errors(g, POLICY))

    def test_indirect_reference_dependency_is_rejected(self):
        g = graph()
        g['preloader_l_core']['dependencies'].append({'id': 'preloader_l_reference_models'})
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_test_library_cannot_mask_production_integration_dependency(self):
        g = graph()
        g['application_contract']['dependencies'].append({'id': 'preloader_l_reference_models'})
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_source_api_cannot_mask_production_dependency(self):
        g = graph()
        g['preloader_l']['dependencies'].append({'id': 'preloader_l_source_api'})
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_direct_test_reference_source_is_rejected(self):
        for path in ['src/net/transport_operations.cpp', 'C:/source/src/net/transport_operations.cpp']:
            g = graph(); g['application_contract']['sources'].append({'path': path})
            self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_direct_test_reference_object_is_rejected(self):
        g = graph(); g['application_contract']['sources'].append({'path': 'C:/build/transport_operations.cpp.obj'})
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_new_test_cannot_silently_escape_scope_classification(self):
        g = graph(); g['unclassified'] = target('unclassified', deps=['preloader_l_reference_models'])
        g['unclassified']['type'] = 'EXECUTABLE'
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_first_build_generated_graph_does_not_require_file_api_reply(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'partition.Release.targets').write_text(
                'TARGET\tpreloader_l\nTYPE\tSHARED_LIBRARY\nDEPS\tpreloader_l_core\n'
                'SOURCES\tsrc/platform/dll_entry.cpp;C:/build path/other.cpp.obj\nEND\n')
            g = boundary.load_generated_graph(root, 'Release')
            self.assertEqual(g['preloader_l']['dependencies'], [{'id': 'preloader_l_core'}])
            self.assertEqual(g['preloader_l']['sources'][1]['path'], 'C:/build path/other.cpp.obj')

    def test_actual_compiler_source_set_is_checked(self):
        g = graph()
        g['preloader_l_objects']['sources'].append({'path': POLICY['reference_sources'][0]})
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_retained_reference_target_source_set_is_checked(self):
        g = graph(); g['preloader_l_reference_objects']['sources'].pop()
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_reference_api_and_tests_must_keep_their_payloads(self):
        for name in ['preloader_l_source_api', 'reference_contract']:
            g = graph(); g[name]['dependencies'] = [{'id': 'preloader_l_core'}]
            self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_testing_disabled_needs_no_reference_targets(self):
        g = graph()
        for name in ['preloader_l_reference_models', 'preloader_l_reference_objects',
                     'preloader_l_source_api', 'application_contract', 'reference_contract']:
            del g[name]
        self.assertEqual(boundary.codemodel_errors(g, POLICY), [])

    def test_every_source_is_owned_and_overlap_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); (root / 'cmake').mkdir()
            runtime = POLICY['production_object_sources'][:-1]
            text = 'set(PRELOADER_L_APPLICATION_SOURCES ' + ' '.join(runtime) + ')\n'
            text += 'set(PRELOADER_L_VDF_SOURCES ' + ' '.join(POLICY['vdf_sources']) + ')\n'
            text += 'set(PRELOADER_L_REFERENCE_SOURCES ' + ' '.join(POLICY['reference_sources']) + ')\n'
            (root / 'cmake/Sources.cmake').write_text(text)
            for name in sum([POLICY[k] for k in ['production_object_sources', 'vdf_sources', 'dll_entry_sources', 'reference_sources']], []):
                path = root / name; path.parent.mkdir(parents=True, exist_ok=True); path.write_text('// owned\n')
            self.assertEqual(boundary.source_errors(root, POLICY), [])
            path = root / 'src/runtime/unlisted.CPP'; path.write_text('// unlisted uppercase\n')
            self.assertTrue(boundary.source_errors(root, POLICY)); path.unlink()
            path = root / 'src/runtime/unowned.cpp'; path.write_text('// unowned\n')
            self.assertTrue(boundary.source_errors(root, POLICY)); path.unlink()
            altered = copy.deepcopy(POLICY); altered['production_object_sources'].append(POLICY['reference_sources'][0])
            self.assertTrue(boundary.source_errors(root, altered))


if __name__ == '__main__':
    unittest.main(verbosity=2)
