# SPDX-License-Identifier: GPL-3.0-only

"""Check the declared application/test/vendor source bytes and layout boundary."""
from pathlib import Path
import hashlib
import json
import re
from project_files import vendor_symlinks, source_payload, alias_payload


def verify(root):
    manifest=json.loads((root/'metadata/SOURCE_FILES.json').read_text())
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
            text=(root/name).read_text()
            qualified=re.findall(r'\bnamespace\s+([A-Za-z_]\w*(?:::\w+)+)\s*\{',text)
            if any(value.split('::',1)[0]!='preloader_l' for value in qualified):
                errors.append('Foreign first-party namespace: '+name)
    return {'status':'FAIL' if errors else 'PASS','files_checked':len(expected),
            'scope':'declared source bytes and project boundaries, not native equivalence or signed authorization',
            'errors':errors}


if __name__=='__main__':
    result=verify(Path(__file__).resolve().parents[1])
    print(json.dumps(result,indent=2))
    raise SystemExit(0 if result['status']=='PASS' else 1)
