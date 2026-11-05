"""Authorized full-build link step; main compiles/links but never loads the DLL.

Validate inputs before rotating outputs. Importing this module starts no tools.
"""
from pathlib import Path
from datetime import datetime, timezone
import hashlib
import json
import os
import re
import subprocess


def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def write(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def main():
    root = Path(__file__).resolve().parents[1]
    outputs = root / "outputs"
    source = outputs / "source-workspace"
    gate = outputs / "dll-link-gate-source"
    target = gate / "build-current-full"
    run = Path((root / "work/current-rebuild-path.txt").read_text(encoding="utf-8-sig").strip()).resolve(strict=True)
    require(run.is_relative_to((root / "work").resolve()) and run.name.startswith("rebuild-"), "Invalid rebuild run path")
    record = read(source / "build-record.json")
    snapshot = read(source / "source-snapshot.json")
    full = read(source / "full-build-verification.json")
    require(record["result"] == full["result"] == "PASS", "Verified common build required")
    require((root / full["run"]).resolve() == run, "Rebuild pointer does not match verified common build")
    require(sha(source / "build-record.json") == snapshot["build_record_sha256"] == full["build_record"]["sha256"], "Common build record mismatch")
    require(sha(source / "source-snapshot.json") == full["source_snapshot"]["sha256"], "Source snapshot mismatch")
    require(snapshot.get("additional_dependency_headers", []) == full.get("additional_dependency_headers", []), "Additional dependency header proof mismatch")
    for row in snapshot["sources"] + snapshot.get("additional_dependency_headers", []):
        path = (outputs / row["path"]).resolve()
        require(path.is_relative_to(outputs.resolve()), "Source path escapes outputs")
        require(sha(path) == row["sha256"], "Unbuilt source change: " + row["path"])
    compiler = Path(os.environ["ANADIUS_TOOLCHAIN_ROOT"]) / "bin/clang++.exe"
    require(sha(compiler) == record["compiler_sha256"] == full["compiler_sha256"], "Compiler differs from verified common build")
    archives = [source / "build" / name for name in ("anadius_core_partial.a", "hook_engine_compat.a")]
    for path in archives:
        digest = sha(path)
        require(digest == next(row["sha256"] for row in record["archives"] if row["file"] == path.name), "Archive differs from common build: " + path.name)
        require(digest == next(row["sha256"] for row in full["archives"] if row["file"] == path.name), "Archive differs from full verification: " + path.name)

    # All input checks precede output mutation. Validate resolved move targets
    # and preserve existing same-run history rather than overwriting it.
    history = run / "historical-full-link-build"
    history_record = run / "historical-full-link.json"
    require(target.resolve() == gate.resolve() / "build-current-full", "Link output path was redirected")
    require(history.resolve().parent == run, "Link history escapes rebuild run")
    if target.exists():
        require(not history.exists() and not history_record.exists(), "Link history already exists; preserve this run before another link")
        target.rename(history)
        if (gate / "current-full-link.json").exists():
            history_record.write_bytes((gate / "current-full-link.json").read_bytes())
    target.mkdir()
    commands = []

    def invoke(arguments, name):
        result = subprocess.run([str(compiler), *map(str, arguments)], capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=120)
        log = target / name
        log.write_text(result.stdout + result.stderr, encoding="utf-8")
        commands.append({"program": str(compiler), "arguments": list(map(str, arguments)), "exit_code": result.returncode, "log": log.relative_to(outputs).as_posix(), "log_sha256": sha(log)})
        return result

    entry = target / "entry.o"
    leaf = target / "export_leaf.o"
    require(invoke(["-std=c++20", "-O2", "-Wall", "-Wextra", "-Werror", "-c", gate / "entry.cpp", "-o", entry], "entry-compile.log").returncode == 0, "Entry compilation failed; inspect retained log")
    require(invoke(["-c", gate / "export_leaf.S", "-o", leaf], "leaf-compile.log").returncode == 0, "Export assembly failed; inspect retained log")
    dll = target / "anadius64.reconstruction.dll"
    link_map = target / "anadius64.reconstruction.map"
    result = invoke(["-shared", "-static", entry, leaf, gate / "anadius64.def", "-Wl,--whole-archive", *archives, "-Wl,--no-whole-archive", "-lkernel32", "-luser32", "-lshell32", "-lws2_32", "-lmswsock", "-Wl,--exclude-all-symbols,--no-insert-timestamp,--verbose", "-Wl,-Map," + str(link_map), "-o", dll], "link.log")
    undefined = sorted(set(re.findall(r"(?m)^.*undefined symbol: (.+)$", result.stdout + result.stderr)))
    created = dll.is_file() and dll.stat().st_size > 0
    linked = result.returncode == 0 and created and not undefined
    status = {
        "schema": "fresh-full-archive-dll-link.v3", "checked_at": datetime.now(timezone.utc).isoformat(),
        "diagnostic_validation": "LINKED_NOT_EXECUTED" if linked else "UNEXPECTED_LINK_FAILURE",
        "production_dll_readiness": "LINKED_NOT_RUNTIME_VALIDATED" if linked else "BLOCKED",
        "dll_link_attempted": True, "dll_link_succeeded": linked, "link_exit_code": result.returncode,
        "dll_created": created, "dll_loaded_or_executed": False, "originals_accessed": False,
        "dll_artifact": {"path": dll.relative_to(outputs).as_posix(), "bytes": dll.stat().st_size, "sha256": sha(dll)} if created else None,
        "whole_archive": True, "selected_archives": [{"path": path.relative_to(outputs).as_posix(), "sha256": sha(path)} for path in archives],
        "selection_scope": "All anadius core and hook compatibility archive members; separate preloader archive excluded",
        "frozen_source_files_verified": len(snapshot["sources"]),
        "common_build_record_sha256": sha(source / "build-record.json"), "source_snapshot_sha256": sha(source / "source-snapshot.json"),
        "compiler": record["compiler"], "compiler_sha256": sha(compiler), "undefined_symbols": undefined,
        "expected_missing_production_hooks": [], "entry_source_sha256": sha(gate / "entry.cpp"), "header_sha256": sha(gate / "production_image.hpp"),
        "commands": commands, "run_directory": target.relative_to(outputs).as_posix(), "rebuild_run": run.relative_to(root).as_posix(),
        "runtime_or_loader_equivalence_established": False,
    }
    write(target / "validation.json", status)
    write(gate / "current-full-link.json", status)
    write(run / "link-result.json", status)
    print(json.dumps({key: status[key] for key in ("diagnostic_validation", "production_dll_readiness", "link_exit_code", "dll_created", "undefined_symbols")}, indent=2))
    return 0 if linked else 1


if __name__ == "__main__":
    raise SystemExit(main())
