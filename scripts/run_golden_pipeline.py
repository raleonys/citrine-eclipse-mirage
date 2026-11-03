# SPDX-License-Identifier: GPL-3.0-only

"""Build-bound Golden verification and distribution staging."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from golden_policy import load_target

ROOT = Path(__file__).resolve().parents[1]
POLICIES = ['URSYNEX_GOLDEN_FINALIZER.json', 'ARYRA_GOLDEN_LAYOUT.json',
            'MUSTAVEX_GOLDEN_METADATA.json', 'CORVEXON_GOLDEN_TRAIL.json']
ORDER = ['MSVC Build', 'URSYNEX Golden Finalizer', 'ARYRA Golden Layout',
         'MUSTAVEX Golden Metadata', 'CORVEXON Golden Trail', 'FALCION Golden Cleaning']


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def pending_targets(root):
    errors = []
    for name in POLICIES:
        try:
            load_target(root, name)
        except (OSError, ValueError, KeyError) as error:
            errors.append(str(error))
    return errors


def execute_steps(steps, run):
    """Stop at the first failed stage; no downstream success is synthesized."""
    results = []
    for name, command in steps:
        result = run(command)
        results.append({'stage': name, **result})
        if result['exit_code'] != 0:
            break
    return results


def stage_dist(root, artifact, report):
    """Commit a byte-identical runtime DLL only after Golden and runtime PASS."""
    root, artifact = Path(root).resolve(), Path(artifact).resolve()
    if (report.get('result') != 'GOLDEN_PASS'
            or [s['stage'] for s in report.get('steps', [])] != ORDER
            or any(s['exit_code'] for s in report['steps'])
            or report.get('runtime_check', {}).get('exit_code') != 0):
        raise ValueError('Dist requires all six Golden stages and final-name runtime PASS')
    contract = json.loads((root / 'metadata/DIST_CONTRACT.json').read_text(encoding='utf-8'))
    if contract['runtime_dll_name'] != 'preloader_l.dll' or contract['directory'] != 'dist':
        raise ValueError('Runtime filename/directory contract drift')
    digest = sha(artifact)
    if digest != report['sha256'] or report['runtime_check']['sha256_after'] != digest:
        raise ValueError('Validated candidate hash changed before dist')
    dist = root / 'dist'
    if dist.is_symlink() or dist.resolve() != dist:
        raise ValueError('Dist must be an owned directory, not a link')
    dist.mkdir(exist_ok=True)
    names = ['preloader_l.dll', 'preloader_l.dll.sha256', 'DIST_RECEIPT.json']
    for name in names:
        path = dist / name
        if path.is_symlink() or (path.exists() and not path.is_file()):
            raise ValueError('Unsafe existing dist member: ' + name)
    receipt = {'schema': 'surge.dist-receipt.v1',
               'result': 'GOLDEN_PASS', 'runtime_dll_name': names[0], 'sha256': digest,
               'input_sha256': report['input_sha256'],
               'source_manifest_sha256': sha(root / 'metadata/ACTIVE_SOURCE_MANIFEST.json'),
               'golden_artifact': str(artifact.relative_to(root)),
               'steps': [{'stage': s['stage'], 'exit_code': s['exit_code']} for s in report['steps']],
               'runtime_check': report['runtime_check']}
    lock = dist / '.golden.lock'
    descriptor = os.open(lock, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    try:
        with tempfile.TemporaryDirectory(prefix='.candidate-', dir=dist) as td:
            stage = Path(td)
            shutil.copy2(artifact, stage / names[0])
            if sha(stage / names[0]) != digest:
                raise ValueError('Dist copy hash mismatch')
            (stage / names[1]).write_text(digest + '  ' + names[0] + '\n', encoding='utf-8')
            (stage / names[2]).write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
            backups = stage / 'previous'
            backups.mkdir()
            for name in names:
                if (dist / name).exists():
                    shutil.copy2(dist / name, backups / name)
            committed = []
            try:
                # Receipt is written last. On a handled failure restore the previous bundle.
                for name in names:
                    os.replace(stage / name, dist / name)
                    committed.append(name)
            except BaseException:
                for name in reversed(committed):
                    if (backups / name).exists():
                        os.replace(backups / name, dist / name)
                    else:
                        (dist / name).unlink()
                raise
    finally:
        os.close(descriptor)
        lock.unlink()
    return {'dist_artifact': str(dist / names[0]), 'dist_sha256_file': str(dist / names[1]),
            'dist_receipt': str(dist / names[2])}


def pipeline(root, dll, build_dir, output_dir):
    root, dll, build_dir, output_dir = map(lambda p: Path(p).resolve(), (root, dll, build_dir, output_dir))
    if not any(parent == root / 'build' for parent in output_dir.parents):
        raise ValueError('Golden output must be a subdirectory of this project build/')
    report = {'schema': 'surge.golden.v1',
              'stage_order': ORDER,
              'result': 'BLOCKED_TARGET_SEMANTICS', 'steps': [], 'pending': pending_targets(root)}
    if report['pending']:
        return report
    before = sha(dll)
    report['input_sha256'] = before
    child_environment = dict(os.environ, PYTHONDONTWRITEBYTECODE='1')
    output_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='candidate-', dir=output_dir) as td:
        candidate = Path(td) / 'preloader_l.dll'
        shutil.copy2(dll, candidate)
        scripts = root / 'scripts'
        steps = [
            (ORDER[0], ['check_msvc_build_evidence.py', str(build_dir), str(dll)]),
            (ORDER[1], ['ursynex_golden_finalizer.py', str(candidate)]),
            (ORDER[2], ['verify_aryra_golden_layout.py', str(candidate)]),
            (ORDER[3], ['verify_mustavex_golden_metadata.py', str(candidate)]),
            (ORDER[4], ['verify_corvexon_golden_trail.py', str(candidate), '--build-dir', str(build_dir)]),
            (ORDER[5], ['check_falcion_golden_cleaning.py', '--root', str(root)])]
        index = 0
        def run(command):
            nonlocal index
            old_sha = sha(candidate)
            try:
                cp = subprocess.run([sys.executable, '-B', str(scripts / command[0]), *command[1:]],
                                    cwd=root, text=True, capture_output=True, timeout=120, env=child_environment)
                result = {'exit_code': cp.returncode, 'output': cp.stdout + cp.stderr}
            except subprocess.TimeoutExpired:
                result = {'exit_code': 124, 'output': 'Golden stage timed out'}
            new_sha = sha(candidate)
            if index != 1 and new_sha != old_sha:
                result = {'exit_code': 1, 'output': 'Read-only Golden stage modified candidate bytes'}
            result.update(sha256_before=old_sha, sha256_after=new_sha)
            print(f'[{index+1}/6] {ORDER[index]}: ' + ('PASS' if result['exit_code'] == 0 else 'FAIL'), flush=True)
            index += 1
            return result
        report['steps'] = execute_steps(steps, run)
        report['result'] = 'GOLDEN_PASS' if len(report['steps']) == 6 and all(s['exit_code'] == 0 for s in report['steps']) else 'FAIL'
        if report['result'] == 'GOLDEN_PASS':
            candidate_before = sha(candidate)
            try:
                cp = subprocess.run([str(build_dir / 'preloader_l_runtime_host.exe'), str(candidate)],
                                    cwd=root, text=True, capture_output=True, timeout=120, env=child_environment)
                runtime = {'exit_code': cp.returncode, 'output': cp.stdout + cp.stderr}
            except subprocess.TimeoutExpired:
                runtime = {'exit_code': 124, 'output': 'Final-name runtime check timed out'}
            except OSError as error:
                runtime = {'exit_code': 127, 'output': str(error)}
            runtime.update(sha256_before=candidate_before, sha256_after=sha(candidate), runtime_dll_name=candidate.name)
            if runtime['sha256_after'] != candidate_before:
                runtime.update(exit_code=1, output='Runtime check modified the candidate DLL')
            report['runtime_check'] = runtime
            print('Final-name runtime check: ' + ('PASS' if runtime['exit_code'] == 0 else 'FAIL'), flush=True)
            if runtime['exit_code'] != 0:
                report['result'] = 'FAIL'
        if sha(dll) != before:
            report['result'] = 'FAIL'
            report['errors'] = ['Original build DLL changed during Golden execution']
        if report['result'] == 'GOLDEN_PASS':
            target = output_dir / ('preloader_l.' + sha(candidate)[:16] + '.dll')
            shutil.copy2(candidate, target)
            sidecar = Path(str(target) + '.sha256')
            sidecar.write_text(sha(target) + '  ' + target.name + '\n', encoding='utf-8')
            report.update(artifact=str(target), sha256=sha(target), sha256_file=str(sidecar))
            try:
                report.update(stage_dist(root, target, report))
            except (OSError, ValueError, KeyError) as error:
                report.update(result='FAIL', dist_error=str(error))
    report['input_sha256'] = before
    return report


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--check-target', action='store_true')
    ap.add_argument('--dll', type=Path)
    ap.add_argument('--build-dir', type=Path)
    ap.add_argument('--output-dir', type=Path, default=ROOT / 'build/golden')
    args = ap.parse_args()
    if args.check_target:
        pending = pending_targets(ROOT)
        print(json.dumps({'result': 'BLOCKED_TARGET_SEMANTICS' if pending else 'READY_TARGET',
                          'pending': pending}, indent=2))
        return 3 if pending else 0
    if not args.dll or not args.build_dir:
        ap.error('--dll and --build-dir are required')
    try:
        report = pipeline(ROOT, args.dll, args.build_dir, args.output_dir)
        args.output_dir.mkdir(parents=True, exist_ok=True)
        (args.output_dir / 'GOLDEN_REPORT.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(report, indent=2))
        return 0 if report['result'] == 'GOLDEN_PASS' else 3 if report['pending'] else 1
    except (OSError, ValueError, KeyError) as error:
        print('GOLDEN_PIPELINE: FAIL:', error, file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
