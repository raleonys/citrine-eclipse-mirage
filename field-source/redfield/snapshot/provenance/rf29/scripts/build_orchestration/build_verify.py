#!/usr/bin/env python3
"""REDFIELD developer build + tests; no official release/pointer operations."""
import argparse
import codecs
from contextlib import ExitStack
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time
import signal
from hook_build import prepare as prepare_minhook
from build_contract import compile_role

ROOT = Path(__file__).resolve().parents[2]

def stop_process_tree(process):
    if process.poll() is not None:
        return
    if os.name == "nt":
        taskkill = Path(os.environ.get("SystemRoot", r"C:\Windows")) / "System32/taskkill.exe"
        try:
            subprocess.run([str(taskkill), "/PID", str(process.pid), "/T", "/F"],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=10)
        except (OSError, subprocess.TimeoutExpired):
            pass
        if process.poll() is None:
            process.kill()
    else:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
    process.wait(timeout=10)

def run_logged(command, *, cwd, env, log, label, timeout=180, allowed_exit=None):
    """Run a real process; show phase/output immediately and persist output live."""
    command = [str(x) for x in command]
    print(f"[RUN] {label}", flush=True)
    started = time.monotonic()
    collected = []
    decoder = codecs.getincrementaldecoder("oem" if os.name == "nt" else "utf-8")(errors="replace")
    with Path(log).open("ab", buffering=0) as sink:
        sink.write(("\n[RUN] "+label+"\n"+json.dumps(command)+"\n").encode("utf-8"))
        offset = sink.tell()
        process = subprocess.Popen(command, cwd=cwd, env=env, stdout=sink,
            stderr=subprocess.STDOUT, start_new_session=os.name != "nt")
        heartbeat = started + 5
        with Path(log).open("rb") as reader:
            reader.seek(offset)
            def drain(final=False):
                text = decoder.decode(reader.read(), final=final)
                if text:
                    collected.append(text)
                    print(text, end="", flush=True)
            try:
                while True:
                    drain()
                    code = process.poll()
                    if code is not None:
                        drain(final=True)
                        break
                    now = time.monotonic()
                    if now-started >= timeout:
                        stop_process_tree(process)
                        drain(final=True)
                        message = f"\n[TIMEOUT {timeout}s] {label}\n"
                        sink.write(message.encode("utf-8"));print(message, end="", flush=True)
                        raise subprocess.TimeoutExpired(command, timeout, output="".join(collected))
                    if now >= heartbeat:
                        message = f"[WAIT {int(now-started)}s] {label}"
                        print(message, flush=True)
                        # Do not write heartbeat into the same file being tailed.
                        heartbeat = now+5
                    time.sleep(0.05)
            except BaseException:
                stop_process_tree(process)
                raise
        message = f"\n[EXIT {code}; {time.monotonic()-started:.1f}s] {label}\n"
        sink.write(message.encode("utf-8"));print(message, end="", flush=True)
    output = "".join(collected)
    if allowed_exit is not None and code != allowed_exit:
        raise RuntimeError(f"command exit={code}: {label}. Log: {log}")
    return subprocess.CompletedProcess(command, code, output)

def windows_environment(build):
    print("[ENV] Preparing Visual Studio x64 environment", flush=True)
    # Windows keys are case-insensitive. Avoid duplicate PATH/Path entries and
    # remove compiler/linker injection variables before setup as well as after.
    env = {key.upper(): value for key, value in os.environ.items()}
    forbidden = {"CL", "_CL_", "LINK", "_LINK_"}
    for key in forbidden:
        env.pop(key, None)
    if not shutil.which("cl.exe", path=env.get("PATH")):
        log = build / "msvc_environment.log"
        log.write_text("REDFIELD RF29 BUILD BOUNDARY REFACTOR 01 - MSVC environment\n", encoding="utf-8")
        base = Path(env.get("PROGRAMFILES(X86)", r"C:\Program Files (x86)"))
        vswhere = base / "Microsoft Visual Studio/Installer/vswhere.exe"
        if not vswhere.is_file():
            raise RuntimeError("MSVC not found: install Visual Studio C++ x64 tools")
        discovered = run_logged([str(vswhere), "-latest", "-products", "*", "-requires",
            "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"],
            cwd=build, env=env, log=log, label="Discover Visual Studio (vswhere)", timeout=30)
        if discovered.returncode:
            log.write_text(discovered.stdout, encoding="utf-8")
            raise RuntimeError(f"Visual Studio discovery failed (exit {discovered.returncode}).\n{discovered.stdout}\nLog: {log}")
        install = discovered.stdout.strip()
        setup = Path(install) / "Common7/Tools/VsDevCmd.bat"
        if not install or not setup.is_file():
            raise RuntimeError("Visual Studio x64 development environment not found")
        # CALL performs a second percent expansion. Reject paths that could
        # escape this known local batch-file invocation; spaces/&/() stay quoted.
        if any(c in str(setup) for c in ['"', '%', '\r', '\n']):
            raise RuntimeError("unsupported VS setup path")
        # Only fixed, quote-free CMD arguments cross subprocess's Windows CRT
        # quoting layer. The resolved setup path travels through the environment
        # and is quoted inside this generated internal helper (not an entrypoint).
        with tempfile.TemporaryDirectory(prefix="rf_vs_", dir=build) as temporary, ExitStack() as cleanup:
            directory = Path(temporary)
            dump = directory / "environment.txt"
            # A descendant of VsDevCmd may retain its inherited working
            # directory after CMD exits. Never make the disposable directory
            # a process CWD. Only the private environment dump lives there.
            # The helper contains no captured environment and is removed before
            # the private dump directory, including on setup failure/timeout.
            script = build / "_rf_capture_environment.cmd"
            cleanup.callback(script.unlink, missing_ok=True)
            script.write_bytes((
                '@echo off\r\nsetlocal DisableDelayedExpansion\r\n'
                'call "%PRELOADER_L_VS_SETUP%" -no_logo -arch=x64 -host_arch=x64\r\n'
                'set "PRELOADER_L_VS_SETUP_EXIT=%ERRORLEVEL%"\r\n'
                'if not "%PRELOADER_L_VS_SETUP_EXIT%"=="0" exit /b %PRELOADER_L_VS_SETUP_EXIT%\r\n'
                '"%COMSPEC%" /d /u /c set > "%PRELOADER_L_VS_ENV_FILE%"\r\n'
                'exit /b %ERRORLEVEL%\r\n').encode("ascii"))
            child_env = dict(env, PRELOADER_L_VS_SETUP=str(setup), PRELOADER_L_VS_ENV_FILE=str(dump))
            try:
                result = run_logged([env.get("COMSPEC", "cmd.exe"), "/d", "/v:off", "/c", "_rf_capture_environment.cmd"],
                    cwd=build, env=child_env, log=log, label="Initialize Visual Studio x64 (VsDevCmd)", timeout=120)
            except subprocess.TimeoutExpired as error:
                data = error.stdout or b""
                diagnostic = decode_cmd_output(data)
                raise RuntimeError(f"Visual Studio setup timed out.\n{diagnostic}\nLog: {log}") from error
            diagnostic = decode_cmd_output(result.stdout)
            if result.returncode:
                raise RuntimeError(f"Visual Studio setup failed (exit {result.returncode}).\n{diagnostic}\nLog: {log}")
            if not dump.is_file():
                raise RuntimeError(f"Visual Studio setup produced no environment. Log: {log}")
            # CMD /u writes SET's redirected output as UTF-16LE. Never include
            # the full environment (which may contain secrets) in diagnostics.
            captured = {}
            try:
                lines = dump.read_text(encoding="utf-16le").splitlines()
            except UnicodeError as error:
                raise RuntimeError(f"Visual Studio environment output is not valid UTF-16LE. Log: {log}") from error
            for line in lines:
                if "=" in line and not line.startswith("="):
                    key, value = line.split("=", 1)
                    if key:
                        captured[key.upper()] = value
            if not captured.get("PATH") or not shutil.which("cl.exe", path=captured["PATH"]):
                raise RuntimeError(f"Visual Studio setup did not expose cl.exe; install the C++ x64 tools. Log: {log}")
            env = captured
    for key in list(env):
        if key in forbidden or key.startswith("PRELOADER_L_VS_"):
            del env[key]
    print("[ENV] Visual Studio environment ready", flush=True)
    return env

def decode_cmd_output(data):
    if isinstance(data, str):
        return data
    if not data:
        return ""
    if b"\x00" in data[:2048] or data.startswith(b"\xff\xfe"):
        return data.decode("utf-16" if data.startswith(b"\xff\xfe") else "utf-16le", errors="replace")
    return data.decode("oem" if os.name == "nt" else "utf-8", errors="replace")

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sanitize", choices=["none", "address", "undefined", "thread"], default="none")
    parser.add_argument("--cxx", default="g++")
    parser.add_argument("--leak-check", choices=["on", "off"], default="on")
    args = parser.parse_args()
    windows = os.name == "nt"
    if windows and args.sanitize != "none":
        raise RuntimeError("RF29 sanitizers are supported by portable GCC validation only")
    mode = f"portable_{args.sanitize}" + ("_no_leaks" if args.leak_check == "off" else "")
    build = ROOT / "build" / ("msvc_x64" if windows else mode)
    build.mkdir(parents=True, exist_ok=True)
    (build / "receipt.json").unlink(missing_ok=True)
    graph = json.loads((ROOT / "metadata/BUILD_GRAPH.json").read_text())
    print(f"[START] RF29 BUILD BOUNDARY REFACTOR 01: {len(graph['core_sources'])} core sources, {len(graph['tests'])} regressions", flush=True)
    print(f"[OUTPUT] {build}", flush=True)
    (build / "build.log").write_text("REDFIELD RF29 BUILD BOUNDARY REFACTOR 01 - starting environment preparation\n", encoding="utf-8")
    # One source/role/import authority is shared by operator verification,
    # this builder and CMake. Pending reviews are visible, never closure PASS.
    import importlib.util
    spec = importlib.util.spec_from_file_location('source_verifier', ROOT / 'scripts/verify_source.py')
    verifier = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(verifier)
    qualification = verifier.verify(ROOT)
    env = windows_environment(build) if windows else os.environ.copy()
    env["ASAN_OPTIONS"] = f"detect_leaks={int(args.leak_check == 'on')}:halt_on_error=1"
    env["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
    def run(command, timeout=180, allowed_exit=0, label=None):
        return run_logged(command, cwd=build, env=env, log=build/"build.log",
            label=label or Path(command[0]).name, timeout=timeout, allowed_exit=allowed_exit).stdout
    compiler = shutil.which("cl.exe", path=env.get("PATH")) if windows else shutil.which(args.cxx)
    if not compiler:
        raise RuntimeError("C++ compiler not found")
    print(f"[COMPILER] {compiler}", flush=True)
    includes = [("/I" if windows else "-I") + str(ROOT / p) for p in graph["include_directories"]]
    support_includes = [("/I" if windows else "-I") + str(ROOT / p) for p in graph["test_support_include_directories"]]
    flags = ["/nologo", "/std:c++20", "/EHsc", "/permissive-", "/utf-8", "/MT", "/W4", "/O2"] if windows else ["-std=c++20", "-pthread", "-Wall", "-Wextra", "-Werror", "-O1", "-g"]
    definitions=graph["compile_definitions"]+(graph["windows_compile_definitions"] if windows else [])
    flags += [("/D" if windows else "-D")+item for item in definitions]
    windows_libraries=graph["windows_libraries"] if windows else []
    if not windows and args.sanitize != "none":
        flags += ["-fsanitize=" + args.sanitize, "-fno-omit-frame-pointer", "-fno-pie", "-no-pie"]
    suffix = ".obj" if windows else ".o"
    linked_records = {}
    def validate_output(path, object_file=False):
        data = path.read_bytes()
        if windows:
            valid = (len(data) >= 20 and int.from_bytes(data[:2], "little") == 0x8664) if object_file else (len(data) >= 64 and data[:2] == b"MZ")
            if valid and not object_file:
                pe = int.from_bytes(data[0x3c:0x40], "little")
                valid = (pe >= 64 and len(data) >= pe+24 and data[pe:pe+4] == b"PE\0\0"
                         and int.from_bytes(data[pe+4:pe+6], "little") == 0x8664)
        else:
            valid = (len(data) >= 64 and data[:4] == b"\x7fELF" and data[4] == 2
                     and data[5] == 1 and (not object_file or int.from_bytes(data[16:18], "little") == 1))
        if not valid:
            raise RuntimeError(f"invalid compiler output ({len(data)} bytes): {path}")
        if not object_file:
            if not windows and not path.stat().st_mode & 0o111:
                raise RuntimeError("linked output lacks execute permission: "+str(path))
            linked_records[path]=(len(data),hashlib.sha256(data).hexdigest())
        return data
    object_records = {}
    def check_objects(paths):
        for path in paths:
            data=path.read_bytes()
            if object_records[path] != (len(data),hashlib.sha256(data).hexdigest()):
                raise RuntimeError("compiled object changed after validation: "+str(path))
    compile_index = 0
    compile_roles = {}
    if windows:
        print("[PHASE] Verify and prepare pinned MinHook SAFE-FREEZE", flush=True)
    vendor = prepare_minhook(ROOT,build) if windows else None
    compile_total = len(graph['core_sources'])+len(graph.get('test_support_sources',[]))+len(graph['tests'])+len(graph.get('diagnostics',{}))
    if windows:
        compile_total += len(graph.get('windows_core_sources',[]))+len(graph.get('windows_vendor_sources',[]))+2*len(graph['windows_dll_sources'])+2
    def compile_one(relative, defines=(), variant=""):
        nonlocal compile_index
        role = compile_role(graph, relative, defines, variant)
        compile_index += 1
        object_dir = build / 'objects' / role
        object_dir.mkdir(parents=True, exist_ok=True)
        obj = object_dir / (relative.replace("/", "__") + variant + suffix)
        obj.unlink(missing_ok=True)
        source=ROOT/relative
        if windows and relative.startswith('third_party/minhook/upstream/'):
            source=vendor/Path(relative).relative_to('third_party/minhook/upstream')
        output = ["/c", str(source), "/Fo" + str(obj)] if windows else ["-c", str(source), "-o", str(obj)]
        unit_flags=flags
        if windows and source.suffix=='.c':
            unit_flags=['/nologo','/TC','/MT','/W3','/O2','/DWIN32_LEAN_AND_MEAN','/D_WIN32_WINNT=0x0A00']
        assertion_flags=(["/UNDEBUG"] if windows else ["-UNDEBUG"]) if relative.startswith("tests/") else []
        unit_includes = includes + (support_includes if role in {'test_support','regression','diagnostic'} else [])
        run([compiler, *unit_flags, *assertion_flags, *defines, *unit_includes, *output],
            label=f"CXX {compile_index}/{compile_total}: {relative}{variant}")
        data=validate_output(obj, object_file=True)
        object_records[obj]=(len(data),hashlib.sha256(data).hexdigest())
        compile_roles[obj.relative_to(build).as_posix()] = {'source':relative, 'role':role,
            'defines':list(defines), 'include_directories':graph['include_directories'] +
            (graph['test_support_include_directories'] if role in {'test_support','regression','diagnostic'} else [])}
        return obj
    print("[INPUT] Hashing source/header/tool inputs", flush=True)
    source_digest = hashlib.sha256()
    # Bind every compile/header/test/tool/launcher input. Evidence, reports and
    # acceptance metadata are outputs, so cannot recursively enter this digest.
    input_files = []
    for path in sorted(ROOT.rglob("*")):
        rel = path.relative_to(ROOT)
        if path.is_file() and "__pycache__" not in rel.parts and (rel.parts[0] in {"include", "src", "third_party", "tests", "tools", "scripts", "exports", "provenance"}
                or rel.as_posix() in {"CMakeLists.txt", "metadata/BUILD_GRAPH.json", "metadata/DERIVED_SOURCES.json", "metadata/MINHOOK_BUILD.json", "metadata/IMPORT_REFACTOR.json"}):
            digest = hashlib.sha256(path.read_bytes()).digest()
            source_digest.update(rel.as_posix().encode() + b"\0" + digest)
            input_files.append({"path":rel.as_posix(), "sha256":digest.hex()})
    print("[PHASE] Compile core", flush=True)
    objects = [compile_one(p) for p in graph["core_sources"]]
    if windows:
        print('[PHASE] Compile Windows hooks and pinned MinHook SAFE-FREEZE',flush=True)
        objects += [compile_one(p) for p in graph.get('windows_core_sources',[])+graph.get('windows_vendor_sources',[])]
    support_objects = [compile_one(p) for p in graph.get("test_support_sources",[])]
    receipt = {"project":"REDFIELD", "checkpoint":"RF29", "classification":"LOCAL_BUILD/NOT_RELEASE",
               "source_tree_sha256":source_digest.hexdigest(), "platform":sys.platform,
               "build_input_files":input_files, "compiler":compiler,
               "production_objects":[p.name for p in objects], "test_support_objects":[p.name for p in support_objects],
               "sanitizer":args.sanitize, "leak_detection_requested":args.leak_check,
               "tests":[], "windows_dll":"NOT_RUN", "consumer":"HOOKS_INTEGRATED; SDK_HANDSHAKE_NOT_IMPLEMENTED"}
    receipt["compiler_output_validation"] = "fresh object; ELF64 relocatable/COFF x64; linked ELF64/PE x64; object hashes unchanged before links/final receipt; all linked output hashes/execute permissions rechecked before receipt"
    receipt['import_refactor'] = qualification
    def link(main_obj, output, extra=()):
        check_objects([main_obj,*objects,*support_objects])
        output.unlink(missing_ok=True)
        tail = ["/Fe"+str(output), "/link", *windows_libraries, *extra] if windows else ["-o", str(output), *extra]
        run([compiler, *flags, main_obj, *objects, *support_objects, *tail], label=f"LINK {output.name}")
        validate_output(output)
    for name, relative in graph["tests"].items():
        main_obj = compile_one(relative)
        exe = build / (name + (".exe" if windows else ""))
        link(main_obj, exe)
        output = run([exe], timeout=45)
        receipt["tests"].append({"name":name, "status":"PASS", "exit_code":0,
                                 "binary_sha256":hashlib.sha256(exe.read_bytes()).hexdigest(), "stdout":output.strip()})
        print("PASS", name, flush=True)
    receipt["diagnostics"] = []
    for name, diagnostic in graph.get("diagnostics",{}).items():
        main_obj=compile_one(diagnostic["source"])
        exe=build/(name+(".exe" if windows else ""))
        link(main_obj,exe)
        observed=diagnostic["observed_exit"]
        output=run([exe],timeout=45,allowed_exit=observed)
        receipt["diagnostics"].append({"name":name,"status":"FAIL_OBSERVED","exit_code":observed,
            "reason":diagnostic["reason"],"stdout":output.strip(),"binary_sha256":hashlib.sha256(exe.read_bytes()).hexdigest()})
        print("DIAGNOSTIC_FAIL_OBSERVED",name,"exit",observed,flush=True)
    if windows:
        dll_objects = [compile_one(p) for p in graph["windows_dll_sources"]]
        check_objects([*dll_objects,*objects])
        dll = build / "preloader_l.dll"
        run([compiler, "/nologo", "/LD", "/MT", *dll_objects, *objects, "/Fe"+str(dll), "/link",
             "/DEF:"+str(ROOT/"exports/preloader_l.def"), "/DYNAMICBASE", "/NXCOMPAT", "/HIGHENTROPYVA", *windows_libraries], label="LINK preloader_l.dll")
        validate_output(dll)
        data = dll.read_bytes()
        pe = int.from_bytes(data[0x3c:0x40], "little")
        if data[pe:pe+4] != b"PE\0\0" or int.from_bytes(data[pe+4:pe+6], "little") != 0x8664:
            raise RuntimeError("DLL is not PE x64")
        smoke_obj = compile_one(graph["windows_smoke_source"])
        check_objects([smoke_obj])
        smoke = build / "abi_smoke.exe"
        run([compiler, "/nologo", "/MT", smoke_obj, "/Fe"+str(smoke),"/link",*windows_libraries], label="LINK abi_smoke.exe")
        validate_output(smoke)
        output = run([smoke, dll])
        receipt["windows_dll"] = {"status":"PASS", "binary_sha256":hashlib.sha256(data).hexdigest(),
                                   "scope":"BOOTSTRAP_LIFECYCLE_ONLY", "smoke":output.strip()}
        print("PASS production DLL lifecycle smoke", flush=True)
        native_objects=[compile_one(p, ["/DPRELOADER_L_NATIVE_TEST=1"], "_native_test") for p in graph["windows_dll_sources"]]
        check_objects([*native_objects,*objects])
        native_dll=build/"preloader_l_native_test.dll"
        run([compiler,"/nologo","/LD","/MT",*native_objects,*objects,"/Fe"+str(native_dll),"/link",
             "/DEF:"+str(ROOT/"exports/preloader_l_native_test.def"),"/DYNAMICBASE","/NXCOMPAT","/HIGHENTROPYVA",*windows_libraries], label="LINK preloader_l_native_test.dll")
        validate_output(native_dll)
        native_host_obj=compile_one(graph["windows_native_test_source"])
        check_objects([native_host_obj])
        native_host=build/"native_acceptance.exe"
        run([compiler,"/nologo","/MT",native_host_obj,"/Fe"+str(native_host),"/link",*windows_libraries], label="LINK native_acceptance.exe")
        validate_output(native_host)
        native_output=run([native_host,native_dll],timeout=120)
        if "PRELOADER_L_NATIVE_ACCEPTANCE_PASS" not in native_output:
            raise RuntimeError("native acceptance completion marker missing")
        receipt["windows_native_acceptance"]={"status":"PASS","test_only":True,
            "binary_sha256":hashlib.sha256(native_dll.read_bytes()).hexdigest(),"stdout":native_output.strip()}
    check_objects(object_records)
    for path, expected in linked_records.items():
        data=path.read_bytes()
        if (len(data),hashlib.sha256(data).hexdigest())!=expected:
            raise RuntimeError("linked output changed after validation: "+str(path))
        if not windows and not path.stat().st_mode & 0o111:
            raise RuntimeError("linked output lost execute permission: "+str(path))
    receipt["linked_output_sha256"]={p.name:record[1] for p,record in linked_records.items()}
    receipt["compiled_object_sha256"]={p.name:record[1] for p,record in object_records.items()}
    receipt['compile_roles'] = compile_roles
    for item in input_files:
        if hashlib.sha256((ROOT/item["path"]).read_bytes()).hexdigest()!=item["sha256"]:
            raise RuntimeError("build input changed during verification: "+item["path"])
    receipt_path = build / "receipt.json"
    receipt_path.write_text(json.dumps(receipt, indent=2)+"\n", encoding="utf-8")
    print("PRELOADER_L_RF29_SCOPED_PASS_WITH_RECORDED_DIAGNOSTIC", receipt_path)
    return 0

if __name__ == "__main__":
    try:
        sys.exit(main())
    except (RuntimeError, ValueError, OSError, KeyError, TypeError, subprocess.SubprocessError) as error:
        print("PRELOADER_L_RF29_FAIL:", error, file=sys.stderr)
        sys.exit(1)
