#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
from pathlib import Path
import re
import sys
import tempfile


def fail(messages):
    print('MSVC_SHIPPING_LINK_POLICY: FAIL')
    for m in messages:
        print(' -', m)
    return 1


def _safe_under(root: Path, candidate: Path):
    try:
        resolved = candidate.resolve()
        resolved.relative_to(root.resolve())
        return resolved
    except Exception:
        return None


def normalized_link_source(build_dir: Path):
    """Return the target-local source that CMake generated for citrine_eclipse_mirage linking."""
    root = build_dir.resolve()
    target_dir = root / 'CMakeFiles' / 'citrine_eclipse_mirage.dir'
    link_txt = target_dir / 'link.txt'
    if link_txt.is_file():
        return link_txt.resolve(), 'link.txt', []

    build_make = target_dir / 'build.make'
    if build_make.is_file():
        return build_make.resolve(), 'build.make', []

    candidates = []
    for name in ('link.txt', 'build.make'):
        for p in root.rglob(name):
            safe = _safe_under(root, p)
            if safe is None:
                continue
            parts = [x.lower() for x in safe.parts]
            if 'citrine_eclipse_mirage.dir' in parts:
                candidates.append((safe, name))
    unique = []
    seen = set()
    for p, kind in candidates:
        if p not in seen:
            seen.add(p)
            unique.append((p, kind))
    if len(unique) == 1:
        p, kind = unique[0]
        return p, kind, []
    if not unique:
        return None, None, [
            f'normalized citrine_eclipse_mirage link rule not found under: {root} '
            '(expected target-local link.txt or build.make)'
        ]
    return None, None, [
        'ambiguous citrine_eclipse_mirage link rule: ' + ', '.join(str(p) for p, _ in unique)
    ]


def _looks_like_link_invocation(line: str):
    return re.search(r'(?i)(?:^|[\\/\s"])(?:link|lld-link)(?:\.exe)?(?:["\s]|$)', line) is not None


def _extract_invocation(source: Path, kind: str):
    """Return only the selected target link invocation and its working directory hint.

    build.make can contain unrelated commands and stale response-file names.  Do not
    treat those as link evidence.  For NMake inline responses, include only the
    @<< payload owned by the selected link recipe.
    """
    text = source.read_text(errors='replace')
    if kind == 'link.txt':
        lines = [x for x in text.splitlines() if x.strip()]
        matches = [x for x in lines if _looks_like_link_invocation(x)]
        if len(matches) != 1:
            return None, None, [f'expected exactly one linker invocation in {source}, found {len(matches)}']
        return matches[0], None, []

    lines = text.splitlines()
    indices = [i for i, line in enumerate(lines) if _looks_like_link_invocation(line)]
    if len(indices) != 1:
        return None, None, [f'expected exactly one linker invocation in {source}, found {len(indices)}']
    i = indices[0]
    invocation = [lines[i]]
    if '@<<' in lines[i]:
        closed = False
        for j in range(i + 1, len(lines)):
            invocation.append(lines[j])
            if lines[j].strip() == '<<':
                closed = True
                break
        if not closed:
            return None, None, [f'unterminated NMake inline response in {source}']
    return '\n'.join(invocation), lines[i], []


def _working_directory(root: Path, invocation_line: str | None):
    if not invocation_line:
        return root
    # NMake commonly prefixes recipes with `cd /D <dir> &&`.  Response paths
    # belong to the actual invocation working directory, not the .dir folder.
    m = re.search(r'(?i)(?:^|\s)cd\s+(?:/D\s+)?(?:(?:"([^"]+)")|([^&\r\n]+?))\s*&&', invocation_line)
    if not m:
        return root
    raw = (m.group(1) or m.group(2) or '').strip()
    if not raw:
        return root
    p = Path(raw)
    if not p.is_absolute():
        p = root / p
    safe = _safe_under(root, p)
    return safe if safe is not None else None


def _response_refs(text: str):
    refs = []
    for quoted, plain in re.findall(r'@(?:"([^"]+)"|([^\s<>]+))', text):
        raw = quoted or plain
        if raw == '<<':
            continue
        refs.append(raw)
    return refs


def collect_text(build_dir: Path):
    root = build_dir.resolve()
    source, kind, errors = normalized_link_source(root)
    if errors:
        return None, None, errors

    invocation, invocation_line, errors = _extract_invocation(source, kind)
    if errors:
        return None, None, errors
    working = _working_directory(root, invocation_line)
    if working is None:
        return None, None, ['link invocation working directory escapes build root']

    pieces = [invocation]
    queue = list(_response_refs(invocation))
    seen = set()
    while queue:
        raw = queue.pop(0)
        candidate = Path(raw)
        if not candidate.is_absolute():
            candidate = working / candidate
        safe = _safe_under(root, candidate)
        if safe is None:
            return None, None, [f'response reference escapes build root: {raw}']
        if safe in seen:
            continue
        if not safe.is_file():
            return None, None, [f'referenced response file not found: {raw}']
        seen.add(safe)
        body = safe.read_text(errors='replace')
        pieces.append(body)
        queue.extend(_response_refs(body))
        if len(seen) > 128:
            return None, None, ['response-file closure exceeds 128 files']

    return '\n'.join(pieces), kind, []


def audit(command_text: str):
    # LINK accepts '-' and '/' as equivalent option specifiers. Normalize
    # token-start dashes in this audit copy, never the generated link command.
    command_text = re.sub(r'(?<!\S)-(?=[A-Za-z])', '/', command_text)
    errors = []
    debug_opts = re.findall(r'(?i)(?:^|\s)(/DEBUG(?::[^\s"<>]+)?)(?=\s|$|[<>])', command_text)
    if not any(x.upper() == '/DEBUG:NONE' for x in debug_opts):
        errors.append('missing /DEBUG:NONE on normalized shipping link rule')
    bad_debug = [x for x in debug_opts if x.upper() != '/DEBUG:NONE']
    if bad_debug:
        errors.append('forbidden debug linker option(s): ' + ', '.join(bad_debug))
    if re.search(r'(?i)(?:^|\s)/BREPRO(?=\s|$|[<>])', command_text):
        errors.append('/Brepro is forbidden: it emits IMAGE_DEBUG_TYPE_REPRO')

    # S0083: inspect every effective /INCREMENTAL spelling.  Presence of one
    # good token cannot neutralize a conflicting duplicate later/earlier.
    incremental_opts = re.findall(r'(?i)(?:^|\s)(/INCREMENTAL(?::[^\s"<>]+)?)(?=\s|$|[<>])', command_text)
    if not any(x.upper() == '/INCREMENTAL:NO' for x in incremental_opts):
        errors.append('missing /INCREMENTAL:NO on normalized shipping link rule')
    bad_incremental = [x for x in incremental_opts if x.upper() != '/INCREMENTAL:NO']
    if bad_incremental:
        errors.append('forbidden incremental linker option(s): ' + ', '.join(bad_incremental))

    if re.search(r'(?i)(?:^|\s)/NODEFAULTLIB(?::[^\s"<>]+)?(?=\s|$|[<>])', command_text):
        errors.append('/NODEFAULTLIB is forbidden for normalized CRT shipping')
    if re.search(r'(?i)(?:^|\s)/ENTRY:[^\s"<>]+(?=\s|$|[<>])', command_text):
        errors.append('custom /ENTRY is forbidden; linker must select _DllMainCRTStartup')

    ltcg_opts = re.findall(r'(?i)(?:^|\s)(/LTCG(?::[^\s"<>]+)?)(?=\s|$|[<>])', command_text)
    if not any(x.upper() == '/LTCG:OFF' for x in ltcg_opts):
        errors.append('missing /LTCG:OFF on normalized shipping link rule')
    bad_ltcg = [x for x in ltcg_opts if x.upper() != '/LTCG:OFF']
    if bad_ltcg:
        errors.append('forbidden LTCG linker option(s): ' + ', '.join(bad_ltcg))
    if not re.search(r'(?i)(?:^|\s)/NOCOFFGRPINFO(?=\s|$|[<>])', command_text):
        errors.append('missing /NOCOFFGRPINFO on normalized shipping link rule')
    manifest_opts = re.findall(r'(?i)(?:^|\s)(/MANIFEST(?::[^\s"<>]+)?)(?=\s|$|[<>])', command_text)
    if not any(x.upper() == '/MANIFEST:NO' for x in manifest_opts):
        errors.append('missing /MANIFEST:NO on normalized shipping link rule')
    bad_manifest = [x for x in manifest_opts if x.upper() != '/MANIFEST:NO']
    if bad_manifest:
        errors.append('forbidden manifest-producing linker option(s): ' + ', '.join(bad_manifest))
    return errors


def self_test():
    cases = [
        ('good', 'link.exe /DLL /INCREMENTAL:NO /DEBUG:NONE /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', True),
        ('incremental_case_no', 'link.exe /incremental:no /DEBUG:NONE /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', True),
        ('incremental_duplicate_no', 'link.exe /INCREMENTAL:NO /incremental:no /DEBUG:NONE /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', True),
        ('incremental_yes_before_no', 'link.exe /INCREMENTAL /INCREMENTAL:NO /DEBUG:NONE /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', False),
        ('incremental_yes_after_no', 'link.exe /INCREMENTAL:NO /INCREMENTAL:YES /DEBUG:NONE /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', False),
        ('debug_full_before_none', 'link.exe /DEBUG:FULL /DEBUG:NONE /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', False),
        ('debug_after_none', 'link.exe /DEBUG:NONE /DEBUG /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', False),
        ('brepro', 'link.exe /INCREMENTAL:NO /Brepro /DEBUG:NONE /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', False),
        ('missing_none', 'link.exe /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', False),
        ('incremental_missing', 'link.exe /DEBUG:NONE /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', False),
        ('ltcg_missing', 'link.exe /DEBUG:NONE /INCREMENTAL:NO /NOCOFFGRPINFO /MANIFEST:NO a.obj', False),
        ('ltcg_enabled', 'link.exe /DEBUG:NONE /INCREMENTAL:NO /LTCG /NOCOFFGRPINFO /MANIFEST:NO a.obj', False),
        ('coffgrp_missing', 'link.exe /DEBUG:NONE /INCREMENTAL:NO /LTCG:OFF /MANIFEST:NO a.obj', False),
        ('manifest_missing', 'link.exe /DEBUG:NONE /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO a.obj', False),
        ('manifest_embed', 'link.exe /DEBUG:NONE /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:EMBED a.obj', False),
        ('nodefaultlib', 'link.exe /DLL /NODEFAULTLIB /INCREMENTAL:NO /DEBUG:NONE /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', False),
        ('custom_entry', 'link.exe /DLL /ENTRY:DllMain /INCREMENTAL:NO /DEBUG:NONE /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO a.obj', False),
    ]
    good = cases[0][1]
    cases.extend([
        ('dash_option_specifiers', good.replace('/', '-'), True),
        ('mixed_option_specifiers', good.replace('/DEBUG:', '-debug:').replace('/MANIFEST:', '-manifest:'), True),
        ('embedded_dash_paths_are_not_switches', good + r' /OUT:build\-NODEFAULTLIB.dll other-INCREMENTAL.obj', True),
    ])
    # A valid canonical switch cannot hide a conflicting dash spelling,
    # regardless of argument order. Preserve the existing reject-any-conflict
    # policy instead of relying on LINK's last-option-wins behavior.
    for option in ('debug:FULL', 'DEBUG', 'incremental', 'INCREMENTAL:YES',
                   'ltcg', 'LTCG:INCREMENTAL', 'manifest', 'MANIFEST:EMBED',
                   'nodefaultlib', 'NODEFAULTLIB:libcmt', 'ENTRY:UserEntry', 'Brepro'):
        for before in (False, True):
            text = good.replace('link.exe ', 'link.exe -' + option + ' ', 1) if before else good + ' -' + option
            cases.append(('dash_' + option + ('_before' if before else '_after'), text, False))
    failed = []
    for name, text, expected in cases:
        actual = not audit(text)
        if actual != expected:
            failed.append(name)

    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        target = root / 'CMakeFiles' / 'citrine_eclipse_mirage.dir'
        target.mkdir(parents=True)

        # NMake inline response is part of the selected invocation and needs no
        # unrelated target-local response-file scan.
        (target / 'build.make').write_text(
            'citrine_eclipse_mirage.unsigned.dll:\n\tlink.exe @<<\n'
            '/DLL /INCREMENTAL:NO /DEBUG:NONE /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO /OUT:citrine_eclipse_mirage.unsigned.dll\n<<\n',
            encoding='utf-8',
        )
        (target / 'stale.rsp').write_text('/INCREMENTAL /DEBUG /LTCG /MANIFEST:EMBED\n', encoding='utf-8')
        text, kind, errors = collect_text(root)
        if errors or kind != 'build.make' or audit(text or ''):
            failed.append('nmake_inline_ignores_unreferenced_rsp')

        # External response closure: only an actually referenced file may
        # supply required options.  A disconnected stale file must not help.
        (target / 'build.make').write_text('x:\n\tlink.exe @flags.rsp a.obj\n', encoding='utf-8')
        (root / 'flags.rsp').write_text('/INCREMENTAL:NO /DEBUG:NONE /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO\n', encoding='utf-8')
        (target / 'stale.rsp').write_text('/NODEFAULTLIB\n', encoding='utf-8')
        text, _, errors = collect_text(root)
        if errors or audit(text or ''):
            failed.append('referenced_rsp_closure')

        (target / 'build.make').write_text('x:\n\tlink.exe a.obj\n', encoding='utf-8')
        text, _, errors = collect_text(root)
        if errors or not audit(text or ''):
            failed.append('unreferenced_rsp_cannot_complete_policy')

        (target / 'build.make').write_text('x:\n\tlink.exe @missing.rsp a.obj\n', encoding='utf-8')
        _, _, errors = collect_text(root)
        if not errors:
            failed.append('missing_referenced_rsp_fails_closed')

    if failed:
        return fail(['self-test mismatch: ' + ', '.join(failed)])
    total = len(cases) + 4
    print(f'MSVC_SHIPPING_LINK_POLICY_SELF_TEST: PASS ({total}/{total})')
    return 0


def main():
    if len(sys.argv) == 2 and sys.argv[1] == '--self-test':
        return self_test()
    if len(sys.argv) != 2:
        print('usage: check_msvc_shipping_link_command.py <cmake-build-dir>', file=sys.stderr)
        return 2
    text, kind, errors = collect_text(Path(sys.argv[1]))
    if errors:
        return fail(errors)
    errors = audit(text)
    if errors:
        return fail(errors)
    print(
        'MSVC_SHIPPING_LINK_POLICY: PASS '
        f'(source={kind}; referenced-response-closure only; CRT default entry/default libs; '
        '/DEBUG:NONE only; /Brepro absent; /INCREMENTAL:NO only; /LTCG:OFF; /NOCOFFGRPINFO; /MANIFEST:NO)'
    )
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
