# SPDX-License-Identifier: GPL-3.0-only

"""Check the declared application/test/vendor source bytes and layout boundary."""
from pathlib import Path
import hashlib
import json
import re
from project_files import vendor_symlinks, source_payload, alias_payload, load_unique_json


def mask_cpp_noncode(source):
    """Preserve code/newline positions while masking C++ comments and literals.

    This is a bounded lexical prepass, not a replacement for native compilation.
    """
    masked = list(source)
    length = len(source)

    def blank(start, end):
        for offset in range(start, end):
            if source[offset] not in "\r\n":
                masked[offset] = " "

    index = 0
    while index < length:
        if source.startswith("//", index):
            end = source.find("\n", index)
            # C++20 removes backslash-newline before recognizing line comments.
            while end >= 0:
                last = end - 1
                if last >= index and source[last] == "\r":
                    last -= 1
                if last < index or source[last] != "\\":
                    break
                end = source.find("\n", end + 1)
            end = length if end < 0 else end
            blank(index, end)
            index = end
        elif source.startswith("/*", index):
            end = source.find("*/", index + 2)
            end = length if end < 0 else end + 2
            blank(index, end)
            index = end
        elif source.startswith('R"', index):
            opening = source.find("(", index + 2, min(length, index + 19))
            if (opening >= 0 and
                    all(char not in "\r\n\t ()\\" for char in source[index + 2:opening])):
                terminator = ")" + source[index + 2:opening] + '"'
                end = source.find(terminator, opening + 1)
                end = length if end < 0 else end + len(terminator)
                blank(index, end)
                index = end
            else:
                index += 1
        elif ((source[index] in "0123456789" or
               (source[index] == "." and index + 1 < length and
                source[index + 1] in "0123456789")) and
              (index == 0 or not (source[index - 1].isalnum() or source[index - 1] == "_"))):
            # Consume pp-numbers without masking them. Their apostrophes are
            # digit separators, not character-literal openers. The boundary
            # check also keeps the u8 prefix of character/raw literals intact.
            number = re.match(r"(?:[0-9]|\.[0-9])(?:[eEpP][+-]|[\w.]|'\w)*", source[index:])
            index += number.end()
        elif source[index] in ("'", '"'):
            quote = source[index]
            end = index + 1
            while end < length:
                if source[end] == "\\":
                    end += 2
                elif source[end] == quote:
                    end += 1
                    break
                else:
                    end += 1
            end = min(end, length)
            blank(index, end)
            index = end
        else:
            index += 1
    return "".join(masked)


def verify(root):
    manifest=load_unique_json(root/'metadata/SOURCE_FILES.json')
    errors=[]
    expected=manifest['files']
    allowed_links=vendor_symlinks(root)
    actual={p.relative_to(root).as_posix() for folder in ['src','include','tests','third_party']
            for p in (root/folder).rglob('*') if p.is_file() or p.is_symlink()}
    if actual!=set(expected):
        errors.append({'missing':sorted(set(expected)-actual),'unrecorded':sorted(actual-set(expected))})
    for name,entry in expected.items():
        p=root/name
        if name in allowed_links:
            try:
                alias_payload(p, allowed_links[name])
                if entry.get('kind') not in ('vendor_alias', 'symlink'):
                    raise ValueError('Missing alias type')
                if entry.get('kind') == 'symlink' and not p.is_symlink():
                    raise ValueError('Legacy source requires a real symlink')
            except (OSError, ValueError):
                errors.append('Unregistered/altered vendor link: '+name)
                continue
        elif p.is_symlink():
            errors.append('Unregistered source symlink: '+name)
            continue
        elif not p.is_file() or entry.get('kind')=='symlink':
            errors.append('Missing/unsafe source: '+name)
            continue
        if root.resolve() not in p.resolve().parents:
            errors.append('Missing/unsafe source: '+name)
            continue
        payload=source_payload(p)
        size=entry.get('bytes')
        if type(size) is not int or size<0:
            errors.append('Invalid source size: '+name)
        elif len(payload)!=size:
            errors.append('Source size differs: '+name)
        if hashlib.sha256(payload).hexdigest()!=entry['sha256']:
            errors.append('Source differs: '+name)
    for name in actual:
        if not name.startswith('third_party/') and any(part.endswith('-source') or part=='recovered' for part in Path(name).parts):
            errors.append('Raw RE module directory in project: '+name)
        if name.startswith(('src/','include/')):
            text=(root/name).read_text(encoding='utf-8')
            qualified=re.findall(r'\bnamespace\s+([A-Za-z_]\w*(?:::\w+)+)\s*\{',
                                 mask_cpp_noncode(text))
            if any(value.split('::',1)[0]!='laureondo' for value in qualified):
                errors.append('Foreign first-party namespace: '+name)
    return {'status':'FAIL' if errors else 'PASS','files_checked':len(expected),
            'scope':'declared source bytes and project boundaries, not native equivalence or signed authorization',
            'errors':errors}


if __name__=='__main__':
    result=verify(Path(__file__).resolve().parents[1])
    print(json.dumps(result,indent=2))
    raise SystemExit(0 if result['status']=='PASS' else 1)
