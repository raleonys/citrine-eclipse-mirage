# SPDX-License-Identifier: GPL-3.0-only

"""Extract a verified source ZIP; portable locked aliases require no symlink rights."""
import argparse
import json
from pathlib import Path
import os
import shutil
import stat
import tempfile
import zipfile
from project_files import MANIFEST, ALIAS_STORAGE, safe_member, digest, verify_manifest


def extract(archive_path, destination):
    destination=Path(destination).absolute()
    if destination.exists() or destination.is_symlink():
        raise ValueError('Extraction destination already exists; refusing overwrite')
    destination.parent.mkdir(parents=True,exist_ok=True)
    temp=Path(tempfile.mkdtemp(prefix='.surge-extract-',dir=destination.parent))
    try:
        with zipfile.ZipFile(archive_path) as archive:
            if archive.testzip():raise ValueError('ZIP CRC failed')
            infos=archive.infolist();names=[safe_member(i.filename) for i in infos]
            if len(names)!=len(set(n.casefold() for n in names)):
                raise ValueError('Duplicate or case-fold archive members')
            manifest=json.loads(archive.read(MANIFEST));links=manifest.get('symlinks',{})
            storage = manifest.get('vendor_alias_storage')
            if storage not in (None, ALIAS_STORAGE):
                raise ValueError('Unsupported source alias storage policy')
            portable = storage == ALIAS_STORAGE
            if set(names)!=set(manifest['files'])|{MANIFEST}:
                raise ValueError('Archive membership does not match its manifest')
            for info in infos:
                name=info.filename
                if info.is_dir():raise ValueError('Source archive must use flat file membership')
                payload=archive.read(info)
                if name!=MANIFEST and digest(payload)!=manifest['files'][name]:
                    raise ValueError('Source archive hash mismatch: '+name)
                mode = info.external_attr >> 16
                if stat.S_ISLNK(mode)!=(name in links and not portable):
                    raise ValueError('Archive source/link type mismatch: '+name)
                if portable and mode and not stat.S_ISREG(mode):
                    raise ValueError('Portable archive contains a non-regular member: '+name)
                if name in links:
                    if payload!=links[name].encode('utf-8'):
                        raise ValueError('Archive link payload mismatch')
                    if not portable:continue
                path=temp/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(payload)
            # Check link paths/targets against the actual source vendor lock, not
            # merely an arbitrary archive link declaration. Create links last.
            from project_files import vendor_symlinks
            if links!=vendor_symlinks(temp):raise ValueError('Archive/vendor-lock link mismatch')
            if not portable:
                for name,target in links.items():
                    path=temp/name;path.parent.mkdir(parents=True,exist_ok=True)
                    os.symlink(target,path,target_is_directory=True)
            errors=verify_manifest(temp)
            if errors:raise ValueError('; '.join(errors))
        temp.rename(destination)
        return {'result':'PASS','destination':str(destination),'files':len(names),'symlinks':len(links)}
    finally:
        if temp.exists():shutil.rmtree(temp)


if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('archive',type=Path)
    ap.add_argument('destination',type=Path)
    args=ap.parse_args()
    try:print(json.dumps(extract(args.archive,args.destination),indent=2))
    except (OSError,ValueError,KeyError,zipfile.BadZipFile) as error:
        ap.exit(1,'SOURCE_EXTRACTION: FAIL: '+str(error)+'\n')
