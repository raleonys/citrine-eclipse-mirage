#!/usr/bin/env python3
"""Verify exact REDFIELD package files and immutable source adoptions."""
import argparse
import hashlib
import json
from pathlib import Path
import sys
import importlib.util
import re
sys.path.insert(0, str(Path(__file__).resolve().parent / 'build_orchestration'))
from build_contract import verify_build_contract
from build_common import load_profiles

def verify_application_architecture(root):
    contract=json.loads((root/'metadata/ARCHITECTURE.json').read_text(encoding='utf-8'))
    application={p.relative_to(root).as_posix() for folder in ['src','include','tests','tools','debug']
                 for p in (root/folder).rglob('*') if p.is_file()}
    if application!=set(contract['application_files']):
        raise ValueError('APPLICATION_OWNERSHIP_INVENTORY_MISMATCH')
    project_name=contract['project_name'].lower()
    for old in ['include/'+project_name,'tests/adopted','tests/regression']:
        if (root/old).exists():raise ValueError('LEGACY_APPLICATION_TREE:'+old)
    for relative in sorted(application):
        text=(root/relative).read_text(encoding='utf-8')
        if project_name in relative.lower() or project_name in text.lower():
            raise ValueError('PROJECT_NAME_IN_APPLICATION_CODE:'+relative)
        if 'SPDX-License-Identifier: GPL-3.0-only' not in text:
            raise ValueError('APPLICATION_LICENSE_MISSING:'+relative)
        for name in contract['retired_global_apis']:
            if re.search(r'\b'+re.escape(name)+r'\s*\(',text):
                raise ValueError('RETIRED_GLOBAL_API:'+relative+':'+name)
    for folder in ['scripts']:
        for path in (root/folder).rglob('*'):
            if not path.is_file() or '__pycache__' in path.parts:continue
            if re.search(r'\b'+re.escape(project_name)+r'[_/]',path.read_text(encoding='utf-8'),re.IGNORECASE):
                raise ValueError('PROJECT_NAME_IN_BUILD_IDENTIFIER:'+str(path.relative_to(root)))
    for line in (root/'CMakeLists.txt').read_text(encoding='utf-8').splitlines():
        if project_name in line.lower() and not re.fullmatch(
                r'project\(\s*'+re.escape(contract['project_name'])+r'\s+LANGUAGES C CXX\)',line.strip()):
            raise ValueError('PROJECT_NAME_IN_BUILD_IDENTIFIER:CMakeLists.txt')
    adopted=json.loads((root/'metadata/ADOPTED_SOURCES.json').read_text(encoding='utf-8'))['files']
    if any(x['destination'] in application for x in adopted):
        raise ValueError('ORIGIN_BASED_APPLICATION_EXEMPTION')
    graph=json.loads((root/'metadata/BUILD_GRAPH.json').read_text(encoding='utf-8'))
    generic={'tests/support/runtime/memory_allocator.cpp','tests/support/runtime/ptd_runtime.cpp'}
    if generic & set(graph['core_sources']) or set(graph.get('test_support_sources',[]))!=generic:
        raise ValueError('PRODUCTION_PTD_COUPLING')
    for key,value in contract['preserved_build_surfaces'].items():
        if graph[key]!=value:raise ValueError('PRESERVED_BUILD_SURFACE_CHANGED:'+key)

def verify(root, require_refactor_complete=False):
    manifest_path = root / "metadata/PACKAGE_MANIFEST.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    expected = manifest["files"]
    actual = {p.relative_to(root).as_posix() for p in root.rglob("*")
              if p.is_file() and p.relative_to(root).parts[0] not in {"build", "dist"} and "__pycache__" not in p.parts}
    if actual != set(expected) | {"metadata/PACKAGE_MANIFEST.json"}:
        raise ValueError("FILE_SET_MISMATCH")
    for relative, record in expected.items():
        path = (root / relative)
        if not path.resolve().is_relative_to(root.resolve()) or path.is_symlink():
            raise ValueError("UNSAFE_PATH:"+relative)
        content = path.read_bytes()
        if len(content) != record["bytes"] or hashlib.sha256(content).hexdigest() != record["sha256"]:
            raise ValueError("SHA_MISMATCH:"+relative)
    adopted = json.loads((root / "metadata/ADOPTED_SOURCES.json").read_text())["files"]
    for record in adopted:
        if hashlib.sha256((root / record["destination"]).read_bytes()).hexdigest() != record["source_sha256"]:
            raise ValueError("ADOPTION_MISMATCH:"+record["destination"])
    adopted_paths={item["destination"] for item in adopted}
    derived=json.loads((root/"metadata/DERIVED_SOURCES.json").read_text())["files"]
    destinations=set()
    validated_references=set(adopted_paths)
    for item in derived:
        dest=item["destination"]
        if dest in destinations or dest in adopted_paths:raise ValueError("DERIVATION_INVENTORY_MISMATCH")
        destinations.add(dest);expected_bytes=bytearray()
        for segment in item["segments"]:
            if "literal" in segment:expected_bytes.extend(segment["literal"].encode())
            else:
                if segment["reference"] not in validated_references:raise ValueError("DERIVATION_REFERENCE_MISMATCH")
                data=(root/segment["reference"]).read_bytes();offset=segment["offset"];length=segment["length"]
                if offset<0 or length<0 or offset+length>len(data):raise ValueError("DERIVATION_RANGE_MISMATCH")
                fragment=data[offset:offset+length]
                if hashlib.sha256(fragment).hexdigest()!=segment["sha256"]:raise ValueError("DERIVATION_FRAGMENT_MISMATCH")
                expected_bytes.extend(fragment)
        if hashlib.sha256(expected_bytes).hexdigest()!=item["destination_sha256"] or (root/dest).read_bytes()!=expected_bytes:
            raise ValueError("DERIVATION_MISMATCH:"+dest)
        validated_references.add(dest)
    verify_application_architecture(root)
    graph = json.loads((root / "metadata/BUILD_GRAPH.json").read_text())
    qualification = verify_build_contract(root, graph, require_refactor_complete)
    load_profiles(root)
    units = graph["core_sources"] + graph.get("test_support_sources",[]) + graph.get("windows_core_sources",[]) + graph["windows_dll_sources"] + list(graph["tests"].values()) + [v["source"] for v in graph.get("diagnostics",{}).values()] + [graph["windows_smoke_source"],graph["windows_native_test_source"]]
    if len(units) != len(set(units)) or set(units) != {p.relative_to(root).as_posix() for d in ["src","tests","tools","third_party"] for p in (root/d).rglob("*.cpp")}:
        raise ValueError("BUILD_GRAPH_MISMATCH")
    spec=importlib.util.spec_from_file_location('hook_build',root/'scripts/build_orchestration/hook_build.py')
    hook_build=importlib.util.module_from_spec(spec);spec.loader.exec_module(hook_build);hook_build.verify_patch(root)
    registry = json.loads((root / "metadata/ENTRYPOINTS.json").read_text())
    commands = {item["launcher"] for item in registry["human_entrypoints"]}
    if commands != {p.relative_to(root).as_posix() for p in root.rglob("*.cmd") if p.relative_to(root).parts[0] not in {"build","dist"}} or [p for p in root.rglob("*.bat") if p.relative_to(root).parts[0] not in {"third_party","build","dist"}]:
        raise ValueError("LAUNCHER_SET_MISMATCH")
    for item in registry["human_entrypoints"]:
        launcher, target = root/item["launcher"], root/item["target"]
        if launcher.with_suffix("") != target.with_suffix("") or not target.is_file():
            raise ValueError("LAUNCHER_TARGET_MISMATCH")
    exports = (root / "exports/preloader_l.def").read_text().splitlines()[2:]
    if [x.split() for x in exports] != [["DllMain","@1"],["SURGE","@2"],["DllShutdown","@3"]]:
        raise ValueError("EXPORT_CONTRACT_MISMATCH")
    native_exports=(root / "exports/preloader_l_native_test.def").read_text().splitlines()[2:]
    if [x.split() for x in native_exports] != [["DllMain","@1"],["SURGE","@2"],["DllShutdown","@3"],["RFTestStatus","@100"],["RFTestSetFaults","@101"],["RFTestTransport","@102"]]:
        raise ValueError("NATIVE_TEST_EXPORT_CONTRACT_MISMATCH")
    print(f"PRELOADER_L_INTEGRITY_PASS files={len(actual)} adopted={len(adopted)} derived={len(derived)} build_units={len(units)}")
    print('PRELOADER_L_IMPORT_REFACTOR ' + json.dumps(qualification, sort_keys=True))
    return qualification

if __name__ == "__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--require-refactor-complete", action="store_true",
                        help="Reject incomplete imported-code qualification;ordinary local verification reports pending work")
    args=parser.parse_args()
    try:
        verify(args.root, args.require_refactor_complete)
    except (ValueError, OSError, KeyError, TypeError) as error:
        print("PRELOADER_L_INTEGRITY_FAIL:", error, file=sys.stderr)
        sys.exit(1)
