"""Visible local source QA, independent of publication and Windows availability."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
CHECKS = [
    ('Wine test process identity and exit propagation', ['tests/infrastructure/test_wine_runner.py']),
    ('source bytes', ['scripts/verify_sources.py']),
    ('four active supplied vendor trees', ['scripts/vendor_sync.py','--root',str(ROOT),'--verify-installed']),
    ('vendor tamper and link packaging regressions', ['tests/infrastructure/test_vendor_contract.py']),
    ('repository and resource ownership', ['scripts/check_repository_contract.py']),
    ('independent repository root allowlist regression', ['tests/infrastructure/test_root_ownership.py']),
    ('owner integration requirements', ['scripts/check_integration_requirements.py']),
    ('metadata ownership', ['scripts/check_metadata_ownership.py']),
    ('production/reference build boundary regressions', ['tests/infrastructure/test_build_partition.py']),
    ('native size evidence reader regressions', ['tests/infrastructure/test_size_evidence.py']),
    ('weekly draft release regressions', ['tests/infrastructure/test_weekly_release.py']),
    ('operator entrypoints', ['scripts/check_human_launcher_contract.py']),
    ('whole-source identity', ['scripts/check_manifest.py']),
    ('FALCION source boundary', ['scripts/check_falcion_golden_cleaning.py']),
    ('Golden PE negative regressions', ['tests/infrastructure/test_pe_metadata_tools.py']),
    ('restricted named imports', ['tests/infrastructure/test_corvexon_mswsock_imports.py']),
    ('MSVC link negative regressions', ['scripts/check_msvc_shipping_link_command.py', '--self-test']),
    ('first-party language audit regressions', ['tests/infrastructure/test_english_only_audit.py']),
    ('first-party English-only prose gate', ['scripts/check_english_only.py', '--check']),
    ('SURGE infrastructure regressions', ['tests/infrastructure/test_infrastructure.py']),
    ('SURGE directive regressions', ['tests/infrastructure/test_requirements.py'])]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--report', type=Path)
    args = ap.parse_args()
    results = []
    for index, (name, command) in enumerate(CHECKS, 1):
        start = time.monotonic()
        try:
            cp = subprocess.run([sys.executable, '-B', str(ROOT / command[0]), *command[1:]],
                                cwd=ROOT, capture_output=True, text=True, timeout=90)
            result = {'check': name, 'exit_code': cp.returncode, 'output': cp.stdout + cp.stderr}
        except subprocess.TimeoutExpired:
            result = {'check': name, 'exit_code': 124, 'output': 'QA timeout'}
        result['seconds'] = round(time.monotonic() - start, 3)
        results.append(result)
        print(f'[{index}/{len(CHECKS)}] {name}: ' + ('PASS' if result['exit_code'] == 0 else 'FAIL'), flush=True)
        if result['exit_code']:
            print(result['output'], flush=True)
    passed = all(r['exit_code'] == 0 for r in results)
    report = {'schema': 'surge.source-qa.v1', 'result': 'PASS_SOURCE_QA' if passed else 'FAIL',
              'native_game_acceptance': 'NOT_RUN', 'checks': results}
    if args.report:
        path = args.report.resolve()
        if ROOT / 'build' not in path.parents:
            ap.error('--report must be inside project build/ to avoid changing the source manifest')
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print('SOURCE_QA:', report['result'])
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
