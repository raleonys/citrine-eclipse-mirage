# SPDX-License-Identifier: GPL-3.0-only

"""Bind the MSVC link audit to its exact DLL, cache and source receipt."""
import argparse
import hashlib
import json
from pathlib import Path
import re
from check_msvc_shipping_link_command import collect_text, audit


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def _cmake_commands(text):
    """Read command boundaries without executing generated CMake code.

    Quoted/bracket arguments are opaque; comments cannot declare commands.
    This lexical reader does not expand variables or evaluate CMake programs.
    """
    token = re.compile(r'(?P<space>[ \t\r\n]+)|'
                       r'(?P<bracket>#?\[(?P<equals>=*)\[)|'
                       r'(?P<comment>#[^\r\n]*)|'
                       r'(?P<quoted>"(?:\\[\s\S]|[^"\\])*")|'
                       r'(?P<escape>\\[\s\S])|'
                       r'(?P<word>[^ \t\r\n()#"\\]+)|(?P<paren>[()])')
    text = text.removeprefix('\ufeff')
    position, depth = 0, 0
    name, arguments = None, []
    while position < len(text):
        match = token.match(text, position)
        if match is None:
            raise ValueError('Malformed generated CMake compiler record')
        kind, value = match.lastgroup, match.group()
        position = match.end()
        if kind == 'bracket':
            closing = ']' + match.group('equals') + ']'
            end = text.find(closing, position)
            if end < 0:
                raise ValueError('Unterminated generated CMake bracket')
            position = end + len(closing)
            if value.startswith('#'):
                kind, value = 'comment', ' '
            else:
                kind, value = 'bracket_argument', text[match.start():position]
        if kind in {'space', 'comment'}:
            if depth:
                arguments.append(value if kind == 'space' else ' ')
            continue
        if not depth:
            if name is None and kind == 'word' and re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', value):
                name = value.lower()
            elif name is not None and kind == 'paren' and value == '(':
                depth = 1
            else:
                raise ValueError('Malformed generated CMake command')
            continue
        if kind == 'paren':
            if value == '(':
                depth += 1
            else:
                depth -= 1
                if not depth:
                    yield name, ''.join(arguments)
                    name, arguments = None, []
                    continue
        arguments.append(value)
    if depth or name is not None:
        raise ValueError('Incomplete generated CMake command')


def _compiler_id_is_msvc(text):
    # CMake emits one unconditional literal assignment. Refuse ambiguous,
    # conditional or indirect evidence instead of interpreting arbitrary code.
    openings = {'if', 'foreach', 'while', 'function', 'macro', 'block'}
    closings = {'end' + name: name for name in openings}
    blocks, identities = [], 0
    variable = r'[ \t\r\n]*CMAKE_CXX_COMPILER_ID(?=[ \t\r\n]|$)'
    target = (r'[ \t\r\n]*(?:CMAKE_CXX_COMPILER_ID|"CMAKE_CXX_COMPILER_ID"|'
              r'\[(?P<id_equals>=*)\[(?:\r?\n)?CMAKE_CXX_COMPILER_ID\](?P=id_equals)\])(?=[ \t\r\n]|$)')
    try:
        for name, arguments in _cmake_commands(text):
            if name in openings:
                blocks.append(name)
            elif name in closings:
                if not blocks or blocks.pop() != closings[name]:
                    return False
            elif name in {'set', 'unset'} and re.match(target, arguments):
                if (blocks or name != 'set' or
                        not re.fullmatch(variable + r'[ \t\r\n]+"MSVC"[ \t\r\n]*', arguments)):
                    return False
                identities += 1
    except ValueError:
        return False
    return identities == 1 and not blocks


def check(root, build_dir, dll):
    root, build_dir, dll = map(Path, (root, build_dir, dll))
    receipt = json.loads((build_dir / 'BUILD_RECEIPT.json').read_text(encoding='utf-8-sig'))
    errors = []
    if receipt.get('build_succeeded') is not True or receipt.get('mode') != 'Golden' or receipt.get('configuration') != 'Release':
        errors.append('Missing successful Golden/Release MSVC build receipt')
    for field, path in [('dll_sha256', dll), ('cmake_cache_sha256', build_dir / 'CMakeCache.txt'),
                        ('source_manifest_sha256', root / 'metadata/ACTIVE_SOURCE_MANIFEST.json')]:
        if receipt.get(field) != sha(path):
            errors.append('Build identity mismatch: ' + field)
    # The public receipt keeps a basename. Resolve the actual executable from
    # this build's already hash-bound cache, never from the current PATH.
    compiler_entries = re.findall(r'^CMAKE_CXX_COMPILER:FILEPATH=(.+)$',
                                 (build_dir / 'CMakeCache.txt').read_text(encoding='utf-8'), re.M)
    if len(compiler_entries) != 1:
        errors.append('Exact build compiler path is missing or ambiguous')
        return errors
    compiler = Path(compiler_entries[0].strip())
    recorded = Path(receipt['compiler'])
    if not compiler.is_absolute() or compiler.name.lower() != 'cl.exe':
        errors.append('Exact build compiler path is not an absolute MSVC executable')
    elif (recorded.name.lower() != compiler.name.lower() or
          (recorded.is_absolute() and recorded.resolve() != compiler.resolve()) or
          (not recorded.is_absolute() and len(recorded.parts) != 1) or
          sha(compiler) != receipt.get('compiler_sha256')):
        errors.append('MSVC compiler tool identity mismatch')
    compiler_records = list((build_dir / 'CMakeFiles').glob('*/CMakeCXXCompiler.cmake'))
    if len(compiler_records) != 1 or not _compiler_id_is_msvc(compiler_records[0].read_text(encoding='utf-8-sig')):
        errors.append('Exact build compiler ID is not MSVC')
    command, kind, link_errors = collect_text(build_dir)
    errors.extend(link_errors)
    if not link_errors:
        errors.extend(audit(command))
    return errors


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('build_dir', type=Path)
    ap.add_argument('dll', type=Path)
    args = ap.parse_args()
    try:
        errors = check(Path(__file__).resolve().parents[1], args.build_dir, args.dll)
    except (OSError, ValueError, KeyError) as error:
        errors = [str(error)]
    for error in errors:
        print(' -', error)
    print('MSVC_BUILD_EVIDENCE:', 'FAIL' if errors else 'PASS_MSVC_BUILD')
    raise SystemExit(1 if errors else 0)
