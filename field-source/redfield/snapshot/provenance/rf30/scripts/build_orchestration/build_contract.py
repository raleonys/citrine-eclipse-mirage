# SPDX-License-Identifier: GPL-3.0-only
"""Internal shared source-role and imported-code qualification checks."""
import hashlib
import json
from pathlib import Path

APPLICATION_ROOTS = ("src", "include", "tests", "tools", "debug")
TEST_SUPPORT = {"tests/support/runtime/memory_allocator.cpp", "tests/support/runtime/ptd_runtime.cpp"}
TEST_HEADERS = {"preloader_l/runtime/memory_allocator.hpp", "preloader_l/runtime/ptd_runtime.hpp"}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def owned_path(root, relative):
    if not isinstance(relative, str) or not relative or "\\" in relative:
        raise ValueError("BUILD_CONTRACT_UNSAFE_PATH")
    path = Path(relative)
    result = root / path
    if path.is_absolute() or ".." in path.parts or result.is_symlink() or not result.resolve().is_relative_to(root.resolve()):
        raise ValueError("BUILD_CONTRACT_UNSAFE_PATH:" + relative)
    return result


def managed_files(root):
    files = {p.relative_to(root).as_posix() for folder in APPLICATION_ROOTS
             for p in (root / folder).rglob("*") if p.is_file()}
    files |= {p.relative_to(root).as_posix() for p in (root / "scripts").rglob("*")
              if p.is_file() and "__pycache__" not in p.parts and p.suffix in {".py", ".cmd", ".cmake"}}
    return files | {"CMakeLists.txt"}


def source_roles(graph):
    roles = {}
    groups = {"production": graph["core_sources"] + graph["windows_core_sources"] + graph["windows_vendor_sources"] + graph["windows_dll_sources"],
              "test_support": graph["test_support_sources"], "regression": list(graph["tests"].values()),
              "diagnostic": [x["source"] for x in graph["diagnostics"].values()],
              "abi_host": [graph["windows_smoke_source"]], "native_host": [graph["windows_native_test_source"]]}
    for role, paths in groups.items():
        for path in paths:
            if path in roles:
                raise ValueError("BUILD_ROLE_DUPLICATE:" + path)
            roles[path] = role
    return roles


def role_for_file(relative, roles):
    if relative in roles:
        return roles[relative]
    if relative.startswith("tests/support/"):
        return "test_support"
    if relative.startswith(("tests/native/", "tools/")):
        return "native_host"
    if relative.startswith("tests/"):
        return "regression"
    if relative.startswith("debug/"):
        return "debug"
    if relative.startswith("scripts/") or relative == "CMakeLists.txt":
        return "build_tool"
    return "production"


def verify_roles(root, graph):
    roles = source_roles(graph)
    if graph.get('test_arguments') != {'test_profile_response':['examples/surge.cfg']}:
        raise ValueError('REGRESSION_ARGUMENT_CONTRACT')
    if not owned_path(root,'examples/surge.cfg').is_file():
        raise ValueError('REGRESSION_EXAMPLE_MISSING')
    if set(graph["test_support_sources"]) != TEST_SUPPORT:
        raise ValueError("PRODUCTION_PTD_COUPLING")
    if graph.get("test_support_include_directories") != ["tests/support/include"]:
        raise ValueError("TEST_SUPPORT_INCLUDE_CONTRACT")
    for relative, role in roles.items():
        path = owned_path(root, relative)
        if not path.is_file():
            raise ValueError("BUILD_ROLE_SOURCE_MISSING:" + relative)
        if role == "production" and not relative.startswith(("src/", "third_party/")):
            raise ValueError("PRODUCTION_TEST_SOURCE:" + relative)
        if role in {"abi_host", "native_host"} and not relative.startswith("tests/native/"):
            raise ValueError("NATIVE_HOST_TREE:" + relative)
    for relative in graph["include_directories"]:
        path = owned_path(root, relative)
        if not path.is_dir() or relative.startswith(("tests/", "tools/", "debug/")):
            raise ValueError("PRODUCTION_TEST_INCLUDE:" + relative)
    for definition in graph["compile_definitions"] + graph["windows_compile_definitions"]:
        name = definition.split("=", 1)[0]
        if name.startswith("PRELOADER_L_") or name in {"_DEBUG", "NDEBUG"}:
            raise ValueError("PRODUCTION_TEST_DEFINE:" + definition)
    # Explicitly deny the isolated model declarations in the production source
    # and header closure. System/vendor headers retain their own pinned policy.
    for folder in ("src", "include"):
        for path in (root / folder).rglob("*"):
            if path.is_file():
                text = path.read_text(encoding="utf-8")
                if any('"' + name + '"' in text for name in TEST_HEADERS):
                    raise ValueError("PRODUCTION_TEST_HEADER:" + path.relative_to(root).as_posix())
    return roles


def compile_role(graph, relative, defines=(), variant=""):
    roles = source_roles(graph)
    if relative not in roles:
        raise ValueError("UNREGISTERED_COMPILE_INPUT:" + relative)
    if variant or defines:
        if relative not in graph["windows_dll_sources"] or variant != "_native_test" or list(defines) != ["/DPRELOADER_L_NATIVE_TEST=1"]:
            raise ValueError("UNAPPROVED_COMPILE_VARIANT:" + relative)
        return "native_entry"
    return roles[relative]


def verify_imports(root, graph, roles, require_complete=False):
    ledger = json.loads((root / "metadata/IMPORT_REFACTOR.json").read_text(encoding="utf-8"))
    if ledger.get("schema") != 1 or ledger.get("rule_revision") != "2026-10-21.1":
        raise ValueError("IMPORT_REFACTOR_SCHEMA")
    records = ledger["files"]
    paths = [row["path"] for row in records]
    if len(paths) != len(set(paths)) or set(paths) != managed_files(root):
        raise ValueError("IMPORT_REFACTOR_INVENTORY")
    pending = []
    for row in records:
        relative = row["path"]
        if digest(owned_path(root, relative)) != row["sha256"]:
            raise ValueError("IMPORT_REFACTOR_STALE:" + relative)
        if row.get("role") != role_for_file(relative, roles):
            raise ValueError("IMPORT_REFACTOR_ROLE:" + relative)
        for key in ("owner", "responsibility", "lifetime_dependency", "failure_contract", "adaptation_plan"):
            if not isinstance(row.get(key), str) or not row[key].strip():
                raise ValueError("IMPORT_REFACTOR_RECORD:" + relative + ":" + key)
        if row.get("license") != "GPL-3.0-only" or row.get("origin_exempt", False):
            raise ValueError("IMPORT_REFACTOR_ORIGIN_EXEMPTION:" + relative)
        if row["origin"] not in {"inherited", "project_authored"} or (row["origin"] == "inherited" and not row["sources"]):
            raise ValueError("IMPORT_REFACTOR_SOURCE:" + relative)
        for source in row["sources"]:
            if not source["path"].startswith("provenance/") or digest(owned_path(root, source["path"])) != source["sha256"]:
                raise ValueError("IMPORT_REFACTOR_SOURCE:" + relative)
        if row["status"] == "PENDING":
            pending.append(relative)
            continue
        if row["status"] != "QUALIFIED" or row.get("adaptation_kind") not in {"STRUCTURAL_REFACTOR", "TARGET_BOUNDARY_REFACTOR", "ALREADY_CONFORMING_ISOLATED_ALGORITHM"}:
            raise ValueError("IMPORT_REFACTOR_QUALIFICATION:" + relative)
        proof = json.loads(owned_path(root, row["evidence"]).read_text(encoding="utf-8"))
        if (not row["evidence"].startswith("evidence/") or proof.get("status") != "PASS" or proof.get("subject") != relative
                or proof.get("sha256") != row["sha256"] or proof.get("adaptation_kind") != row["adaptation_kind"]
                or not proof.get("structural_justification", "").strip() or not proof.get("checks") or not proof.get("dependencies")):
            raise ValueError("IMPORT_REFACTOR_EVIDENCE:" + relative)
        for dependency, expected in proof["dependencies"].items():
            if digest(owned_path(root, dependency)) != expected:
                raise ValueError("IMPORT_REFACTOR_EVIDENCE_STALE:" + relative)
        if any(check.get("status") != "PASS" or not check.get("name") for check in proof["checks"]):
            raise ValueError("IMPORT_REFACTOR_EVIDENCE:" + relative)
    integrations = ledger["vendor_integrations"]
    if {x["vendor"] for x in integrations} != {"asio", "pegtl", "tinyxml2", "minhook"} or len(integrations) != 4:
        raise ValueError("IMPORT_REFACTOR_VENDOR_INVENTORY")
    for row in integrations:
        if not row.get("integration_plan"):
            raise ValueError("IMPORT_REFACTOR_VENDOR_QUALIFICATION")
        if row["status"] == "PENDING":
            pending.append("vendor-integration:" + row["vendor"])
            continue
        if row["status"] != "QUALIFIED":
            raise ValueError("IMPORT_REFACTOR_VENDOR_QUALIFICATION")
        proof = json.loads(owned_path(root, row["evidence"]).read_text(encoding="utf-8"))
        if (not row["evidence"].startswith("evidence/") or proof.get("status") != "PASS"
                or proof.get("vendor") != row["vendor"] or not proof.get("dependencies")
                or not proof.get("integration_justification") or not proof.get("checks")
                or any(check.get("status") != "PASS" or not check.get("name") for check in proof["checks"])):
            raise ValueError("IMPORT_REFACTOR_VENDOR_EVIDENCE")
        for dependency, expected in proof["dependencies"].items():
            if digest(owned_path(root, dependency)) != expected:
                raise ValueError("IMPORT_REFACTOR_VENDOR_EVIDENCE_STALE")
    summary = {"rule_revision": ledger["rule_revision"], "managed_files": len(records),
               "qualified_files": len(records) - len([p for p in pending if not p.startswith("vendor-integration:")]),
               "pending_files": len([p for p in pending if not p.startswith("vendor-integration:")]),
               "pending_vendor_integrations": len([p for p in pending if p.startswith("vendor-integration:")]),
               "status": "PENDING" if pending else "COMPLETE"}
    if require_complete and pending:
        raise ValueError("IMPORT_REFACTOR_PENDING:" + str(len(pending)))
    return summary


def verify_build_contract(root, graph, require_complete=False):
    try:
        roles = verify_roles(root, graph)
        return verify_imports(root, graph, roles, require_complete)
    except (KeyError, TypeError) as error:
        raise ValueError("BUILD_CONTRACT_SCHEMA:" + str(error)) from error
