# SPDX-License-Identifier: GPL-3.0-only

"""Prepare source-bound release candidates; only draft GitHub releases may be written."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import xml.etree.ElementTree as ET
import zipfile
from weekly_naming import FIRST, successor, approved_tag, require_tag, asset_names, validate_registry, load_registry

from check_falcion_golden_cleaning import check
from package_source import package
from run_golden_pipeline import ORDER

ROOT = Path(__file__).resolve().parents[1]
COMMIT = re.compile(r'[0-9a-f]{40}\Z')
MARKER = re.compile(r'<!-- surge-weekly:(\{[^\n]*\}) -->')


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def load(path):
    return json.loads(Path(path).read_text(encoding='utf-8-sig'))


def write(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')


def head(root=ROOT):
    return subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()


def state(release):
    matches = MARKER.findall(release.get('body') or '')
    if not matches:
        return None
    if len(matches) != 1:
        raise ValueError('Ambiguous candidate source marker')
    value = json.loads(matches[0])
    if value.get('schema') != 1 or not COMMIT.fullmatch(value.get('source_sha', '')) or value.get('status') not in ('preparing', 'complete'):
        raise ValueError('Invalid candidate source marker')
    require_tag(release['tag_name'])
    return value


def marker(source, status):
    return '<!-- surge-weekly:' + json.dumps(
        {'schema': 1, 'source_sha': source, 'status': status}, separators=(',', ':')) + ' -->'


def plan(releases, source, start_tag=''):
    if not COMMIT.fullmatch(source):
        raise ValueError('Expected full source commit')
    validate_registry(load_registry())
    tracked = [(r, state(r)) for r in releases]
    tracked = [(r, s) for r, s in tracked if s is not None]
    result = {'source_sha': source, 'ready': False, 'tag': '', 'existing_id': None}
    if tracked:
        last, status = max(tracked, key=lambda pair: tuple(map(int, require_tag(pair[0]['tag_name']).split('.'))))
        tag = last['tag_name']
        if status['source_sha'] == source:
            if start_tag and start_tag != tag:
                raise ValueError('Same source cannot be repackaged under a new version')
            if status['status'] == 'complete':
                return {**result, 'reason': 'NO_NEW_COMMITS'}
            if not last['draft']:
                raise ValueError('Incomplete draft already published')
            return {**result, 'ready': True, 'tag': tag, 'existing_id': last['id']}
        version = successor(require_tag(tag))
    else:
        version = FIRST
    tag = approved_tag(version)
    if start_tag and start_tag != tag:
        raise ValueError('start_tag must match exact next approved semantic version')
    if any(r['tag_name'] == tag for r in releases):
        raise ValueError('Tag reserved by unrelated release')
    return {**result, 'ready': True, 'tag': tag}


def junit(path):
    tree = ET.parse(path).getroot()
    cases = list(tree.iter('testcase'))
    if not cases or any(node.tag in ('failure', 'error', 'skipped') for node in tree.iter()):
        raise ValueError('CTest evidence is empty, failed or skipped')
    for node in tree.iter():
        for key in ('failures', 'errors', 'skipped', 'disabled'):
            if int(node.attrib.get(key, '0')):
                raise ValueError('CTest reports an incomplete test run')
    return len(cases)


def validate_profile(folder, profile, manifest, source):
    folder = Path(folder)
    identity = load(folder / 'EVIDENCE_IDENTITY.json')
    if identity != {'source_sha': source, 'source_manifest_sha256': manifest, 'profile': profile}:
        raise ValueError('Evidence belongs to a different source or profile')
    receipt = load(folder / 'BUILD_RECEIPT.json')
    mode = 'Native' if profile == 'native' else 'Golden'
    if (receipt.get('mode') != mode or receipt.get('configuration') != 'Release'
            or receipt.get('build_succeeded') is not True
            or receipt.get('validation_result') != 'PASS_SELECTED_PROFILE'
            or receipt.get('validation_exit_code') != 0
            or receipt.get('source_manifest_sha256') != manifest):
        raise ValueError('A successful exact-source native MSVC profile is required')
    if not receipt.get('compiler', '').lower().endswith('cl.exe'):
        raise ValueError('Candidate requires the native MSVC compiler')
    for name in ('BUILD_TOOLCHAIN_PROVENANCE.log', 'PUBLIC_DLL_ABI_CHECK.json', 'DLL_RESOURCE_CHECK.json'):
        if not (folder / name).is_file():
            raise ValueError('Missing build evidence: ' + name)
    if profile == 'native':
        return {'tests': junit(folder / 'CTEST_RESULTS.xml')}
    dll = folder / 'dist/preloader_l.dll'
    digest = sha(dll)
    verify_sidecar(dll)
    dist = load(folder / 'dist/DIST_RECEIPT.json')
    golden = load(folder / 'GOLDEN_REPORT.json')
    for report in (dist, golden):
        if (report.get('result') != 'GOLDEN_PASS' or report.get('sha256') != digest
                or [s['stage'] for s in report.get('steps', [])] != ORDER
                or any(s['exit_code'] for s in report['steps'])):
            raise ValueError('All six Golden gates must pass for this exact DLL')
        runtime = report.get('runtime_check', {})
        if (runtime.get('exit_code') != 0 or runtime.get('sha256_before') != digest
                or runtime.get('sha256_after') != digest):
            raise ValueError('Final-name DLL runtime acceptance is missing')
    if (dist.get('source_manifest_sha256') != manifest
            or dist.get('input_sha256') != receipt.get('dll_sha256')
            or receipt.get('dist_sha256') != digest):
        raise ValueError('Golden DLL/source/build hash mismatch')
    return {'dll_sha256': digest}


def verify_sidecar(path):
    path = Path(path)
    expected = sha(path) + '  ' + path.name
    if Path(str(path) + '.sha256').read_text(encoding='utf-8').strip() != expected:
        raise ValueError('Checksum mismatch: ' + path.name)


def sidecar(path):
    path = Path(path)
    Path(str(path) + '.sha256').write_text(sha(path) + '  ' + path.name + '\n', encoding='utf-8')


def capture(root, profile, build, output, source):
    root, build, output = map(lambda p: Path(p).resolve(), (root, build, output))
    if root / 'build' not in build.parents or output == root or root in output.parents:
        raise ValueError('Use an owned build directory and an external evidence output')
    if head(root) != source:
        raise ValueError('Checkout does not match planned source')
    folder = output / profile
    folder.mkdir(parents=True, exist_ok=False)
    for name in ('BUILD_RECEIPT.json', 'BUILD_TOOLCHAIN_PROVENANCE.log',
                 'PUBLIC_DLL_ABI_CHECK.json', 'DLL_RESOURCE_CHECK.json'):
        shutil.copyfile(build / name, folder / name)
    if profile == 'native':
        shutil.copyfile(build / 'CTEST_RESULTS.xml', folder / 'CTEST_RESULTS.xml')
    else:
        shutil.copyfile(build / 'golden/GOLDEN_REPORT.json', folder / 'GOLDEN_REPORT.json')
        dist = folder / 'dist'
        dist.mkdir()
        for name in ('preloader_l.dll', 'preloader_l.dll.sha256', 'DIST_RECEIPT.json'):
            shutil.copyfile(root / 'dist' / name, dist / name)
    manifest = sha(root / 'metadata/ACTIVE_SOURCE_MANIFEST.json')
    write(folder / 'EVIDENCE_IDENTITY.json',
          {'source_sha': source, 'source_manifest_sha256': manifest, 'profile': profile})
    validate_profile(folder, profile, manifest, source)


def bundle(root, payload, output):
    root, payload, output = map(lambda p: Path(p).resolve(), (root, payload, output))
    if output == root or root in output.parents or output.exists():
        raise ValueError('Use a fresh external candidate directory')
    p = load(payload / 'plan.json')
    require_tag(p['tag'])
    if not p['ready'] or p['source_sha'] != head(root):
        raise ValueError('Candidate checkout does not match planned source')
    manifest = sha(root / 'metadata/ACTIVE_SOURCE_MANIFEST.json')
    qa = load(payload / 'SOURCE_QA.json')
    if qa.get('result') != 'PASS_SOURCE_QA' or not qa.get('checks') or any(
            c['exit_code'] != 0 for c in qa['checks']):
        raise ValueError('Source QA did not pass')
    portable_tests = junit(payload / 'PORTABLE_CTEST.xml')
    native = validate_profile(payload / 'native', 'native', manifest, p['source_sha'])
    golden = validate_profile(payload / 'production', 'production', manifest, p['source_sha'])
    source_name, binary_name = asset_names(p['tag'])
    source = payload / source_name
    verify_sidecar(source)
    errors = check(root, source)
    if errors:
        raise ValueError('Source package mismatch: ' + '; '.join(errors))
    output.mkdir(parents=True)
    shutil.copyfile(source, output / source.name)
    shutil.copyfile(Path(str(source) + '.sha256'), output / (source.name + '.sha256'))
    receipt = {'schema': 'surge.draft-release-receipt.v1', 'publication_status': 'DRAFT',
               'tag': p['tag'], 'source_sha': p['source_sha'], 'source_manifest_sha256': manifest,
               'source_qa_groups': len(qa['checks']), 'portable_tests': portable_tests,
               'native_tests': native['tests'], 'dll_sha256': golden['dll_sha256'],
               'source_zip_sha256': sha(source), 'game_acceptance': 'NOT_RUN',
               'publication': False}
    write(output / 'RELEASE_RECEIPT.json', receipt)
    binary = output / binary_name
    files = {path.name: path for path in (payload / 'production/dist').iterdir() if path.is_file()}
    if set(files) != {'preloader_l.dll', 'preloader_l.dll.sha256', 'DIST_RECEIPT.json'}:
        raise ValueError('Unexpected runtime payload')
    files['RELEASE_RECEIPT.json'] = output / 'RELEASE_RECEIPT.json'
    files['LICENSE'] = root / 'LICENSE'
    for vendor, license_file in (('asio', 'LICENSE_1_0.txt'), ('minhook', 'LICENSE.txt'),
                                 ('pegtl', 'LICENSE_1_0.txt'), ('pugixml', 'LICENSE.md')):
        files['licenses/' + vendor + '.txt'] = root / 'third_party' / vendor / license_file
    for profile in ('native', 'production'):
        for path in (payload / profile).rglob('*'):
            if path.is_file() and 'dist' not in path.relative_to(payload / profile).parts:
                files['evidence/' + path.relative_to(payload).as_posix()] = path
    for name in ('SOURCE_QA.json', 'PORTABLE_CTEST.xml'):
        files['evidence/' + name] = payload / name
    with zipfile.ZipFile(binary, 'w', compression=zipfile.ZIP_DEFLATED) as archive:
        for name, path in sorted(files.items()):
            if path.is_symlink():
                raise ValueError('Candidate payload cannot contain symlinks')
            archive.writestr(name, path.read_bytes())
    sidecar(binary)
    sidecar(output / 'RELEASE_RECEIPT.json')
    return receipt


class GitHub:
    def __init__(self, repository):
        if not re.fullmatch(r'[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+', repository):
            raise ValueError('Invalid repository')
        self.repository = repository

    def api(self, endpoint, data=None, paginate=False):
        args = ['gh', 'api', 'repos/' + self.repository + '/' + endpoint]
        if paginate:
            args += ['--paginate']
        if data is not None:
            args += ['--method', 'POST', '--input', '-']
        cp = subprocess.run(args, input=json.dumps(data) if data is not None else None,
                            text=True, capture_output=True, timeout=120)
        if cp.returncode:
            raise RuntimeError('GitHub API request failed: ' + cp.stderr.strip())
        if not paginate:
            return json.loads(cp.stdout)
        # Older supported gh versions stream JSON pages without --slurp.
        pages, remaining = [], cp.stdout.lstrip()
        decoder = json.JSONDecoder()
        while remaining:
            value, end = decoder.raw_decode(remaining)
            if not isinstance(value, list):
                raise ValueError('Expected a paginated GitHub list')
            pages.append(value)
            remaining = remaining[end:].lstrip()
        return pages

    def releases(self):
        return [r for page in self.api('releases?per_page=100', paginate=True) for r in page]

    def update(self, release_id, body):
        cp = subprocess.run(['gh', 'api', '--method', 'PATCH',
                             f'repos/{self.repository}/releases/{release_id}', '--input', '-'],
                            input=json.dumps({'body': body}),
                            text=True, capture_output=True, check=True, timeout=120)
        return json.loads(cp.stdout)

    def upload(self, tag, paths):
        subprocess.run(['gh', 'release', 'upload', tag, '--repo', self.repository,
                        '--clobber', *map(str, paths)], check=True, timeout=300)


def publish_draft(github, p, assets):
    assets = Path(assets)
    receipt = load(assets / 'RELEASE_RECEIPT.json')
    tag = p['tag']
    require_tag(tag)
    if (not p['ready'] or receipt['source_sha'] != p['source_sha'] or receipt['tag'] != tag
            or receipt.get('publication_status') != 'DRAFT'
            or receipt.get('publication') is not False
            or receipt.get('game_acceptance') != 'NOT_RUN'):
        raise ValueError('Draft receipt/source mismatch')
    paths = sorted(assets.iterdir())
    source_name, binary_name = asset_names(tag)
    expected = {source_name, source_name + '.sha256', binary_name, binary_name + '.sha256'}
    expected |= {'RELEASE_RECEIPT.json', 'RELEASE_RECEIPT.json.sha256'}
    if {path.name for path in paths} != expected:
        raise ValueError('Unexpected release assets')
    for name in (source_name, binary_name, 'RELEASE_RECEIPT.json'):
        verify_sidecar(assets / name)
    releases = github.releases()
    existing = next((r for r in releases if r['tag_name'] == tag), None)
    if existing and (not existing['draft'] or not state(existing)
                     or state(existing)['source_sha'] != p['source_sha']
                     or existing.get('target_commitish') != p['source_sha']):
        raise ValueError('Refusing to modify a published or unrelated release')
    complete = existing and state(existing)['status'] == 'complete'
    if complete:
        return existing
    if not existing:
        refs = github.api('git/matching-refs/tags/' + tag)
        if any(ref['ref'] == 'refs/tags/' + tag for ref in refs):
            raise ValueError('Refusing to reuse an existing Git tag')
    notes = github.api('releases/generate-notes', {'tag_name': tag, 'target_commitish': p['source_sha']})
    summary = ('# Draft candidate\n\nSource: `' + p['source_sha'] + '`\n\n'
               'Source QA, portable tests, native MSVC acceptance, and Production/Golden '
               'passed for this source. Game acceptance is NOT RUN. '
               'The owner must review and publish this draft.\n\n')
    preparing = marker(p['source_sha'], 'preparing') + '\n\n**Asset preparation is incomplete; do not publish yet.**\n\n'
    if not existing:
        existing = github.api('releases', {'tag_name': tag, 'target_commitish': p['source_sha'],
                              'name': tag, 'draft': True, 'prerelease': False,
                              'body': preparing + summary + notes['body']})
    release_id = existing['id']
    latest = github.api(f'releases/{release_id}')
    if not latest['draft']:
        raise ValueError('Candidate was published during preparation; refusing to overwrite')
    github.upload(tag, paths)
    latest = github.api(f'releases/{release_id}')
    if not latest['draft']:
        raise ValueError('Candidate was published during upload; manual review is required')
    remote = {a['name']: a for a in latest['assets']}
    if set(remote) != expected:
        raise ValueError('Incomplete or unexpected remote asset set; draft remains incomplete')
    for path in paths:
        asset = remote[path.name]
        if asset['size'] != path.stat().st_size:
            raise ValueError('Remote asset size mismatch')
        if asset.get('digest') and asset['digest'] != 'sha256:' + sha(path):
            raise ValueError('Remote asset hash mismatch')
    result = github.update(release_id, marker(p['source_sha'], 'complete') + '\n\n' + summary + notes['body'])
    if not result['draft']:
        raise ValueError('Candidate was published concurrently; manual review is required')
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    sub = ap.add_subparsers(dest='command', required=True)
    planning = sub.add_parser('plan')
    planning.add_argument('--output', type=Path, required=True)
    planning.add_argument('--start-tag', default='')
    source = sub.add_parser('source')
    source.add_argument('--plan', type=Path, required=True)
    source.add_argument('--output', type=Path, required=True)
    evidence = sub.add_parser('capture')
    evidence.add_argument('--profile', choices=('native', 'production'), required=True)
    evidence.add_argument('--build', type=Path, required=True)
    evidence.add_argument('--output', type=Path, required=True)
    evidence.add_argument('--source', required=True)
    packing = sub.add_parser('bundle')
    packing.add_argument('--payload', type=Path, required=True)
    packing.add_argument('--output', type=Path, required=True)
    publishing = sub.add_parser('draft')
    publishing.add_argument('--plan', type=Path, required=True)
    publishing.add_argument('--assets', type=Path, required=True)
    args = ap.parse_args()
    if subprocess.check_output(['git', 'status', '--porcelain', '--untracked-files=all'], cwd=ROOT):
        raise ValueError('Release preparation requires a clean, committed checkout')
    if args.command == 'plan':
        github = GitHub(os.environ['GITHUB_REPOSITORY'])
        p = plan(github.releases(), head(), args.start_tag)
        if p['ready'] and p['existing_id'] is None:
            refs = github.api('git/matching-refs/tags/' + p['tag'])
            if any(ref['ref'] == 'refs/tags/' + p['tag'] for ref in refs):
                raise ValueError('Candidate tag already exists; select an unused start_tag')
        write(args.output, p)
        if 'GITHUB_OUTPUT' in os.environ:
            with open(os.environ['GITHUB_OUTPUT'], 'a', encoding='utf-8') as out:
                out.write('ready=' + str(p['ready']).lower() + '\nsource=' + p['source_sha'] + '\n')
        print(json.dumps(p))
    elif args.command == 'source':
        p = load(args.plan)
        if not p['ready'] or p['source_sha'] != head():
            raise ValueError('Source plan is stale')
        args.output.mkdir(parents=True, exist_ok=False)
        shutil.copyfile(args.plan, args.output / 'plan.json')
        shutil.copyfile(ROOT / 'build/qa/weekly-source-qa.json', args.output / 'SOURCE_QA.json')
        shutil.copyfile(ROOT / 'build/weekly-portable/CTEST_RESULTS.xml', args.output / 'PORTABLE_CTEST.xml')
        source_name, _ = asset_names(p['tag'])
        package(ROOT, args.output / source_name)
    elif args.command == 'capture':
        capture(ROOT, args.profile, args.build, args.output, args.source)
    elif args.command == 'bundle':
        print(json.dumps(bundle(ROOT, args.payload, args.output)))
    else:
        p = load(args.plan)
        if head() != p['source_sha']:
            raise ValueError('Publishing checkout does not match validated source')
        result = publish_draft(GitHub(os.environ['GITHUB_REPOSITORY']), p, args.assets)
        print(json.dumps({'draft_url': result['html_url'], 'draft': result['draft']}))


if __name__ == '__main__':
    main()
