# SPDX-License-Identifier: GPL-3.0-only

"""Hash-pinned build-only cleanup controls with precise failure oracles."""
import argparse
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import tempfile

CURRENT_SHA256 = '34390dda120e912d642d46874e21fdadf786db7f6e183e4621c782a0c034f28a'
GUARD = b'''\n\n    if (g_hHeap != NULL && (g_shutdownPending || !RetryRetainedThreadOwnership()))
    {
        LeaveSpinLock();
        return MH_ERROR_MEMORY_PROTECT;
    }'''
CONTROLS = {
    'create': (
        b'MH_STATUS WINAPI MH_CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal)',
        'ee27d06f2f52a5d3357c3b3fb525316493812b796e5392bc754481a2384d7e1f',
        'FAIL: new hook creation cannot bypass retained cleanup'),
    'remove': (
        b'MH_STATUS WINAPI MH_RemoveHook(LPVOID pTarget)',
        '61717f4e04b9789387226160346411cda0360d32b502fe70770c51546a95765c',
        'FAIL: disabled removal cannot retire trampoline under unresolved ownership'),
    'enable': (
        b'static MH_STATUS EnableHook(LPVOID pTarget, BOOL enable)',
        '97a74400e2bf6c975963ac234a46c99d299ed6a1c14f65ff6c982cfabee7ad7a',
        'FAIL: already-disabled mutation must report pending cleanup'),
    'queue': (
        b'static MH_STATUS QueueHook(LPVOID pTarget, BOOL queueEnable)',
        'da81bbd6643d2d462437c11529fb92d427826ab7681d3f4eae0130de51eddaf3',
        'FAIL: queue mutation cannot bypass retained cleanup'),
    'apply': (
        b'MH_STATUS WINAPI MH_ApplyQueued(VOID)',
        'fb6874e9d715ea8679886b1636c2f60f82e17fea69baedce7d15b2ecb448307a',
        'FAIL: no-op apply must not hide unresolved owner'),
}


def controlled_source(source, mode):
    if mode not in CONTROLS:
        raise ValueError('Unknown cleanup control')
    signature, expected_sha, _ = CONTROLS[mode]
    if hashlib.sha256(source).hexdigest() != CURRENT_SHA256 or source.count(signature) != 1:
        raise ValueError('Cleanup control requires the exact verified hook source')
    start = source.index(signature)
    end = source.index(b'\n}', start)
    body = source[start:end]
    if body.count(GUARD) != 1:
        raise ValueError('Cleanup control requires one guard in its selected entry')
    result = source[:start] + body.replace(GUARD, b'', 1) + source[end:]
    if hashlib.sha256(result).hexdigest() != expected_sha:
        raise ValueError('Cleanup control does not match its pinned single-guard mutation')
    return result


def prepare(source, output, mode):
    root = Path(__file__).resolve().parents[2]
    build = root / 'build'
    source, output = Path(source).absolute(), Path(output).absolute()
    for path in (source, output):
        if build not in path.parents or path.is_symlink() or path.resolve() != path:
            raise ValueError('Cleanup control paths must remain inside the owned build directory')
    data = controlled_source(source.read_bytes(), mode)
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=output.parent, prefix='.cleanup-hook-', delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(data)
        os.replace(temporary, output)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)
    print('HOOK_CLEANUP_CONTROL_SOURCE: ' + mode + ' SHA256=' + CONTROLS[mode][1])


def require_failure(mode, code, stdout, stderr):
    expected = CONTROLS[mode][2]
    failures = [line for line in (stdout + '\n' + stderr).splitlines() if line.startswith('FAIL:')]
    if code != 1 or failures != [expected]:
        raise RuntimeError('Cleanup control did not reproduce exactly its selected ownership failure')


def run(mode, command):
    result = subprocess.run(command, capture_output=True, text=True, encoding='utf-8',
                            errors='replace', timeout=25)
    print(result.stdout, end='')
    print(result.stderr, end='', file=sys.stderr)
    require_failure(mode, result.returncode, result.stdout, result.stderr)
    print('HOOK_CLEANUP_CONTROL: PASS ' + mode + ' source=' + CONTROLS[mode][1])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='operation', required=True)
    build = commands.add_parser('prepare')
    build.add_argument('--source', type=Path, required=True)
    build.add_argument('--output', type=Path, required=True)
    build.add_argument('--mode', choices=CONTROLS, required=True)
    execute = commands.add_parser('run')
    execute.add_argument('--mode', choices=CONTROLS, required=True)
    execute.add_argument('command', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    if args.operation == 'prepare':
        prepare(args.source, args.output, args.mode)
    else:
        command = args.command[1:] if args.command[:1] == ['--'] else args.command
        if not command:
            parser.error('A cleanup-control executable command is required')
        run(args.mode, command)


if __name__ == '__main__':
    main()
