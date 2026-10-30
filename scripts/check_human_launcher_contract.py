"""Validate registered human entrypoints and their automation/exit-code boundary."""
import json
from pathlib import Path


def check(root):
    root = Path(root)
    entries = json.loads((root / 'metadata/HUMAN_ENTRYPOINT_REGISTRY.json').read_text())['entrypoints']
    expected = {e['launcher'] for e in entries}
    actual = {p.relative_to(root).as_posix() for p in (root / 'scripts').rglob('*.cmd')}
    errors = [] if expected == actual else ['Registered launcher set differs from actual CMD set']
    for entry in entries:
        path = root / entry['launcher']
        target = root / entry['automation']
        if not path.is_file() or not target.is_file():
            errors.append('Missing entrypoint: ' + entry['launcher'])
            continue
        text = path.read_text(encoding='ascii')
        tokens = ['%~dp0' + target.name, '%*', 'set "RC=%ERRORLEVEL%"',
                  'exit /b %RC%', 'Exit code: %RC%', 'ForegroundColor Green', 'ForegroundColor Red',
                  'pause >nul', 'SURGE - %LABEL%']
        for token in tokens:
            if token not in text:
                errors.append('Missing launcher behavior ' + token + ': ' + entry['launcher'])
        if not text.rstrip().endswith('exit /b %RC%'):
            errors.append('Launcher masks child exit: ' + entry['launcher'])
        automation = target.read_text()
        if not automation.rstrip().endswith('exit $LASTEXITCODE'):
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
