# SPDX-License-Identifier: GPL-3.0-only

"""Negative regressions for production/reference object and target isolation."""
from pathlib import Path
import copy
import json
import shutil
import subprocess
import sys
import tempfile
import unittest
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'scripts'))
import check_build_partition as boundary
import audit_translation_units as unit_audit

POLICY = {
    'production_targets': ['citrine_eclipse_mirage', 'citrine_eclipse_mirage_core', 'citrine_eclipse_mirage_objects', 'citrine_eclipse_mirage_vdf_objects'],
    'reference_targets': ['citrine_eclipse_mirage_reference_models', 'citrine_eclipse_mirage_reference_objects'],
    'production_test_targets': ['application_contract'],
    'reference_test_targets': ['reference_contract'],
    'source_api_targets': ['citrine_eclipse_mirage_source_api', 'citrine_eclipse_mirage_source_host'],
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
        target('citrine_eclipse_mirage', POLICY['dll_entry_sources'], ['citrine_eclipse_mirage_core']),
        target('citrine_eclipse_mirage_core', deps=['citrine_eclipse_mirage_objects', 'citrine_eclipse_mirage_vdf_objects']),
        target('citrine_eclipse_mirage_objects', POLICY['production_object_sources']),
        target('citrine_eclipse_mirage_vdf_objects', POLICY['vdf_sources']),
        target('citrine_eclipse_mirage_reference_objects', POLICY['reference_sources']),
        target('citrine_eclipse_mirage_reference_models', deps=['citrine_eclipse_mirage_reference_objects', 'citrine_eclipse_mirage_core']),
        target('citrine_eclipse_mirage_source_api', deps=['citrine_eclipse_mirage_reference_objects', 'citrine_eclipse_mirage_core']),
        target('application_contract', deps=['citrine_eclipse_mirage_core']),
        target('reference_contract', deps=['citrine_eclipse_mirage_reference_models']),
    ]}


class MapBoundaryTests(unittest.TestCase):
    def test_retired_xml_vendor_cannot_reenter_production(self):
        policy=copy.deepcopy(POLICY)
        policy['forbidden_production_source_prefixes']=['third_party/tinyxml2/']
        allowed=graph()
        allowed['xml_vendor']=target('xml_vendor',['third_party/pugixml/src/pugixml.cpp'])
        allowed['citrine_eclipse_mirage_core']['dependencies'].append({'id':'xml_vendor'})
        self.assertEqual(boundary.codemodel_errors(allowed,policy),[])
        for path in ['third_party/tinyxml2/tinyxml2.cpp','C:/source/third_party/tinyxml2/tinyxml2.cpp',
                     r'C:\source\THIRD_PARTY\TINYXML2\tinyxml2.cpp']:
            broken=copy.deepcopy(allowed)
            broken['xml_vendor']['sources']=[{'path':path}]
            self.assertTrue(any('Retired vendor source' in e for e in boundary.codemodel_errors(broken,policy)),path)

    def test_retired_xml_object_cannot_hide_in_production_archive(self):
        policy=copy.deepcopy(POLICY)
        policy['forbidden_production_object_stems']=['tinyxml2']
        self.assertEqual(boundary.map_errors(LLVM_MAP,policy)[0],[])
        for text in [LLVM_MAP+' citrine_eclipse_mirage_core.a(tinyxml2.cpp.obj):(.text)',
                     MSVC_MAP+' citrine_eclipse_mirage_core.lib:tinyxml2.obj']:
            self.assertTrue(any('Retired vendor object' in e for e in boundary.map_errors(text,policy)[0]))

    def test_current_model_members_cannot_hide_in_production_named_archives(self):
        policy = json.loads((Path(__file__).resolve().parents[2] / 'metadata/BUILD_PARTITION_POLICY.json').read_text())
        for name in ['src/config/order.cpp', 'src/config/config_startup.cpp',
                     'src/net/session_routing.cpp', 'src/platform/windows_effects.cpp']:
            self.assertIn(name, policy['reference_sources'])
            stem = Path(name).stem
            for text in [LLVM_MAP + ' citrine_eclipse_mirage_core.a(' + stem + '.cpp.obj):(.text)',
                         MSVC_MAP + ' citrine_eclipse_mirage_core.lib:' + stem + '.obj']:
                with self.subTest(source=name, text=text):
                    self.assertTrue(boundary.map_errors(text, policy)[0])

    def test_runtime_startup_is_distinct_from_retained_config_startup(self):
        policy = json.loads((Path(__file__).resolve().parents[2] / 'metadata/BUILD_PARTITION_POLICY.json').read_text())
        for text in [LLVM_MAP + ' objects/src/runtime/startup.cpp.obj:(.text)',
                     MSVC_MAP + ' citrine_eclipse_mirage_core.lib:startup.obj']:
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
        self.assertTrue(boundary.map_errors(LLVM_MAP + ' citrine_eclipse_mirage_reference_models.lib:other.obj', POLICY)[0])

    def test_empty_unknown_and_wrong_target_maps_are_rejected(self):
        for text in ['', 'success', 'Address Size Align Out In Symbol\n']:
            self.assertTrue(boundary.map_errors(text, POLICY)[0])

    def test_utf16_msvc_map(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'link.map'
            path.write_text(MSVC_MAP, encoding='utf-16')
            self.assertEqual(boundary.map_errors(boundary.read_map(path), POLICY), ([], 'MSVC'))


class TranslationUnitObjectPathTests(unittest.TestCase):
    def test_canonical_relative_and_absolute_ninja_objects(self):
        with tempfile.TemporaryDirectory() as tmp:
            root, build = Path(tmp) / 'checkout', Path(tmp) / 'build'
            source = root / 'src/runtime/sample.cpp'
            source.parent.mkdir(parents=True)
            source.write_text('int sample;', encoding='utf-8')
            obj = build / 'CMakeFiles/sample.dir/src/runtime/sample.cpp.obj'
            obj.parent.mkdir(parents=True)
            obj.write_bytes(b'owned-object')
            self.assertEqual(unit_audit.object_path(root, build, 'sample',
                            'src/runtime/sample.cpp'), obj)
            self.assertEqual(unit_audit.object_path(root, build, 'sample',
                            str(source)), obj)

    def test_parent_traversal_cannot_select_an_unrelated_object(self):
        with tempfile.TemporaryDirectory() as tmp:
            root, build = Path(tmp) / 'checkout', Path(tmp) / 'build'
            root.mkdir()
            directory = build / 'CMakeFiles/sample.dir'
            directory.mkdir(parents=True)
            fake = build / 'CMakeFiles/other.cpp.obj'
            fake.write_bytes(b'unrelated-object')
            # The old string concatenation accepts this existing foreign .obj
            # even though it lives outside the selected target directory.
            self.assertTrue((directory / '../other.cpp.obj').is_file())
            with self.assertRaisesRegex(ValueError, 'Noncanonical source'):
                unit_audit.object_path(root, build, 'sample', '../other.cpp')
            with self.assertRaisesRegex(ValueError, 'Noncanonical CMake object target'):
                unit_audit.object_path(root, build, '../sample', 'src/runtime/sample.cpp')
            with self.assertRaisesRegex(ValueError, 'outside checkout'):
                unit_audit.object_path(root, build, 'sample', str(Path(tmp) / 'other.cpp'))

    def test_redirected_source_or_object_is_not_attributed_to_target(self):
        with tempfile.TemporaryDirectory() as tmp:
            root, build = Path(tmp) / 'checkout', Path(tmp) / 'build'
            source = root / 'src/runtime/real.cpp'
            source.parent.mkdir(parents=True)
            source.write_text('int real;', encoding='utf-8')
            obj = build / 'CMakeFiles/sample.dir/src/runtime/real.cpp.obj'
            obj.parent.mkdir(parents=True)
            target = Path(tmp) / 'foreign.cpp'
            target.write_text('int foreign;', encoding='utf-8')
            try:
                (source.parent / 'alias.cpp').symlink_to(target)
            except (OSError, NotImplementedError):
                self.skipTest('File symlink creation is unavailable')
            alias_obj = obj.with_name('alias.cpp.obj')
            alias_obj.write_bytes(b'wrong-source-object')
            with self.assertRaisesRegex(ValueError, 'redirects or escapes'):
                unit_audit.object_path(root, build, 'sample', 'src/runtime/alias.cpp')
            outside_obj = Path(tmp) / 'outside.obj'
            outside_obj.write_bytes(b'foreign-object')
            obj.symlink_to(outside_obj)
            with self.assertRaisesRegex(ValueError, 'redirected'):
                unit_audit.object_path(root, build, 'sample', 'src/runtime/real.cpp')


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
                'set(CITRINE_ECLIPSE_MIRAGE_APPLICATION_SOURCES src/runtime/app.cpp src/runtime/unlisted.CPP)\n')
            self.assertIn('src/runtime/unlisted.CPP',
                          boundary.declared_sources(root, 'CITRINE_ECLIPSE_MIRAGE_APPLICATION_SOURCES'))

    def test_uppercase_source_in_generated_target_is_not_ignored(self):
        g = graph()
        g['citrine_eclipse_mirage_objects']['sources'].append({'path': 'src/runtime/unlisted.CPP'})
        self.assertIn('Generated target source set differs: citrine_eclipse_mirage_objects',
                      boundary.codemodel_errors(g, POLICY))

    def test_dangling_reference_target_cannot_escape_production_gate(self):
        g = graph()
        # The generator can omit an imported/out-of-directory target record;
        # the dependency name must still be rejected by the ownership gate.
        del g['citrine_eclipse_mirage_reference_models']
        del g['reference_contract']
        g['citrine_eclipse_mirage_core']['dependencies'].append({'id': 'citrine_eclipse_mirage_reference_models'})
        errors = boundary.codemodel_errors(g, POLICY)
        self.assertTrue(any('citrine_eclipse_mirage_core -> citrine_eclipse_mirage_reference_models' in e for e in errors), errors)

    def test_dangling_source_api_target_cannot_escape_test_gate(self):
        g = graph()
        del g['citrine_eclipse_mirage_source_api']
        g['application_contract']['dependencies'].append({'id': 'citrine_eclipse_mirage_source_api'})
        errors = boundary.codemodel_errors(g, POLICY)
        self.assertTrue(any('application_contract -> citrine_eclipse_mirage_source_api' in e for e in errors), errors)

    def test_unlisted_external_target_remains_allowed(self):
        g = graph()
        g['citrine_eclipse_mirage_core']['dependencies'].append({'id': 'asio::asio'})
        self.assertEqual([], boundary.codemodel_errors(g, POLICY))

    def test_missing_model_still_fails_reference_test_positive_check(self):
        g = graph()
        del g['citrine_eclipse_mirage_reference_models']
        errors = boundary.codemodel_errors(g, POLICY)
        self.assertIn('Reference test lost its model library: reference_contract', errors)

    def test_indirect_reference_dependency_is_rejected(self):
        g = graph()
        g['citrine_eclipse_mirage_core']['dependencies'].append({'id': 'citrine_eclipse_mirage_reference_models'})
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_test_library_cannot_mask_production_integration_dependency(self):
        g = graph()
        g['application_contract']['dependencies'].append({'id': 'citrine_eclipse_mirage_reference_models'})
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_source_api_cannot_mask_production_dependency(self):
        g = graph()
        g['citrine_eclipse_mirage']['dependencies'].append({'id': 'citrine_eclipse_mirage_source_api'})
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_direct_test_reference_source_is_rejected(self):
        for path in ['src/net/transport_operations.cpp', 'C:/source/src/net/transport_operations.cpp']:
            g = graph(); g['application_contract']['sources'].append({'path': path})
            self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_direct_test_reference_object_is_rejected(self):
        g = graph(); g['application_contract']['sources'].append({'path': 'C:/build/transport_operations.cpp.obj'})
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_configuration_defaults_targets_follow_real_cmake_ownership(self):
        root = Path(__file__).resolve().parents[2]
        policy = json.loads((root / 'metadata/BUILD_PARTITION_POLICY.json').read_text())
        cmake = (root / 'cmake/Tests.cmake').read_text(encoding='utf-8')
        production = 'configuration_defaults_contracts'
        reference = 'configuration_defaults_integration_contracts'
        # A build with BUILD_TESTING=ON must classify both executables.
        self.assertIn(production, policy['production_test_targets'])
        self.assertNotIn(production, policy['reference_test_targets'])
        self.assertIn(reference, policy['reference_test_targets'])
        self.assertNotIn(reference, policy['production_test_targets'])
        self.assertIn('add_executable(' + production + ' ', cmake)
        self.assertIn('add_executable(' + reference + ' ', cmake)
        self.assertIn(
            'target_link_libraries(' + reference + ' PRIVATE citrine_eclipse_mirage_reference_models)',
            cmake)
        # The generated-graph validator stays fail-closed rather than exempting
        # previously unknown executable targets.
        scope = copy.deepcopy(POLICY)
        scope['production_test_targets'].append(production)
        scope['reference_test_targets'].append(reference)
        current = graph()
        current[production] = target(production, ['src/runtime/application_defaults.cpp'])
        current[reference] = target(reference, deps=['citrine_eclipse_mirage_reference_models'])
        current[production]['type'] = 'EXECUTABLE'
        current[reference]['type'] = 'EXECUTABLE'
        self.assertEqual([], boundary.codemodel_errors(current, scope))
        omitted = copy.deepcopy(scope)
        omitted['production_test_targets'].remove(production)
        omitted['reference_test_targets'].remove(reference)
        self.assertIn(
            'Unclassified executable test/tool target: ' + production,
            boundary.codemodel_errors(current, omitted))
        self.assertIn(
            'Unclassified executable test/tool target: ' + reference,
            boundary.codemodel_errors(current, omitted))

    def test_new_test_cannot_silently_escape_scope_classification(self):
        g = graph(); g['unclassified'] = target('unclassified', deps=['citrine_eclipse_mirage_reference_models'])
        g['unclassified']['type'] = 'EXECUTABLE'
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_first_build_generated_graph_does_not_require_file_api_reply(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'partition.Release.targets').write_text(
                'TARGET\tcitrine_eclipse_mirage\nTYPE\tSHARED_LIBRARY\nDEPS\tcitrine_eclipse_mirage_core\n'
                'SOURCES\tsrc/platform/dll_entry.cpp;C:/build path/other.cpp.obj\nEND\n')
            g = boundary.load_generated_graph(root, 'Release')
            self.assertEqual(g['citrine_eclipse_mirage']['dependencies'], [{'id': 'citrine_eclipse_mirage_core'}])
            self.assertEqual(g['citrine_eclipse_mirage']['sources'][1]['path'], 'C:/build path/other.cpp.obj')

    def test_actual_compiler_source_set_is_checked(self):
        g = graph()
        g['citrine_eclipse_mirage_objects']['sources'].append({'path': POLICY['reference_sources'][0]})
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_retained_reference_target_source_set_is_checked(self):
        g = graph(); g['citrine_eclipse_mirage_reference_objects']['sources'].pop()
        self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_reference_api_and_tests_must_keep_their_payloads(self):
        for name in ['citrine_eclipse_mirage_source_api', 'reference_contract']:
            g = graph(); g[name]['dependencies'] = [{'id': 'citrine_eclipse_mirage_core'}]
            self.assertTrue(boundary.codemodel_errors(g, POLICY))

    def test_testing_disabled_needs_no_reference_targets(self):
        g = graph()
        for name in ['citrine_eclipse_mirage_reference_models', 'citrine_eclipse_mirage_reference_objects',
                     'citrine_eclipse_mirage_source_api', 'application_contract', 'reference_contract']:
            del g[name]
        self.assertEqual(boundary.codemodel_errors(g, POLICY), [])

    def test_separate_source_api_keeps_complete_payload_without_static_crt_graph(self):
        g = graph()
        sources = sum([POLICY[k] for k in
                       ['production_object_sources', 'vdf_sources', 'reference_sources', 'dll_entry_sources']], [])
        g['citrine_eclipse_mirage_source_api'] = target('citrine_eclipse_mirage_source_api', sources)
        self.assertEqual(boundary.codemodel_errors(g, POLICY), [])
        for source in sources:
            broken = copy.deepcopy(g)
            broken['citrine_eclipse_mirage_source_api']['sources'].remove({'path': source})
            self.assertTrue(boundary.codemodel_errors(broken, POLICY), source)
        for dependency in ['citrine_eclipse_mirage_core', 'citrine_eclipse_mirage_objects', 'citrine_eclipse_mirage_vdf_objects',
                           'citrine_eclipse_mirage_reference_objects', 'citrine_eclipse_mirage_reference_models',
                           'citrine_eclipse_mirage_minhook', 'citrine_eclipse_mirage_pugixml']:
            broken = copy.deepcopy(g)
            broken.setdefault(dependency, target(dependency))
            broken['citrine_eclipse_mirage_source_api']['dependencies'].append({'id': dependency})
            self.assertTrue(any('static-CRT' in e for e in boundary.codemodel_errors(broken, POLICY)), dependency)
        broken = copy.deepcopy(g)
        broken['citrine_eclipse_mirage_source_api']['sources'].append({'path': 'tests/unapproved.cpp'})
        self.assertTrue(boundary.codemodel_errors(broken, POLICY))

    @unittest.skipUnless(shutil.which('cmake'), 'CMake required for generated CRT property regression')
    def test_real_source_api_cmake_block_separates_crt_properties(self):
        # Configure the actual development-target block with minimal inputs.
        # This verifies CMake properties/graph construction on the host; it is
        # deliberately not native MSVC compilation or DLL runtime acceptance.
        root = Path(__file__).resolve().parents[2]
        text = (root / 'CMakeLists.txt').read_text()
        block = text.split('\nif(CITRINE_ECLIPSE_MIRAGE_BUILD_SOURCE_API_DLL)\n', 1)[1]
        block = 'if(CITRINE_ECLIPSE_MIRAGE_BUILD_SOURCE_API_DLL)\n' + block.split('\nadd_executable(citrine_eclipse_mirage_runtime_host', 1)[0]
        dev = ['citrine_eclipse_mirage_source_api', 'citrine_eclipse_mirage_source_host',
               'citrine_eclipse_mirage_source_minhook', 'citrine_eclipse_mirage_source_pugixml']
        static = ['citrine_eclipse_mirage_core', 'citrine_eclipse_mirage_objects', 'citrine_eclipse_mirage_vdf_objects',
                  'citrine_eclipse_mirage_reference_objects', 'citrine_eclipse_mirage_minhook', 'citrine_eclipse_mirage_pugixml']
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / 'src'; source.mkdir()
            for name in ['application.cpp', 'parser.cpp', 'reference.cpp', 'hook.c',
                         'src/platform/crypt_query_host.cpp', 'src/platform/dll_entry.cpp',
                         'tools/citrine_eclipse_mirage_source_host/main.cpp', 'resources/surge.rc',
                         'third_party/pugixml/src/pugixml.cpp', 'avatar.hpp']:
                path = source / name; path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('// configure fixture\n')
            cmake = '''cmake_minimum_required(VERSION 3.20)
project(source_crt_fixture LANGUAGES C CXX)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CITRINE_ECLIPSE_MIRAGE_BUILD_SOURCE_API_DLL ON)
set(CITRINE_ECLIPSE_MIRAGE_APPLICATION_SOURCES application.cpp)
set(CITRINE_ECLIPSE_MIRAGE_VDF_SOURCES parser.cpp)
set(CITRINE_ECLIPSE_MIRAGE_REFERENCE_SOURCES reference.cpp)
set(_avatar_header "${CMAKE_CURRENT_SOURCE_DIR}/avatar.hpp")
add_custom_target(citrine_eclipse_mirage_verify_vendors)
add_custom_target(citrine_eclipse_mirage_verify_hook_engine)
add_library(citrine_eclipse_mirage_asio INTERFACE)
add_library(citrine_eclipse_mirage_pegtl INTERFACE)
'''
            for name in static:
                kind = 'OBJECT' if name.endswith('_objects') else 'STATIC'
                cmake += f'add_library({name} {kind} application.cpp)\n'
            cmake += 'target_include_directories(citrine_eclipse_mirage_minhook PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")\n'
            cmake += 'set(MSVC TRUE)\n' + block
            content = ''.join(f'{name}=$<TARGET_GENEX_EVAL:{name},$<TARGET_PROPERTY:{name},MSVC_RUNTIME_LIBRARY>>\\n'
                              for name in dev + static)
            cmake += f'file(GENERATE OUTPUT "${{CMAKE_BINARY_DIR}}/crt.txt" CONTENT "{content}")\n'
            cmake += 'file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/dev-links.txt" CONTENT "$<TARGET_PROPERTY:citrine_eclipse_mirage_source_api,LINK_LIBRARIES>")\n'
            (source / 'CMakeLists.txt').write_text(cmake)
            for config, suffix in [('Debug', 'Debug'), ('Release', '')]:
                build = Path(tmp) / config
                cp = subprocess.run(['cmake', '-S', str(source), '-B', str(build),
                                     '-DCMAKE_BUILD_TYPE=' + config,
                                     '-DCMAKE_CONFIGURATION_TYPES=' + config], capture_output=True, text=True)
                self.assertEqual(cp.returncode, 0, cp.stdout + cp.stderr)
                modes = dict(line.split('=', 1) for line in (build / 'crt.txt').read_text().splitlines())
                for name in dev:
                    self.assertEqual(modes[name], 'MultiThreaded' + suffix + 'DLL', name)
                for name in static:
                    self.assertEqual(modes[name], 'MultiThreaded' + suffix, name)
                links = (build / 'dev-links.txt').read_text().split(';')
                self.assertIn('citrine_eclipse_mirage_source_minhook', links)
                self.assertIn('citrine_eclipse_mirage_source_pugixml', links)
                self.assertFalse(set(links) & set(static), links)

    def test_every_source_is_owned_and_overlap_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); (root / 'cmake').mkdir()
            runtime = POLICY['production_object_sources'][:-1]
            text = 'set(CITRINE_ECLIPSE_MIRAGE_APPLICATION_SOURCES ' + ' '.join(runtime) + ')\n'
            text += 'set(CITRINE_ECLIPSE_MIRAGE_VDF_SOURCES ' + ' '.join(POLICY['vdf_sources']) + ')\n'
            text += 'set(CITRINE_ECLIPSE_MIRAGE_REFERENCE_SOURCES ' + ' '.join(POLICY['reference_sources']) + ')\n'
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
