# SPDX-License-Identifier: GPL-3.0-only

"""Prepare and verify the hash-pinned hook engine build copy; vendor bytes stay immutable."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
from vendor_sync import load_lock, expected_files, validate_installed


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def canonicalize_generated_hook(output, expected_sha256):
    """Use pinned LF bytes for CMake's platform-dependent generated output only."""
    path = Path(output) / 'src/hook.c'
    original = path.read_bytes()
    canonical = original.replace(b'\r\n', b'\n')
    # Never repair arbitrary content or normalize the supplied vendor tree.
    # Unknown output remains unchanged and fails the full member verification.
    if canonical != original and hashlib.sha256(canonical).hexdigest() == expected_sha256:
        path.write_bytes(canonical)


def checked_policy(root, expected):
    policy=json.loads((root/'metadata/HOOK_ENGINE_PATCH.json').read_text())
    if set(policy['patch_modules'])!={'cmake/PatchHookCompletion.cmake','cmake/PatchHookRelocator.cmake'}:
        raise ValueError('Hook engine patch module policy mismatch')
    scripts={policy['patch_script']:policy['patch_script_sha256'], **policy['patch_modules']}
    for name, digest in scripts.items():
        if sha(root/name)!=digest:
            raise ValueError('Hook engine patch script hash mismatch: '+name)
    outputs=policy['output_files']
    if set(outputs)!={'src/hook.c','src/buffer.c','src/buffer.h','src/trampoline.c'} or outputs['src/hook.c']!=policy['output_hook_sha256']:
        raise ValueError('Hook engine patch output policy mismatch')
    if expected['src/hook.c']!=policy['input_hook_sha256']:
        raise ValueError('Hook engine patch input mismatch')
    return policy


def verify(root, output):
    root, output = Path(root).resolve(), Path(output).absolute()
    build=root/'build'
    if build not in output.parents or output.is_symlink() or output.resolve()!=output:
        raise ValueError('Hook engine copy must remain in the owned build directory')
    record=load_lock(root)['vendors']['minhook']
    expected=expected_files(root,record)
    validate_installed(root,'minhook',record,expected,{})
    policy=checked_policy(root,expected)
    if policy['input_commit']!=record['upstream_commit']:
        raise ValueError('Hook engine patch commit mismatch')
    expected.update(policy['output_files'])
    actual={p.relative_to(output).as_posix():sha(p) for p in output.rglob('*') if p.is_file()}
    links = sorted(p.relative_to(output).as_posix() for p in output.rglob('*') if p.is_symlink())
    if links or actual!=expected:
        details = {'expected_members': len(expected), 'actual_members': len(actual),
                   'missing': sorted(expected.keys() - actual.keys()),
                   'extra': sorted(actual.keys() - expected.keys()),
                   'changed': [{'path': name, 'expected_sha256': expected[name], 'actual_sha256': actual[name]}
                               for name in sorted(expected.keys() & actual.keys()) if expected[name] != actual[name]],
                   'symlinks': links}
        raise ValueError('Hook engine build-copy member/hash mismatch: ' + json.dumps(details, sort_keys=True))
    return policy


def prepare(root, output, cmake):
    root, output=Path(root).resolve(),Path(output).absolute()
    if root/'build' not in output.parents or output.is_symlink() or output.resolve()!=output:
        raise ValueError('Hook engine copy must remain in the owned build directory')
    output.parent.mkdir(parents=True,exist_ok=True)
    record=load_lock(root)['vendors']['minhook'];expected=expected_files(root,record)
    policy=checked_policy(root,expected)
    if policy['input_commit']!=record['upstream_commit']:
        raise ValueError('Hook engine patch commit mismatch')
    validate_installed(root,'minhook',record,expected,{})
    if expected['src/hook.c']!=policy['input_hook_sha256']:
        raise ValueError('Hook engine patch input mismatch')
    with tempfile.TemporaryDirectory(prefix='.hook-stage-',dir=output.parent) as temporary:
        stage=Path(temporary)/'engine'
        shutil.copytree(root/record['source_install_path'],stage)
        subprocess.run([str(cmake),'-DMINHOOK_HOOK_C='+str(stage/'src/hook.c'),'-P',str(root/policy['patch_script'])],check=True)
        for name,digest in policy['output_files'].items():
            path=stage/name
            original=path.read_bytes();canonical=original.replace(b'\r\n',b'\n')
            if canonical!=original and hashlib.sha256(canonical).hexdigest()==digest:
                path.write_bytes(canonical)
        # Verify every file, not only the patched translation unit.
        verify(root,stage)
        if output.exists():
            try:verify(root,output)
            except ValueError:shutil.rmtree(output)
            else:return # Keep timestamps so an unchanged configure does not rebuild the engine.
        stage.rename(output)
    verify(root,output)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--cmake',type=Path)
    parser.add_argument('--prepare',action='store_true')
    args=parser.parse_args()
    if args.prepare:
        if not args.cmake:parser.error('--prepare requires --cmake')
        prepare(args.root,args.output,args.cmake)
    else:verify(args.root,args.output)
    print('HOOK_ENGINE_BUILD_COPY=PASS')
