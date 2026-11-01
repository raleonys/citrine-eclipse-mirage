#!/usr/bin/env python3
"""Release-independent, source-only contract checker.

Reads the release identity/ordinal/parent/revision from active source metadata.
Never invokes G0, reads operator credentials, approves an artifact, runs G4, or
updates CURRENT_ARTIFACT. G0/G2/G4 checks remain separate publication gates.
Older release-specific scripts are immutable historical evidence, not active QA.
"""
from __future__ import annotations
import hashlib
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SHA = re.compile(r'[0-9a-f]{64}\Z')
IDENTITY = re.compile(r'(?P<product>preloader_l) (?P<version>[0-9]+\.[0-9]+\.[0-9]+) (?P<codename>[A-Z][A-Z0-9]+) HOTFIX (?P<ordinal>[1-9][0-9]*)\Z')


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_json(root: Path, path: str) -> dict:
    obj = json.loads((root / path).read_text(encoding='utf-8'))
    if not isinstance(obj, dict):
        raise ValueError('not a JSON object: ' + path)
    return obj


def validate(root: Path = ROOT) -> list[str]:
    failures: list[str] = []

    def require(label: str, condition: bool) -> None:
        if not condition:
            failures.append(label)

    try:
        s = read_json(root, 'RELEASE_STATE.json')
        a = read_json(root, 'metadata/RELEASE_AUTHORIZATION.json')
        p = read_json(root, 'metadata/PUBLICATION_TRANSACTION.json')
        g = read_json(root, 'metadata/GOVERNANCE_BINDING.json')
        art = read_json(root, 'metadata/ARTIFACT_PROVENANCE.json')
        v = read_json(root, 'metadata/PROJECT_GOVERNANCE_INVARIANTS.json')
        acc = read_json(root, 'metadata/RELEASE_ACCEPTANCE.json')
        ledger = read_json(root, 'metadata/ACCEPTANCE_FRESHNESS_LEDGER.json')
        arch = read_json(root, 'metadata/RELEASE_ARCHITECTURE.json')
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        return ['metadata-missing-or-malformed:' + type(exc).__name__]

    identity = s.get('official_release_identity')
    match = IDENTITY.fullmatch(identity) if isinstance(identity, str) else None
    require('canonical-identity-grammar', match is not None)
    if not match:
        return failures
    ordinal = int(match.group('ordinal'))
    predecessor = identity[:match.start('ordinal')] + str(ordinal - 1)
    filename = f"{match.group('product')}_{match.group('version')}_{match.group('codename')}_HOTFIX_{ordinal}.zip"
    parent = s.get('immediate_hotfix_parent', {})
    parent_sha = parent.get('sha256') if isinstance(parent, dict) else None
    require('valid-ordinal-and-state', ordinal > 1 and s.get('hotfix_ordinal') == ordinal and s.get('release_type') == 'HOTFIX_RELEASE')
    require('correct-parent-identity', isinstance(parent, dict) and parent.get('release') == predecessor)
    require('parent-sha-present', isinstance(parent_sha, str) and SHA.fullmatch(parent_sha) is not None)

    identity_docs = (s, a, p, g, art, acc, v, arch)
    require('identity-consistency', all(doc.get('official_release_identity') == identity for doc in identity_docs))
    typed_docs = (s, a, g, art, acc, v, arch)
    require('release-class-consistency', all(doc.get('release_type') == 'HOTFIX_RELEASE' for doc in typed_docs))
    require('ordinal-consistency', a.get('requested_ordinal') == ordinal)
    require('package-name-consistency', s.get('official_package_filename') == filename and p.get('official_package_filename') == filename and art.get('official_filename') == filename)
    provenance_parent = art.get('immediate_parent', {})
    canonical_before = p.get('canonical_before', {})
    require('parent-provenance-consistency', provenance_parent.get('identity') == predecessor and provenance_parent.get('sha256') == parent_sha and provenance_parent.get('class') == 'HOTFIX_RELEASE')
    require('unmodified-canonical-parent', canonical_before.get('identity') == predecessor and canonical_before.get('sha256') == parent_sha)
    require('no-false-publication', s.get('canonical_artifact_pointer_commit') is False and p.get('canonical_pointer_committed') is False and p.get('post_commit_verified') is False and s.get('publication_date') is None)
    require('authorization-consistency', all(isinstance(doc.get('publication_authorized'), bool) for doc in (a, p, g, acc)) and len({doc['publication_authorized'] for doc in (a, p, g, acc)}) == 1)
    require('source-development-authorization-field', isinstance(a.get('source_development_authorized'), bool))

    revision = s.get('governance_master_revision')
    md_sha = s.get('governance_master_md_sha256')
    json_sha = s.get('governance_master_json_sha256')
    require('governance-version-and-pins', isinstance(revision, str) and bool(revision) and revision == g.get('master_revision') and isinstance(md_sha, str) and SHA.fullmatch(md_sha) is not None and isinstance(json_sha, str) and SHA.fullmatch(json_sha) is not None and g.get('library_master_md_sha256') == md_sha and g.get('library_master_json_sha256') == json_sha)
    try:
        md = (root / 'docs/PROJECT_GOVERNANCE_CONTRACT.md').read_bytes()
        inv = (root / 'metadata/PROJECT_GOVERNANCE_INVARIANTS.json').read_bytes()
        require('normative-md-sha', digest(md) == g.get('normative_governance_mirror_sha256') and digest(md) == g.get('packaged_mirror', {}).get('md_sha256'))
        require('normative-json-sha', digest(inv) == g.get('packaged_mirror', {}).get('json_sha256'))
        require('library-debt-not-packaged', b'## 20. Library-only governance debt' not in md and 'library_governance_debt' not in v)
    except OSError:
        failures.append('normative-mirror-missing')

    require('hotfix-g3-not-applicable', acc.get('release_class_g3', {}).get('status') == 'NOT_APPLICABLE' and v.get('governance_first_protocol', {}).get('gate_applicability', {}).get('G3', {}).get('not_applicable_status') == 'NOT_APPLICABLE')
    require('source-qa-not-falsely-accepted', isinstance(acc.get('source_qa', {}).get('status'), str) and (acc['source_qa']['status'].startswith('PENDING') or acc['source_qa']['status'].startswith('NOT_RUN')))
    entries = ledger.get('entries', [])
    require('acceptance-ledger-not-carried-forward', isinstance(entries, list) and len(entries) >= 8 and all(item.get('status') == 'PENDING' and item.get('carry_forward') is None and item.get('binary_sha256') is None for item in entries))
    require('acceptance-ledger-current-identity', ledger.get('release_identity') == identity)
    require('no-fake-game-acceptance', all(acc.get(key, {}).get('status', '').startswith('NOT_RUN') for key in ('collegefb27_game_network', 'madden27_game_network')))
    require('minhook-metadata-valid', isinstance(arch.get('minhook', {}).get('detours'), int) and arch['minhook']['detours'] > 0)
    require('release-document-identity', all((root / name).read_text(encoding='utf-8').startswith(prefix) for name, prefix in {
        'README.md': '# ' + identity,
        'docs/BUILD_ID.md': '# BUILD ID — ' + identity,
        'docs/RELEASE_NOTES.md': '# ' + identity,
        'docs/RELEASE_ACCEPTANCE.md': '# ' + identity,
    }.items()))
    require('release-build-id', filename in (root / 'docs/BUILD_ID.md').read_text(encoding='utf-8'))
    return failures


def main() -> int:
    failures = validate()
    if failures:
        print('ACTIVE_RELEASE_CONTRACT: FAIL', len(failures))
        for f in failures:
            print(' -', f)
        return 1
    print('ACTIVE_RELEASE_CONTRACT: PASS_SOURCE_STRUCTURE_ONLY; G0/G2/G4 NOT CLAIMED')
    return 0


if __name__ == '__main__':
    sys.exit(main())
