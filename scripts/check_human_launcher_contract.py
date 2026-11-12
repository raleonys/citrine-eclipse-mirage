# SPDX-License-Identifier: GPL-3.0-only

"""Validate registered human entrypoints and their automation/exit-code boundary."""
import json
from pathlib import Path
import re


OBSOLETE_BUILD_WORDING = re.compile(
    r'\b(?:NOT[_]RELEASE|LOCAL[ _-](?:BUILD|GOLDEN|DIST)|'
    r'GOLDEN[_-]LOCAL|READY[_]LOCAL[_]TARGET)(?![a-zA-Z0-9])', re.IGNORECASE)


def check_build_wording(root):
    """Keep obsolete output labels out of executable tools and active contracts."""
    root = Path(root)
    paths = []
    for directory in ('scripts', 'tools', 'cmake', 'src', 'include', '.github/workflows'):
        paths.extend((root / directory).rglob('*'))
    paths.extend((root / 'metadata').glob('*.json'))
    paths.extend(root / name for name in ('CMakeLists.txt', 'CMakePresets.json'))
    errors = []
    for path in sorted(paths):
        if not path.is_file():
            continue
        if path.name != 'CMakeLists.txt' and path.suffix.lower() not in (
                '.py', '.ps1', '.cmd', '.cmake', '.c', '.h', '.cpp', '.hpp', '.rc', '.def', '.json', '.yml', '.yaml'):
            continue
        if path.name in ('SOURCE_FILES.json', 'ACTIVE_SOURCE_MANIFEST.json'):
            continue
        for line, text in enumerate(path.read_text(encoding='utf-8').splitlines(), 1):
            if OBSOLETE_BUILD_WORDING.search(text):
                errors.append(f'Obsolete build wording: {path.relative_to(root).as_posix()}:{line}')
    return errors


def terminal_line(content):
    """Inspect the entire final command, not a trailing token in a comment."""
    lines = content.rstrip().splitlines()
    return lines[-1].strip().casefold() if lines else ''


def check(root):
    root = Path(root)
    entries = json.loads((root / 'metadata/HUMAN_ENTRYPOINT_REGISTRY.json').read_text())['entrypoints']
    expected = {e['launcher'] for e in entries}
    actual = {p.relative_to(root).as_posix() for p in (root / 'scripts').rglob('*.cmd')}
    errors = check_build_wording(root)
    if expected != actual:
        errors.append('Registered launcher set differs from actual CMD set')
    for entry in entries:
        path = root / entry['launcher']
        target = root / entry['automation']
        if not path.is_file() or not target.is_file():
            errors.append('Missing entrypoint: ' + entry['launcher'])
            continue
        text = path.read_text(encoding='ascii')
        tokens = ['%~dp0' + target.name, '-LauncherOwnsSummary %*', 'set "RC=%ERRORLEVEL%"',
                  'exit /b %RC%', 'Exit code: %RC%', 'ForegroundColor Green', 'ForegroundColor Red',
                  'pause >nul', 'SURGE - %LABEL%']
        for token in tokens:
            if token not in text:
                errors.append('Missing launcher behavior ' + token + ': ' + entry['launcher'])
        if terminal_line(text) != 'exit /b %rc%':
            errors.append('Launcher masks child exit: ' + entry['launcher'])
        automation = target.read_text()
        if ('[switch]$LauncherOwnsSummary' not in automation or
                '-LauncherOwnsSummary:$LauncherOwnsSummary' not in automation):
            errors.append('Automation drops launcher summary ownership: ' + entry['automation'])
        if terminal_line(automation) != 'exit $lastexitcode':
            errors.append('Automation masks child exit: ' + entry['automation'])
        if ('-Mode "' + entry['mode'] + '"') not in automation:
            errors.append('Automation selects wrong build profile: ' + entry['automation'])
        if 'pause' in automation.lower() or 'Read-Host' in automation:
            errors.append('Automation unexpectedly interactive: ' + entry['automation'])
    return errors


if __name__ == '__main__':
    try:
        errors = check(Path(__file__).resolve().parents[1])
    except (OSError, ValueError, KeyError) as error:
        errors = [str(error)]
    for error in errors:
        print(' -', error)
    print('HUMAN_LAUNCHER_CONTRACT:', 'FAIL' if errors else 'PASS_SOURCE_STATIC_ONLY')
    raise SystemExit(1 if errors else 0)
