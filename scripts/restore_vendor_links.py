"""Optionally turn exact locked alias descriptors into symlinks; not needed to build."""
import argparse
from pathlib import Path
import os
from project_files import vendor_symlinks


def restore(root):
    root=Path(root).resolve()
    for name,target in vendor_symlinks(root).items():
        path=Path(root)/name
        for parent in path.parents:
            if parent==root:break
            if parent.is_symlink():raise ValueError('Refusing a vendor alias under a symlink parent: '+name)
        if path.is_symlink():
            if os.readlink(path)!=target:raise ValueError('Altered vendor link: '+name)
            continue
        if not path.is_file() or path.read_bytes()!=target.encode('utf-8'):
            raise ValueError('Refusing to replace a non-descriptor vendor path: '+name)
        data=path.read_bytes();path.unlink()
        try:os.symlink(target,path,target_is_directory=True)
        except OSError:
            path.write_bytes(data)
            raise


if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[1])
    args=ap.parse_args()
    try:restore(args.root);print('LOCKED_VENDOR_LINKS: RESTORED')
    except (OSError,ValueError) as error:
        ap.exit(1,'VENDOR_LINKS: FAIL: '+str(error)+' (Windows symlink creation requires Developer Mode or suitable privileges.)\n')
