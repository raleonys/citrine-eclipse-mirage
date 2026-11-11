# SPDX-License-Identifier: GPL-3.0-only

"""Generate private read-only avatar arrays from exactly pinned source PNG bytes."""
import argparse
import hashlib
import json
from pathlib import Path


def generate(root, output):
    root, output = Path(root), Path(output)
    contract = json.loads((root/'metadata/AVATAR_DATA_CONTRACT.json').read_text(encoding='utf-8'))
    text = ['#pragma once', '// Generated from pinned PNG inputs; not a PE resource.',
            'namespace citrine_eclipse_mirage::application::detail::avatar_data {']
    for symbol in ('profile', 'fallback'):
        spec = contract['assets'][symbol]
        path = root/spec['source']
        if root.resolve() not in path.resolve().parents:
            raise ValueError('Avatar input is outside source tree')
        data = path.read_bytes()
        if not data.startswith(b'\x89PNG\r\n\x1a\n') or hashlib.sha256(data).hexdigest()!=spec['sha256']:
            raise ValueError('Avatar input hash differs: '+spec['source'])
        text.append('inline constexpr unsigned char '+symbol+'[] = {')
        for offset in range(0,len(data),16):
            text.append('  '+','.join('0x%02x'%b for b in data[offset:offset+16])+',')
        text.append('};')
    text.append('}')
    result='\n'.join(text)+'\n'
    # Reject aliases before mkdir: even directory creation may escape through one.
    components = (output, *output.parents)
    if any(p.is_symlink() or getattr(p, 'is_junction', lambda: False)()
           for p in components):
        raise ValueError('Avatar output cannot traverse a symlink or junction')
    output.parent.mkdir(parents=True,exist_ok=True)
    if output.exists() and (not output.is_file() or output.stat().st_nlink != 1):
        raise ValueError('Avatar output must be an unshared regular file')
    if not output.exists() or output.read_text(encoding='utf-8')!=result:
        output.write_text(result,encoding='utf-8')


if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args()
    generate(args.root,args.output)
