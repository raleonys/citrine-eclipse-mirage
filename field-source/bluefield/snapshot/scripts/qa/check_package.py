# SPDX-License-Identifier: GPL-3.0-only
"""Validate a complete source package independently of its PASS receipts."""
import argparse,hashlib,json,sys,tempfile,zipfile
from pathlib import Path,PurePosixPath
from _project import ROOT,sha,save
from check_source import check
def inspect(path,trusted_sha=None):
 errors=[]
 if trusted_sha and sha(path)!=trusted_sha: return ['trusted archive SHA-256 mismatch']
 try:
  with zipfile.ZipFile(path) as archive:
   names=archive.namelist()
   if len(names)!=len({n.casefold() for n in names}):return ['duplicate archive paths']
   if any(not n.startswith('preloader_l/') or PurePosixPath(n).is_absolute() or '..' in PurePosixPath(n).parts or '\\' in n or ':' in n for n in names):return ['unsafe archive path']
   if any(((i.external_attr>>16)&0o170000)==0o120000 for i in archive.infolist()):return ['archive symlink']
   if sum(i.file_size for i in archive.infolist())>64*1024*1024:return ['archive size quota']
   if archive.testzip():return ['archive CRC']
   with tempfile.TemporaryDirectory(prefix='preloader_l_package_') as temp:
    archive.extractall(temp);root=Path(temp)/'preloader_l'
    manifest=json.loads((root/'metadata/PACKAGE_MANIFEST.json').read_text())['files']
    actual={p.relative_to(root).as_posix() for p in root.rglob('*') if p.is_file()}
    if actual!=set(manifest)|{'metadata/PACKAGE_MANIFEST.json'}:errors.append('package inventory mismatch')
    for name,record in manifest.items():
     rel=PurePosixPath(name)
     if rel.is_absolute() or '..' in rel.parts or '\\' in name or ':' in name:errors.append('unsafe manifest path');continue
     p=root/name
     if not p.is_file() or p.stat().st_size!=record['bytes'] or sha(p)!=record['sha256']:errors.append('package member hash: '+name)
    errors+=check(root)
 except Exception as e:errors.append(type(e).__name__+': '+str(e))
 return errors
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('archive',type=Path);ap.add_argument('--sha256');a=ap.parse_args()
 errors=inspect(a.archive,a.sha256);save(ROOT/'build/reports/package_verify.json',{'status':'PASS' if not errors else 'FAIL','errors':errors,'archive_sha256':sha(a.archive) if a.archive.exists() else None,'scope':'static source/ZIP integrity, not native runtime'})
 print('FAIL '+str(errors) if errors else 'PASS complete source package integrity');return int(bool(errors))
if __name__=='__main__':sys.exit(main())
