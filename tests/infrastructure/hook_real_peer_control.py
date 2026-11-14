# SPDX-License-Identifier: GPL-3.0-only

"""Build-only stale-action control and exact-failure oracle for real-peer tests."""
import argparse
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import tempfile

CURRENT_SHA256 = '2b795f8a28a307282c02f752348246a39b10ffb189bbc8ea8a6d1926e4259c92'
STALE_SHA256 = '872b53241e5fe605f33f6d8cde341d2971936c170dbabe2e6af57366e614ecc6'
ANCHOR = b'Freeze(&threads, pos, enable ? ACTION_ENABLE : ACTION_DISABLE)'
STALE = b'Freeze(&threads, pos, ACTION_ENABLE)'
MISMATCH = 'SINGLE_DISABLE_IP_MISMATCH: real IP retained the trampoline/relay address'
CLEANUP = 'REAL_PEER_CLEANUP: PASS original context restored, worker joined, handle count restored'


def stale_source(source):
    if hashlib.sha256(source).hexdigest() != CURRENT_SHA256 or source.count(ANCHOR) != 1:
        raise ValueError('Real-peer control requires the exact current verified hook source')
    result = source.replace(ANCHOR, STALE, 1)
    if hashlib.sha256(result).hexdigest() != STALE_SHA256:
        raise ValueError('Real-peer control does not match the pinned stale-action source')
    return result


def prepare(source, output):
    root = Path(__file__).resolve().parents[2]
    build = root / 'build'
    source, output = Path(source).absolute(), Path(output).absolute()
    if source != root / 'third_party/mithrilhook/src/hook.c' or source.is_symlink():
        raise ValueError('Real-peer control requires the pinned MithrilHook source')
    if build not in output.parents or output.is_symlink() or output.resolve() != output:
        raise ValueError('Real-peer control output must remain inside the owned build directory')
    data = stale_source(source.read_bytes())
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=output.parent, prefix='.stale-hook-', delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(data)
        os.replace(temporary, output)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)
    print('REAL_PEER_CONTROL_SOURCE: SHA256=' + STALE_SHA256)


def require_failure(code, stdout, stderr):
    lines = (stdout + '\n' + stderr).splitlines()
    if code != 42 or MISMATCH not in lines or CLEANUP not in lines or any(
            line.startswith('FAIL:') for line in lines):
        raise RuntimeError('Stale-action control did not reproduce exactly the IP mismatch with complete cleanup')


def run(command):
    result = subprocess.run(command, capture_output=True, text=True, encoding='utf-8',
                            errors='replace', timeout=25)
    print(result.stdout, end='')
    print(result.stderr, end='', file=sys.stderr)
    require_failure(result.returncode, result.stdout, result.stderr)
    print('STALE_ACTION_CONTROL: PASS expected real-IP failure, cleanup complete, source=' + STALE_SHA256)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='operation', required=True)
    build = commands.add_parser('prepare')
    build.add_argument('--source', type=Path, required=True)
    build.add_argument('--output', type=Path, required=True)
    execute = commands.add_parser('run')
    execute.add_argument('command', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    if args.operation == 'prepare':
        prepare(args.source, args.output)
    else:
        command = args.command[1:] if args.command[:1] == ['--'] else args.command
        if not command:
            parser.error('The stale-action executable command is required')
        run(command)


if __name__ == '__main__':
    main()
