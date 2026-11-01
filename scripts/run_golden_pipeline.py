# SPDX-License-Identifier: GPL-3.0-only

"""Local Golden orchestration. Never publishes, and never substitutes fixture targets."""
import argparse
from contextlib import contextmanager
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import uuid
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


DIST_NAMES = ('preloader_l.dll', 'preloader_l.dll.sha256', 'LOCAL_DIST_RECEIPT.json')
DIST_TRANSACTION = '.local-golden-transaction'


def _is_link(path):
    """Reject symlinks and junctions where supported; not handle-anchored confinement."""
    return path.is_symlink() or (hasattr(os.path, 'isjunction') and os.path.isjunction(path))


def _sync_directory(path):
    """Best-effort directory sync (Windows directory handles are platform-specific)."""
    try:
        descriptor = os.open(path, os.O_RDONLY)
    except OSError:
        return
    try:
        try:
            os.fsync(descriptor)
        except OSError:
            pass
    finally:
        os.close(descriptor)


def _sync_file(path):
    with Path(path).open('rb') as stream:
        os.fsync(stream.fileno())


@contextmanager
def _local_dist_lock(dist):
    """Persistent lock pathname prevents the unlink/recreate inode race.

    The OS releases its actual byte/advisory lock when a writer process dies.
    """
    lock = dist / '.local-golden.lock'
    if _is_link(lock):
        raise ValueError('Unsafe local dist writer lock')
    with lock.open('a+b') as stream:
        stream.seek(0, os.SEEK_END)
        if stream.tell() == 0:
            stream.write(b'\0')
            stream.flush()
        stream.seek(0)
        if os.name == 'nt':
            import msvcrt
            msvcrt.locking(stream.fileno(), msvcrt.LK_NBLCK, 1)
        else:
            import fcntl
            fcntl.flock(stream.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        try:
            yield
        finally:
            stream.seek(0)
            if os.name == 'nt':
                msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)
            else:
                fcntl.flock(stream.fileno(), fcntl.LOCK_UN)


def _bundle_hashes(directory):
    """Return the complete receipt-bound bundle hashes or None for an empty dist."""
    paths = [directory / name for name in DIST_NAMES]
    if any(_is_link(path) or (path.exists() and not path.is_file()) for path in paths):
        raise ValueError('Unsafe local dist member')
    present = [path.exists() for path in paths]
    if not any(present):
        return None
    if not all(present):
        raise ValueError('Incomplete local dist bundle; recovery evidence required')
    hashes = {name: sha(directory / name) for name in DIST_NAMES}
    digest = hashes[DIST_NAMES[0]]
    if (paths[1].read_text(encoding='utf-8') != digest + '  ' + DIST_NAMES[0] + '\n'):
        raise ValueError('Local dist checksum sidecar mismatch')
    receipt = json.loads(paths[2].read_text(encoding='utf-8'))
    if (receipt.get('schema') != 'surge.local-dist-receipt.v1'
            or receipt.get('runtime_dll_name') != DIST_NAMES[0]
            or receipt.get('result') != 'LOCAL_GOLDEN_PASS'
            or receipt.get('sha256') != digest
            or receipt.get('official_release') is not False):
        raise ValueError('Local dist receipt mismatch')
    return hashes


def _retire_transaction(dist):
    """Deactivate the journal atomically before best-effort removal."""
    transaction = dist / DIST_TRANSACTION
    retired = dist / ('.local-golden-retired-' + uuid.uuid4().hex)
    os.replace(transaction, retired)
    _sync_directory(dist)
    try:
        shutil.rmtree(retired)
    except OSError:
        # Inactive retired data is not authoritative; a future local cleanup may remove it.
        pass


def _recover_local_dist_locked(dist):
    """Replay a complete verified previous snapshot, including interrupted replays."""
    transaction = dist / DIST_TRANSACTION
    if _is_link(transaction):
        raise ValueError('Unsafe local dist recovery transaction')
    if not transaction.exists():
        return False
    if not transaction.is_dir() or _is_link(transaction / 'recovery.json'):
        raise ValueError('Invalid local dist recovery transaction')
    plan = json.loads((transaction / 'recovery.json').read_text(encoding='utf-8'))
    if (plan.get('schema') != 'surge.local-dist-recovery.v1'
            or 'previous' not in plan):
        raise ValueError('Invalid local dist recovery record')
    previous = plan['previous']
    if previous is not None:
        if not isinstance(previous, dict) or set(previous) != set(DIST_NAMES):
            raise ValueError('Invalid local dist recovery snapshot index')
        backup = transaction / 'previous'
        if _is_link(backup):
            raise ValueError('Corrupt local dist recovery snapshot; refusing overwrite')
        try:
            backup_hashes = _bundle_hashes(backup)
        except (OSError, ValueError, UnicodeError) as error:
            raise ValueError('Corrupt local dist recovery snapshot; refusing overwrite') from error
        if backup_hashes != previous:
            raise ValueError('Corrupt local dist recovery snapshot; refusing overwrite')
    for name in DIST_NAMES:
        path = dist / name
        if _is_link(path) or (path.exists() and not path.is_file()):
            raise ValueError('Unsafe destination during local dist recovery')
    if previous is None:
        for name in DIST_NAMES:
            (dist / name).unlink(missing_ok=True)
    else:
        with tempfile.TemporaryDirectory(prefix='.recovery-', dir=dist) as td:
            staged = Path(td)
            for name in DIST_NAMES:
                shutil.copyfile(backup / name, staged / name)
                if sha(staged / name) != previous[name]:
                    raise ValueError('Recovery snapshot changed during copy')
                _sync_file(staged / name)
            _sync_directory(staged)
            for name in DIST_NAMES:
                os.replace(staged / name, dist / name)
                _sync_directory(dist)
    if _bundle_hashes(dist) != previous:
        raise ValueError('Local dist recovery verification failed; journal retained')
    _sync_directory(dist)
    _retire_transaction(dist)
    return True


def recover_local_dist(root):
    """Recover an interrupted promotion if present; never delete a live writer's lock."""
    root = Path(root).resolve()
    dist = root / 'dist'
    if _is_link(dist) or dist.resolve() != dist or (dist.exists() and not dist.is_dir()):
        raise ValueError('Local dist must be an owned directory, not a link')
    if not dist.exists():
        return False
    with _local_dist_lock(dist):
        return _recover_local_dist_locked(dist)


def stage_local_dist(root, artifact, report):
    """Promote a validated candidate with a replayable local rollback journal."""
    root, artifact = Path(root).resolve(), Path(artifact).resolve()
    if (report.get('result') != 'LOCAL_GOLDEN_PASS'
            or [s['stage'] for s in report.get('steps', [])] != ORDER
            or any(s['exit_code'] for s in report['steps'])
            or report.get('runtime_check', {}).get('exit_code') != 0):
        raise ValueError('Local dist requires all six Golden stages and final-name runtime PASS')
    contract = json.loads((root / 'metadata/LOCAL_DIST_CONTRACT.json').read_text(encoding='utf-8'))
    if contract['runtime_dll_name'] != DIST_NAMES[0] or contract['directory'] != 'dist':
        raise ValueError('Local runtime filename/directory contract drift')
    digest = sha(artifact)
    if digest != report['sha256'] or report['runtime_check']['sha256_after'] != digest:
        raise ValueError('Validated candidate hash changed before local dist')
    dist = root / 'dist'
    if _is_link(dist) or dist.resolve() != dist or (dist.exists() and not dist.is_dir()):
        raise ValueError('Local dist must be an owned directory, not a link')
    dist.mkdir(exist_ok=True)
    receipt = {'schema': 'surge.local-dist-receipt.v1', 'classification': 'NOT_RELEASE',
               'result': 'LOCAL_GOLDEN_PASS', 'runtime_dll_name': DIST_NAMES[0], 'sha256': digest,
               'input_sha256': report['input_sha256'],
               'source_manifest_sha256': sha(root / 'metadata/ACTIVE_SOURCE_MANIFEST.json'),
               'golden_artifact': str(artifact.relative_to(root)),
               'steps': [{'stage': s['stage'], 'exit_code': s['exit_code']} for s in report['steps']],
               'runtime_check': report['runtime_check'], 'official_release': False, 'library_publication': False}
    with _local_dist_lock(dist):
        _recover_local_dist_locked(dist)
        previous = _bundle_hashes(dist)
        transaction = dist / DIST_TRANSACTION
        if _is_link(transaction) or transaction.exists():
            raise ValueError('Unexpected local dist recovery transaction')
        with tempfile.TemporaryDirectory(prefix='.candidate-', dir=dist) as td:
            stage = Path(td)
            fresh = stage / 'new'
            fresh.mkdir()
            shutil.copy2(artifact, fresh / DIST_NAMES[0])
            if sha(fresh / DIST_NAMES[0]) != digest:
                raise ValueError('Local dist copy hash mismatch')
            (fresh / DIST_NAMES[1]).write_text(digest + '  ' + DIST_NAMES[0] + '\n', encoding='utf-8')
            (fresh / DIST_NAMES[2]).write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
            if _bundle_hashes(fresh) is None:
                raise ValueError('Missing staged local dist bundle')
            if previous is not None:
                backup = stage / 'previous'
                backup.mkdir()
                for name in DIST_NAMES:
                    shutil.copyfile(dist / name, backup / name)
                    if sha(backup / name) != previous[name]:
                        raise ValueError('Existing dist changed during snapshot')
                if _bundle_hashes(backup) != previous:
                    raise ValueError('Existing dist snapshot verification failed')
            (stage / 'recovery.json').write_text(json.dumps({
                'schema': 'surge.local-dist-recovery.v1', 'previous': previous
            }, sort_keys=True) + '\n', encoding='utf-8')
            for path in list(fresh.iterdir()) + list((stage / 'previous').iterdir() if previous is not None else []):
                _sync_file(path)
            _sync_file(stage / 'recovery.json')
            _sync_directory(fresh)
            if previous is not None:
                _sync_directory(stage / 'previous')
            _sync_directory(stage)
            # This single rename publishes a verified, durable-on-POSIX recovery intent
            # before the first public filename is replaced.
            os.replace(stage, transaction)
            _sync_directory(dist)
            try:
                # The receipt is last, but a process death is repaired from the journal.
                for name in DIST_NAMES:
                    os.replace(transaction / 'new' / name, dist / name)
                    _sync_directory(dist)
                if _bundle_hashes(dist)[DIST_NAMES[0]] != digest:
                    raise ValueError('Local dist post-commit verification failed')
            except BaseException:
                _recover_local_dist_locked(dist)
                raise
            _retire_transaction(dist)
    return {'dist_artifact': str(dist / DIST_NAMES[0]), 'dist_sha256_file': str(dist / DIST_NAMES[1]),
            'dist_receipt': str(dist / DIST_NAMES[2])}

def pipeline(root, dll, build_dir, output_dir):
    root, dll, build_dir, output_dir = map(lambda p: Path(p).resolve(), (root, dll, build_dir, output_dir))
    if (root / 'dist').exists() or _is_link(root / 'dist'):
        recover_local_dist(root)
    if not any(parent == root / 'build' for parent in output_dir.parents):
        raise ValueError('Golden output must be a subdirectory of this project build/')
    report = {'schema': 'surge.local-golden.v1', 'classification': 'NOT_RELEASE',
              'stage_order': ORDER, 'official_release': False, 'library_publication': False,
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
        report['result'] = 'LOCAL_GOLDEN_PASS' if len(report['steps']) == 6 and all(s['exit_code'] == 0 for s in report['steps']) else 'FAIL'
        if report['result'] == 'LOCAL_GOLDEN_PASS':
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
        if report['result'] == 'LOCAL_GOLDEN_PASS':
            target = output_dir / ('preloader_l.' + sha(candidate)[:16] + '.dll')
            shutil.copy2(candidate, target)
            sidecar = Path(str(target) + '.sha256')
            sidecar.write_text(sha(target) + '  ' + target.name + '\n', encoding='utf-8')
            report.update(artifact=str(target), sha256=sha(target), sha256_file=str(sidecar))
            try:
                report.update(stage_local_dist(root, target, report))
            except (OSError, ValueError, KeyError) as error:
                report.update(result='FAIL', dist_error=str(error))
    report['input_sha256'] = before
    return report


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--check-target', action='store_true')
    ap.add_argument('--recover-dist', action='store_true', help='Recover an interrupted local dist promotion')
    ap.add_argument('--dll', type=Path)
    ap.add_argument('--build-dir', type=Path)
    ap.add_argument('--output-dir', type=Path, default=ROOT / 'build/golden-local')
    args = ap.parse_args()
    if args.check_target:
        pending = pending_targets(ROOT)
        print(json.dumps({'result': 'BLOCKED_TARGET_SEMANTICS' if pending else 'READY_LOCAL_TARGET',
                          'pending': pending, 'official_release': False}, indent=2))
        return 3 if pending else 0
    if args.recover_dist:
        try:
            restored = recover_local_dist(ROOT)
        except (OSError, ValueError, KeyError) as error:
            print('LOCAL_DIST_RECOVERY: FAIL:', error, file=sys.stderr)
            return 1
        print(json.dumps({'recovery': 'RESTORED_PREVIOUS' if restored else 'NO_TRANSACTION',
                          'official_release': False}))
        return 0
    if not args.dll or not args.build_dir:
        ap.error('--dll and --build-dir are required')
    try:
        report = pipeline(ROOT, args.dll, args.build_dir, args.output_dir)
        args.output_dir.mkdir(parents=True, exist_ok=True)
        (args.output_dir / 'GOLDEN_REPORT.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(report, indent=2))
        return 0 if report['result'] == 'LOCAL_GOLDEN_PASS' else 3 if report['pending'] else 1
    except (OSError, ValueError, KeyError) as error:
        print('GOLDEN_PIPELINE: FAIL:', error, file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
