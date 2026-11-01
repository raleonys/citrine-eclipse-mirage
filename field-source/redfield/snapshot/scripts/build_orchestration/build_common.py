# SPDX-License-Identifier: GPL-3.0-only
#!/usr/bin/env python3
"""Internal shared compiler/process/environment and isolated build sessions."""
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
        log.write_text("REDFIELD - MSVC environment\n", encoding="utf-8")
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


# The compiler/session is shared by the developer regression runner and all
# operator profiles. No profile consumes another profile's object or library.
GOLDEN_ORDER = ('URSYNEX Golden Finalizer', 'ARYRA Golden Layout',
                'MUSTAVEX Golden Metadata', 'CORVEXON Golden Trail',
                'FALCION Golden Cleaning')


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def input_snapshot(root):
    files = []
    aggregate = hashlib.sha256()
    metadata = {'BUILD_GRAPH.json', 'DERIVED_SOURCES.json', 'MINHOOK_BUILD.json',
                'IMPORT_REFACTOR.json', 'BUILD_PROFILES.json', 'ARCHITECTURE.json',
                'ENTRYPOINTS.json', 'BUILD_CARRYOVER_SOURCES.json', 'GOLDEN_BINDING.json',
                'ACCEPTANCE_COVERAGE.json','URSYNEX_GOLDEN_FINALIZER.json','ARYRA_GOLDEN_LAYOUT.json',
                'MUSTAVEX_GOLDEN_METADATA.json','CORVEXON_GOLDEN_TRAIL.json'}
    for path in sorted(root.rglob('*')):
        rel = path.relative_to(root)
        if (path.is_file() and '__pycache__' not in rel.parts and
                (rel.parts[0] in {'include', 'src', 'debug', 'third_party', 'tests',
                                 'tools', 'scripts', 'exports', 'provenance', 'resources'} or
                 rel.as_posix() in {'CMakeLists.txt','examples/surge.cfg'} or
                 len(rel.parts) == 2 and rel.parts[0] == 'metadata' and rel.name in metadata)):
            value = digest(path)
            aggregate.update(rel.as_posix().encode('utf-8') + b'\0' + bytes.fromhex(value))
            files.append({'path': rel.as_posix(), 'sha256': value})
    return aggregate.hexdigest(), files


def verify_inputs(root, files):
    # Re-enumerate too: a newly created input must not evade the final binding.
    _, current = input_snapshot(root)
    if current != files:
        raise RuntimeError('BUILD_INPUT_CHANGED')


def confined(root, relative):
    """Only fixed relative output paths; reject symlinks in every component."""
    rel = Path(relative)
    if (not relative or str(relative) != rel.as_posix() or rel.is_absolute() or '..' in rel.parts or
            '\\' in str(relative) or rel.parts[0] not in {'build', 'dist'}):
        raise ValueError('OUTPUT_PATH_NOT_CONFINED:' + str(relative))
    path = root
    for part in rel.parts:
        path = path / part
        if path.is_symlink():
            raise ValueError('OUTPUT_SYMLINK:' + str(relative))
    if not path.resolve().is_relative_to(root.resolve()):
        raise ValueError('OUTPUT_PATH_NOT_CONFINED:' + str(relative))
    return path


class BuildLock:
    """OS-owned exclusive lock; an abandoned process releases it automatically.

    Keep the lock inode after unlock so waiters can never lock different files
    for the same profile. The file is not a stale-PID admission mechanism.
    """
    def __init__(self, root, name):
        if name not in {'production', 'debug', 'native', 'crt', 'verify'}:
            raise ValueError('UNKNOWN_BUILD_PROFILE:' + name)
        self.path = confined(root, 'build/.preloader_l_' + name + '.lock')
        self.stream = None

    def __enter__(self):
        self.path.parent.mkdir(parents=True, exist_ok=True)
        stream = self.path.open('a+b')
        try:
            stream.seek(0, 2)
            if stream.tell() == 0:
                stream.write(b'\0'); stream.flush()
            stream.seek(0)
            if os.name == 'nt':
                import msvcrt
                msvcrt.locking(stream.fileno(), msvcrt.LK_NBLCK, 1)
            else:
                import fcntl
                fcntl.flock(stream.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError as error:
            stream.close()
            raise RuntimeError('BUILD_ALREADY_RUNNING:' + self.path.name) from error
        self.stream = stream
        return self

    def __exit__(self, *unused):
        self.stream.close()
        self.stream = None


def prepare_output(root, relative):
    path = confined(root, relative)
    # Only a locked profile's dedicated leaf directory can be cleared.
    parts = Path(relative).parts
    if not (len(parts) == 2 and parts[0] == 'build' and
            parts[1] in {'production', 'debug', 'native', 'crt'} or
            len(parts) == 3 and parts[:2] == ('build', 'verify')):
        raise ValueError('OUTPUT_RESET_NOT_APPROVED:' + relative)
    if path.exists():
        # Invalidate previous acceptance before removing objects; a locked old
        # binary must never leave its old success receipt looking current.
        (path / 'receipt.json').unlink(missing_ok=True)
        shutil.rmtree(path)
    path.mkdir(parents=True)
    return path


def clear_dist(root, profile):
    if profile == 'production':
        # An unsuccessful new production attempt must not leave a stale DLL
        # looking like its successful result. Candidate/logs remain in build.
        paths = ['dist/preloader_l.dll']
    elif profile in {'debug', 'native', 'crt'}:
        folders = {'debug': 'debug', 'native': 'native_acceptance', 'crt': 'crt_acceptance'}
        paths = ['dist/' + folders[profile]]
    else:
        raise ValueError('DIST_PROFILE_NOT_APPROVED')
    for relative in paths:
        path = confined(root, relative)
        if path.is_dir():
            shutil.rmtree(path)
        else:
            path.unlink(missing_ok=True)


def _atomic_copy(root, source, relative, expected, *, production=False):
    """Same-directory staged copy, hash check, atomic replacement and readback.

    This primitive has no Golden authority. Production reaches it only through
    publish_production after the live session and complete Outcome are checked.
    """
    target = confined(root, relative)
    if target == root / 'dist/preloader_l.dll' and not production:
        raise RuntimeError('PRODUCTION_DIST_REQUIRES_GOLDEN_OUTCOME')
    if (not production and (len(Path(relative).parts) != 3 or Path(relative).parts[0] != 'dist' or
            Path(relative).parts[1] not in {'debug','native_acceptance','crt_acceptance'})):
        raise ValueError('NONPRODUCTION_DIST_PATH_REQUIRED')
    if production and relative != 'dist/preloader_l.dll':
        raise ValueError('PRODUCTION_DIST_PATH_REQUIRED')
    if digest(source) != expected:
        raise RuntimeError('PUBLISH_SOURCE_CHANGED')
    target.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(prefix='.preloader_l.', suffix='.tmp',
                                         dir=target.parent, delete=False) as stream:
            temporary = Path(stream.name)
            with Path(source).open('rb') as incoming:
                shutil.copyfileobj(incoming, stream)
            stream.flush(); os.fsync(stream.fileno())
        if digest(temporary) != expected or digest(source) != expected:
            raise RuntimeError('PUBLISH_STAGED_HASH_MISMATCH')
        os.replace(temporary, target)
        temporary = None
        if digest(target) != expected:
            target.unlink(missing_ok=True)
            raise RuntimeError('PUBLISH_READBACK_MISMATCH')
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)
    return target


def atomic_copy(root, source, relative, expected):
    return _atomic_copy(root,source,relative,expected)


def publish_production(session, outcome, receipt):
    from golden import validate_outcome
    candidate = session.build/'preloader.unsigned.dll'
    if session.profile != 'production' or candidate not in session.linked_records:
        raise ValueError('PRODUCTION_SESSION_REQUIRED')
    expected = session.linked_records[candidate][1]
    validate_outcome(outcome,candidate,expected)
    session.linked_records[candidate] = (candidate.stat().st_size,outcome.final)
    session.finish(receipt)
    target = _atomic_copy(session.root,candidate,'dist/preloader_l.dll',outcome.final,production=True)
    try:
        session.finish(receipt)
    except BaseException:
        target.unlink(missing_ok=True)
        raise
    return target


def load_profiles(root):
    data = json.loads((root / 'metadata/BUILD_PROFILES.json').read_text(encoding='utf-8'))
    expected = {'production', 'debug', 'native', 'crt', 'verify'}
    if data.get('schema') != 1 or set(data['profiles']) != expected:
        raise ValueError('BUILD_PROFILE_SCHEMA')
    if tuple(data['golden_order']) != GOLDEN_ORDER or data.get('golden_binding') != 'BOUND_LOCAL_M20_3':
        raise ValueError('GOLDEN_BINDING_CONTRACT')
    from golden import load_contract
    load_contract(root)
    for name, row in data['profiles'].items():
        debug = name == 'debug'
        expected_dll_flags = ['/SUBSYSTEM:WINDOWS,6.00','/OSVERSION:6.0'] if name == 'production' else []
        if row.get('dll_link_flags',[]) != expected_dll_flags:
            raise ValueError('BUILD_PROFILE_DLL_LINK_POLICY:' + name)
        runtime = '/MTd' if debug else '/MT'
        if (row['runtime'] != runtime or row['build_root'] != 'build/' + name or
                row['configuration'] != ('Debug' if debug else 'Release')):
            raise ValueError('BUILD_PROFILE_BOUNDARY:' + name)
        cxx, c, link = row['cxx_flags'], row['c_flags'], row['link_flags']
        for flags in (cxx, c):
            required_compile = {runtime, '/GS'} | ({'/Od','/Zi','/D_DEBUG'} if debug else {'/O2','/DNDEBUG'})
            if not required_compile.issubset(flags) or len(flags) != len(set(flags)):
                raise ValueError('BUILD_PROFILE_FLAGS:' + name)
            for flag in flags:
                if (flag not in {'/nologo','/std:c++20','/EHsc','/permissive-','/utf-8',
                                 '/MT','/MTd','/W4','/W3','/O2','/Od','/Zi','/GS','/TC',
                                 '/DNDEBUG','/D_DEBUG','/DWIN32_LEAN_AND_MEAN',
                                 '/D_WIN32_WINNT=0x0A00'} or
                        flag in ({'/MT','/O2','/DNDEBUG'} if debug else {'/MTd','/Od','/Zi','/D_DEBUG'})):
                    raise ValueError('BUILD_PROFILE_FLAG_INJECTION:' + name + ':' + flag)
        required = {'/INCREMENTAL:NO','/DYNAMICBASE','/NXCOMPAT','/HIGHENTROPYVA'}
        if not required.issubset(link) or any(x not in required | {
                '/DEBUG:FULL','/DEBUG:NONE','/OPT:REF','/OPT:ICF','/LTCG:OFF',
                '/NOCOFFGRPINFO','/MANIFEST:NO'} for x in link):
            raise ValueError('BUILD_PROFILE_LINK_POLICY:' + name)
        if ('/DEBUG:FULL' in link) != debug or ('/DEBUG:NONE' in link) == debug:
            raise ValueError('BUILD_PROFILE_DEBUG_LEAK:' + name)
    return data


class BuildSession:
    def __init__(self, root, build, graph, *, windows, profile='verify', cxx='g++',
                 sanitize='none', leak_check='on'):
        self.root, self.build, self.graph = root, build, graph
        self.windows, self.profile = windows, profile
        self.policy = load_profiles(root)['profiles'][profile]
        self.log = build / 'build.log'
        self.log.write_text('REDFIELD ' + profile + ' - LOCAL_BUILD/NOT_RELEASE\n', encoding='utf-8')
        self.env = windows_environment(build) if windows else os.environ.copy()
        self.env['ASAN_OPTIONS'] = f"detect_leaks={int(leak_check == 'on')}:halt_on_error=1"
        self.env['UBSAN_OPTIONS'] = 'halt_on_error=1:print_stacktrace=1'
        selected = shutil.which('cl.exe' if windows else cxx, path=self.env.get('PATH'))
        if not selected:
            raise RuntimeError('C++ compiler not found')
        self.compiler = str(Path(selected).resolve())
        self.tools = {self.compiler: digest(self.compiler)}
        if windows:
            # CL and LINK must be from the same selected MSVC bin directory.
            linker = Path(self.compiler).with_name('link.exe')
            if not linker.is_file():
                raise RuntimeError('MSVC_BOUND_LINKER_MISSING')
            self.tools[str(linker)] = digest(linker)
            self.env['PATH'] = str(linker.parent) + os.pathsep + self.env.get('PATH', '')
        self.resource_compiler = None
        if windows and profile == 'production':
            resource_compiler = shutil.which('rc.exe',path=self.env.get('PATH'))
            if not resource_compiler:
                raise RuntimeError('WINDOWS_SDK_RESOURCE_COMPILER_MISSING')
            self.resource_compiler = str(Path(resource_compiler).resolve())
            self.tools[self.resource_compiler] = digest(self.resource_compiler)
        print('[COMPILER] ' + self.compiler, flush=True)
        self.flags = list(self.policy['cxx_flags']) if windows else [
            '-std=c++20', '-pthread', '-Wall', '-Wextra', '-Werror', '-O1', '-g']
        definitions = graph['compile_definitions'] + (graph['windows_compile_definitions'] if windows else [])
        self.flags += [('/D' if windows else '-D') + x for x in definitions]
        if not windows and sanitize != 'none':
            self.flags += ['-fsanitize=' + sanitize, '-fno-omit-frame-pointer', '-fno-pie', '-no-pie']
        self.suffix = '.obj' if windows else '.o'
        self.vendor = prepare_minhook(root, build) if windows else None
        self.source_digest, self.input_files = input_snapshot(root)
        self.object_records, self.linked_records, self.resource_records = {}, {}, {}
        self.compile_roles, self.commands = {}, []
        self.compile_index = 0

    def run(self, command, *, label=None, timeout=180, allowed_exit=0):
        self.commands.append({'label': label or Path(command[0]).name,
                              'argv': [str(x) for x in command]})
        return run_logged(command, cwd=self.build, env=self.env, log=self.log,
                          label=label or Path(command[0]).name, timeout=timeout,
                          allowed_exit=allowed_exit).stdout

    def validate_output(self, path, *, object_file=False):
        data = path.read_bytes()
        if self.windows:
            valid = (len(data) >= 20 and int.from_bytes(data[:2], 'little') == 0x8664) if object_file else False
            if not object_file and len(data) >= 64 and data[:2] == b'MZ':
                pe = int.from_bytes(data[0x3c:0x40], 'little')
                valid = (pe >= 64 and len(data) >= pe+24 and data[pe:pe+4] == b'PE\0\0'
                         and int.from_bytes(data[pe+4:pe+6], 'little') == 0x8664)
        else:
            valid = (len(data) >= 64 and data[:4] == b'\x7fELF' and data[4] == 2 and
                     data[5] == 1 and (not object_file or int.from_bytes(data[16:18], 'little') == 1))
        if not valid:
            raise RuntimeError('INVALID_COMPILER_OUTPUT:' + str(path))
        if not object_file:
            if not self.windows and not path.stat().st_mode & 0o111:
                raise RuntimeError('LINKED_OUTPUT_NOT_EXECUTABLE:' + str(path))
            self.linked_records[path] = (len(data), digest(path))
        return data

    def compile(self, relative, defines=(), variant=''):
        role = compile_role(self.graph, relative, defines, variant)
        allowed = {'production','abi_host'} if self.profile in {'production','debug'} else {
            'production','native_entry','native_host'} if self.profile == 'native' else {
            'production','abi_host','native_entry','native_host'} if self.profile == 'crt' else {
            'production','abi_host','native_entry','native_host','test_support','regression','diagnostic'}
        if role not in allowed:
            raise ValueError('PROFILE_SOURCE_ROLE_REJECTED:' + self.profile + ':' + role)
        if self.profile == 'native' and relative in self.graph['windows_dll_sources'] and role != 'native_entry':
            raise ValueError('NATIVE_PROFILE_REQUIRES_TEST_ENTRY')
        self.compile_index += 1
        directory = self.build / 'objects' / role
        directory.mkdir(parents=True, exist_ok=True)
        obj = directory / (relative.replace('/', '__') + variant + self.suffix)
        obj.unlink(missing_ok=True)
        source = self.root / relative
        if self.windows and relative.startswith('third_party/minhook/upstream/'):
            source = self.vendor / Path(relative).relative_to('third_party/minhook/upstream')
        flags = self.flags
        if self.windows and source.suffix == '.c':
            flags = list(self.policy['c_flags'])
        roots = list(self.graph['include_directories'])
        if role in {'test_support','regression','diagnostic'}:
            roots += self.graph['test_support_include_directories']
        includes = [('/I' if self.windows else '-I') + str(self.root / p) for p in roots]
        assertions = (['/UNDEBUG'] if self.windows else ['-UNDEBUG']) if relative.startswith('tests/') else []
        symbols = []
        if self.windows and self.profile == 'debug':
            pdb = self.build / 'symbols' / (role + '.pdb')
            pdb.parent.mkdir(exist_ok=True)
            symbols = ['/Fd' + str(pdb)]
        output = ['/c', str(source), '/Fo' + str(obj)] if self.windows else ['-c', str(source), '-o', str(obj)]
        self.run([self.compiler, *flags, *assertions, *defines, *symbols, *includes, *output],
                 label=f'CXX {self.compile_index}: {relative}{variant}')
        data = self.validate_output(obj, object_file=True)
        self.object_records[obj] = (len(data), digest(obj))
        self.compile_roles[obj.relative_to(self.build).as_posix()] = {
            'source': relative, 'role': role, 'profile': self.profile,
            'defines': list(defines), 'include_directories': roots}
        return obj

    def check_objects(self, paths):
        for path in paths:
            if path not in self.object_records or not path.is_relative_to(self.build / 'objects'):
                raise RuntimeError('FOREIGN_PROFILE_OBJECT:' + str(path))
            if (path.stat().st_size, digest(path)) != self.object_records[path]:
                raise RuntimeError('COMPILED_OBJECT_CHANGED:' + str(path))

    def compile_resource(self):
        if not self.windows or self.profile != 'production' or not self.resource_compiler:
            raise ValueError('PRODUCTION_RESOURCE_PROFILE_REQUIRED')
        source = self.root/self.graph['windows_production_resource']
        output = self.build/'preloader_version.res'
        output.unlink(missing_ok=True)
        self.run([self.resource_compiler,'/nologo','/fo'+str(output),source],label='RC production VERSIONINFO')
        data = output.read_bytes()
        null_header = bytes.fromhex('0000000020000000ffff0000ffff000000000000000000000000000000000000')
        if len(data) <= 32 or data[:32] != null_header:
            raise RuntimeError('INVALID_RESOURCE_COMPILER_OUTPUT')
        self.resource_records[output] = (len(data),digest(output))
        return output

    def check_resources(self):
        for path, expected in self.resource_records.items():
            if (path.stat().st_size,digest(path)) != expected:
                raise RuntimeError('COMPILED_RESOURCE_CHANGED')

    def link(self, output, objects, *, dll=False, exports=None):
        self.check_objects(objects)
        if (output.parent != self.build or output.is_symlink() or
                dll != bool(exports) or dll and not self.windows):
            raise ValueError('LINK_OUTPUT_CONTRACT')
        if exports not in {None, 'exports/preloader_l.def', 'exports/preloader_l_native_test.def'}:
            raise ValueError('LINK_EXPORT_CONTRACT')
        native = exports == 'exports/preloader_l_native_test.def'
        roles = [self.compile_roles[x.relative_to(self.build).as_posix()]['role'] for x in objects]
        if dll and (any(x not in {'production','native_entry'} for x in roles) or
                    ('native_entry' in roles) != native):
            raise ValueError('DLL_TEST_OBJECT_LEAK')
        output.unlink(missing_ok=True)
        if self.windows:
            tail = ['/Fe' + str(output), '/link', *self.policy['link_flags'], *self.graph['windows_libraries']]
            if dll:
                tail += self.policy.get('dll_link_flags',[])
                tail += ['/DEF:' + str(self.root / exports), '/IMPLIB:' + str(output.with_suffix('.lib'))]
                if self.profile == 'production':
                    resource = self.compile_resource()
                    self.check_resources()
                    tail.append(str(resource))
            if self.profile == 'debug':
                tail += ['/PDB:' + str(output.with_suffix('.pdb')), '/MAP:' + str(output.with_suffix('.map'))]
            command = [self.compiler, *self.flags, *(['/LD'] if dll else []), *objects, *tail]
        else:
            command = [self.compiler, *self.flags, *objects, '-o', output]
        self.run(command, label='LINK ' + output.name)
        self.validate_output(output)

    def finish(self, receipt):
        self.check_objects(self.object_records)
        self.check_resources()
        for path, expected in self.linked_records.items():
            if (path.stat().st_size, digest(path)) != expected:
                raise RuntimeError('LINKED_OUTPUT_CHANGED:' + str(path))
            if not self.windows and not path.stat().st_mode & 0o111:
                raise RuntimeError('LINKED_OUTPUT_NOT_EXECUTABLE:' + str(path))
        for tool, expected in self.tools.items():
            if digest(tool) != expected:
                raise RuntimeError('BOUND_TOOL_CHANGED:' + tool)
        verify_inputs(self.root, self.input_files)
        receipt.update(source_tree_sha256=self.source_digest, build_input_files=self.input_files,
                       compiler=self.compiler, toolchain_sha256=self.tools, compile_roles=self.compile_roles,
                       profile=self.profile, commands=self.commands,
                       compiled_object_sha256={p.name: x[1] for p,x in self.object_records.items()},
                       linked_output_sha256={p.name: x[1] for p,x in self.linked_records.items()},
                       compiled_resource_sha256={p.name: x[1] for p,x in self.resource_records.items()})
        return receipt


def source_preflight(root):
    import importlib.util
    spec = importlib.util.spec_from_file_location('source_verifier', root / 'scripts/verify_source.py')
    verifier = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(verifier)
    return verifier.verify(root)


def write_receipt(path, receipt):
    path.write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')


def audit_production_link(session, candidate):
    """Audit actual shared-session calls before the mutating Golden stage."""
    if not session.windows or session.profile != 'production':
        raise ValueError('PRODUCTION_LINK_SESSION_REQUIRED')
    session.check_objects(session.object_records); session.check_resources()
    links = [c['argv'] for c in session.commands if c['label'] == 'LINK '+candidate.name]
    if len(links) != 1 or candidate not in session.linked_records:
        raise ValueError('PRODUCTION_LINK_COMMAND_REQUIRED')
    command = links[0]; upper = [x.upper() for x in command]
    required = {'/MT','/LD','/INCREMENTAL:NO','/OPT:REF','/OPT:ICF','/DEBUG:NONE',
                '/LTCG:OFF','/NOCOFFGRPINFO','/MANIFEST:NO','/DYNAMICBASE','/NXCOMPAT',
                '/HIGHENTROPYVA','/SUBSYSTEM:WINDOWS,6.00','/OSVERSION:6.0'}
    if not required.issubset(upper) or any(x.startswith(('/ENTRY','/NODEFAULTLIB','/NOENTRY','/MD','/MTD','/DEBUG:FULL')) for x in upper):
        raise ValueError('PRODUCTION_LINK_FLAGS')
    actual = {x for x in command if x.lower().endswith('.obj')}
    expected = {str(p) for p in session.object_records if session.compile_roles[p.relative_to(session.build).as_posix()]['role'] == 'production'}
    if actual != expected or not expected or any(x not in command for x in map(str,session.resource_records)) or len(session.resource_records) != 1:
        raise ValueError('PRODUCTION_LINK_OBJECT_RESOURCE_ROLES')
    for row in session.commands:
        if row['label'].startswith('CXX '):
            flags = [x.upper() for x in row['argv']]
            if '/MT' not in flags or '/GS' not in flags or any(x in flags for x in ['/MTD','/MD','/MDD','/GS-','/ZI','/Z7','/D_DEBUG']):
                raise ValueError('PRODUCTION_COMPILE_CRT_GS_FLAGS')
    return {'status':'PASS','scope':'Actual session compile/link/object/resource/tool contract;part of MSVC Build',
            'command_sha256':hashlib.sha256(json.dumps(command).encode()).hexdigest(),
            'candidate_sha256':digest(candidate),'production_objects':len(expected)}
