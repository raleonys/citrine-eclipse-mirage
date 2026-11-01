"""Verify retained static-link diagnostics. Does not execute any target."""
from pathlib import Path
import hashlib
import json
import re

HERE = Path(__file__).resolve().parent
OUT = HERE.parent

def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def require(condition, explanation):
    if not condition:
        raise RuntimeError(explanation)

v = read(HERE / "validation.json")
require(v["diagnostic_validation"] == "PASS_EXPECTED_BLOCKER", "Wrong diagnostic status")
require(v["production_dll_readiness"] == "BLOCKED", "Incorrect DLL readiness")
require(v["dll_link_attempted"] and not v["dll_link_succeeded"], "Link failure not established")
require(v["link_exit_code"] != 0 and not v["dll_created"], "Unexpected DLL artifact")
require(not v["dll_loaded_or_executed"] and not v["originals_accessed"], "Scope mismatch")
require(v["new_native_ABI_provider_closures"] == 0, "False closure claim")
require(v["overall_completion_percentage"] is None, "Unmeasured overall percentage")
for name, digest in v["source_hashes"].items():
    require(sha(HERE / name) == digest, "Changed gate source: " + name)
build_path = OUT / "source-workspace/build-record.json"
require(sha(build_path) == v["baseline_common_build_record_sha256"], "Changed common record")
snapshot_path = OUT / "source-workspace/source-snapshot.json"
require(sha(snapshot_path) == v["baseline_source_snapshot_sha256"], "Changed snapshot")
snapshot = read(snapshot_path)
for source in snapshot["sources"]:
    require(sha(OUT / source["path"]) == source["sha256"], "Changed frozen source: " + source["path"])
require(len(snapshot["sources"]) == v["frozen_source_files_verified"], "Source count mismatch")
for archive in v["archive_hashes"]:
    require(sha(OUT / "source-workspace/build" / archive["file"]) == archive["sha256"], "Changed archive")

run = Path(v["run_directory"])
require(not (run / "anadius64.reconstruction.dll").exists(), "DLL unexpectedly exists")
log = (run / "link.log").read_text(encoding="utf-8-sig")
undefined = sorted(set(re.findall(r"(?m)^.*undefined symbol: (.+)$", log)))
require(undefined == sorted(v["undefined_symbols"]) and len(undefined) == 2, "Unexpected linker failures")
for symbol in v["expected_missing_production_hooks"]:
    require(sum(symbol in item for item in undefined) == 1, "Missing required production blocker")
require("-entry:DllMainCRTStartup" in log, "Compiler CRT entry was not selected")
loaded = sorted(set(re.findall(r"Loaded anadius_core_partial\.a\(([^)]+)\)", log)))
roots = ["bootstrap_factory.o", "bootstrap_owner.o", "bootstrap_native.o", "bootstrap.o", "bootstrap_backend.o"]
require(all(name in loaded for name in roots), "Actual application source roots were not extracted")
require("crt_entry.o" not in loaded, "Second recovered CRT entry linked into the candidate")

# B52 verifies direct object consumption separately from archive extraction.
factory = v["explicit_configuration_factory"]
require(factory["diagnostic_selection_only"] and not factory["loader_factory_invoked"], "Factory must remain diagnostic-only")
require(factory["cmake_direct_source_configured"] and not factory["cmake_target_executed"], "CMake configuration is not an execution claim")
require(factory["object_path"] == "source-workspace/build/configuration_factory.o", "Unexpected direct object")
require(factory["source_path"] == "anadius-configuration-factory-source/factory.cpp", "Unexpected factory source")
factory_object = OUT / factory["object_path"]
factory_source = OUT / factory["source_path"]
require(sha(factory_object) == factory["object_sha256"] and sha(factory_source) == factory["source_sha256"], "Changed explicit factory/source")
source_rows = [x for x in snapshot["sources"] if x["path"] == factory["source_path"]]
require(len(source_rows) == 1 and source_rows[0]["sha256"] == factory["source_sha256"], "Factory is not frozen common source")
common = read(build_path)
index = factory["common_compile_command_index"]
require(isinstance(index,int) and 0 <= index < len(common["commands"]), "Invalid factory compile index")
command = common["commands"][index]
args = command["arguments"]
require("-c" in args and "-o" in args, "Factory common command is not compilation")
oi = args.index("-o")
require(oi+1 < len(args) and Path(args[oi+1]).resolve() == factory_object.resolve(), "Factory common output mismatch")
require(any(not a.startswith("-") and Path(a).name == "factory.cpp" and Path(a).resolve() == factory_source.resolve() for a in args), "Factory common input mismatch")
link_args = v["commands"][-1]["arguments"]
require(sum(not a.startswith("-") and Path(a).resolve() == factory_object.resolve() for a in link_args) == 1, "Factory object absent/duplicated in actual link arguments")
read_lines = re.findall(r"(?m)^ld\.lld: Reading (.+?)\s*$", log)
require(any(Path(a).resolve() == factory_object.resolve() for a in read_lines), "Linker did not consume explicit factory object")
symbols_path = run / "factory-defined-symbols.txt"
symbols = symbols_path.read_text(encoding="utf-8-sig")
for expected in ("anadius::configuration_factory::Configuration::Configuration(", "anadius::configuration_factory::make_configuration(", "anadius::configuration_factory::install_configuration("):
    require(expected in symbols, "Factory public implementation missing: " + expected)
target = (HERE / "target.cmake").read_text(encoding="utf-8-sig")
require('"${OUT}/anadius-configuration-factory-source/factory.cpp"' in target, "CMake direct factory source missing")
explicit_factory = {
    **factory, "object_hash_verified": True, "common_source_command_verified": True,
    "actual_link_argument_verified": True, "verbose_object_consumption_verified": True,
    "defined_symbols_sha256": sha(symbols_path), "loader_reachability_proven": False,
    "new_native_body_or_ABI_closure": 0,
}

# B53 verifies direct object consumption separately from archive extraction.
services = v["explicit_services_factory"]
require(services["diagnostic_selection_only"] and not services["loader_factory_invoked"], "Factory must remain diagnostic-only")
require(services["cmake_direct_source_configured"] and not services["cmake_target_executed"], "CMake configuration is not an execution claim")
require(services["object_path"] == "source-workspace/build/services_factory.o", "Unexpected direct object")
require(services["source_path"] == "anadius-services-factory-source/factory.cpp", "Unexpected factory source")
services_object = OUT / services["object_path"]
services_source = OUT / services["source_path"]
require(sha(services_object) == services["object_sha256"] and sha(services_source) == services["source_sha256"], "Changed explicit factory/source")
source_rows = [x for x in snapshot["sources"] if x["path"] == services["source_path"]]
require(len(source_rows) == 1 and source_rows[0]["sha256"] == services["source_sha256"], "Factory is not frozen common source")
common = read(build_path)
index = services["common_compile_command_index"]
require(isinstance(index,int) and 0 <= index < len(common["commands"]), "Invalid factory compile index")
command = common["commands"][index]
args = command["arguments"]
require("-c" in args and "-o" in args, "Factory common command is not compilation")
oi = args.index("-o")
require(oi+1 < len(args) and Path(args[oi+1]).resolve() == services_object.resolve(), "Factory common output mismatch")
require(any(not a.startswith("-") and Path(a).name == "factory.cpp" and Path(a).resolve() == services_source.resolve() for a in args), "Factory common input mismatch")
link_args = v["commands"][-1]["arguments"]
require(sum(not a.startswith("-") and Path(a).resolve() == services_object.resolve() for a in link_args) == 1, "Factory object absent/duplicated in actual link arguments")
read_lines = re.findall(r"(?m)^ld\.lld: Reading (.+?)\s*$", log)
require(any(Path(a).resolve() == services_object.resolve() for a in read_lines), "Linker did not consume explicit factory object")
symbols_path = run / "services-factory-defined-symbols.txt"
symbols = symbols_path.read_text(encoding="utf-8-sig")
for expected in ("anadius::services_factory::Remaining::Remaining(", "anadius::services_factory::make_remaining("):
    require(expected in symbols, "Factory public implementation missing: " + expected)
target = (HERE / "target.cmake").read_text(encoding="utf-8-sig")
require('"${OUT}/anadius-services-factory-source/factory.cpp"' in target, "CMake direct factory source missing")
explicit_services_factory = {
    **services, "object_hash_verified": True, "common_source_command_verified": True,
    "actual_link_argument_verified": True, "verbose_object_consumption_verified": True,
    "defined_symbols_sha256": sha(symbols_path), "loader_reachability_proven": False,
    "new_native_body_or_ABI_closure": 0,
}

# B54 verifies direct object consumption separately from archive extraction.
online = v["explicit_online_factory"]
require(online["diagnostic_selection_only"] and not online["loader_factory_invoked"], "Factory must remain diagnostic-only")
require(online["cmake_direct_source_configured"] and not online["cmake_target_executed"], "CMake configuration is not an execution claim")
require(online["object_path"] == "source-workspace/build/online_factory.o", "Unexpected direct object")
require(online["source_path"] == "anadius-online-factory-source/factory.cpp", "Unexpected factory source")
online_object = OUT / online["object_path"]
online_source = OUT / online["source_path"]
require(sha(online_object) == online["object_sha256"] and sha(online_source) == online["source_sha256"], "Changed explicit factory/source")
source_rows = [x for x in snapshot["sources"] if x["path"] == online["source_path"]]
require(len(source_rows) == 1 and source_rows[0]["sha256"] == online["source_sha256"], "Factory is not frozen common source")
common = read(build_path)
index = online["common_compile_command_index"]
require(isinstance(index,int) and 0 <= index < len(common["commands"]), "Invalid factory compile index")
command = common["commands"][index]
args = command["arguments"]
require("-c" in args and "-o" in args, "Factory common command is not compilation")
oi = args.index("-o")
require(oi+1 < len(args) and Path(args[oi+1]).resolve() == online_object.resolve(), "Factory common output mismatch")
require(any(not a.startswith("-") and Path(a).name == "factory.cpp" and Path(a).resolve() == online_source.resolve() for a in args), "Factory common input mismatch")
link_args = v["commands"][-1]["arguments"]
require(sum(not a.startswith("-") and Path(a).resolve() == online_object.resolve() for a in link_args) == 1, "Factory object absent/duplicated in actual link arguments")
read_lines = re.findall(r"(?m)^ld\.lld: Reading (.+?)\s*$", log)
require(any(Path(a).resolve() == online_object.resolve() for a in read_lines), "Linker did not consume explicit factory object")
symbols_path = run / "online-factory-defined-symbols.txt"
symbols = symbols_path.read_text(encoding="utf-8-sig")
for expected in ("anadius::online_factory::Online::Online(", "anadius::online_factory::make_online(", "anadius::online_factory::install_online("):
    require(expected in symbols, "Factory public implementation missing: " + expected)
target = (HERE / "target.cmake").read_text(encoding="utf-8-sig")
require('"${OUT}/anadius-online-factory-source/factory.cpp"' in target, "CMake direct factory source missing")
explicit_online_factory = {
    **online, "object_hash_verified": True, "common_source_command_verified": True,
    "actual_link_argument_verified": True, "verbose_object_consumption_verified": True,
    "defined_symbols_sha256": sha(symbols_path), "loader_reachability_proven": False,
    "new_native_body_or_ABI_closure": 0,
}

defs = (HERE / "anadius64.def").read_text(encoding="utf-8")
aliases = re.findall(r"(?m)^\s*(CDX|RUNE|anadius)=recovered_export_leaf\s+@(\d+)\s*$", defs)
require(aliases == [("CDX", "1"), ("RUNE", "2"), ("anadius", "3")], "Wrong export aliases/ordinals")
leaf = (run / "export-leaf-disassembly.txt").read_text(encoding="utf-8-sig")
instructions = re.findall(r"(?m)^\s*[0-9a-f]+:\s+((?:[0-9a-f]{2}\s+)+)(\S+)", leaf)
require(len(instructions) == 1 and instructions[0][0].strip() == "c3" and instructions[0][1] == "retq", "Export leaf is not one RET")
cmake = (OUT / "source-workspace/CMakeLists.txt").read_text(encoding="utf-8-sig")
require('option(RECOVERED_ENABLE_DLL_LINK_GATE "Enable blocked production DLL integration target" OFF)' in cmake, "Gate must be opt-in")
require('include("${OUT}/dll-link-gate-source/target.cmake")' in cmake, "Missing CMake gate include")
require(all(command["exit_code"] == 0 for command in v["commands"][:-1]), "Compile/inspection failure")
require(v["commands"][-1]["exit_code"] != 0, "Link unexpectedly succeeded")

result = {
    "schema": "dll-link-gate.source-verification.v1",
    "result": "PASS",
    "production_dll_readiness": "BLOCKED",
    "validation_sha256": sha(HERE / "validation.json"),
    "verified_source_hashes": v["source_hashes"],
    "frozen_source_files_verified": len(snapshot["sources"]),
    "selected_archive_members": loaded,
    "required_application_root_members_verified": roots,
    "explicit_configuration_factory_object_verified": explicit_factory,
    "explicit_services_factory_object_verified": explicit_services_factory,
    "explicit_online_factory_object_verified": explicit_online_factory,
    "dll_created": False,
    "final_pe_exports_TLS_imports_unwind_verified": False,
    "cmake_target_executed": False,
    "static_compiler_link_executed": True,
    "remaining_hooks_are_new_integration_boundaries_not_a_count_of_native_functions": True,
    "runtime_equivalence_established": False,
    "canonical_native_inventory_unchanged": True,
    "log_sha256": sha(run / "link.log"),
    "entry_object_sha256": sha(run / "entry.o"),
    "export_leaf_object_sha256": sha(run / "export_leaf.o"),
}
(HERE / "source-verification.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
print(json.dumps({"result": result["result"], "readiness": result["production_dll_readiness"],
                  "application_root_members": len(roots), "selected_core_members": len(loaded),
                  "unchanged_frozen_sources": len(snapshot["sources"])}))
