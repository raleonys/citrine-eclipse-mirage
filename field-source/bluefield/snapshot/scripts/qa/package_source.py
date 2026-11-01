# SPDX-License-Identifier: GPL-3.0-only
"""Seal the complete application repository as a non-release source snapshot."""
import json,sys,zipfile
from _project import ROOT,source_files,source_digest,sha,save
from check_source import check
from check_package import inspect
def main():
 errors=check()
 if errors:print(errors);return 1
 files=source_files();state=source_digest()
 save(ROOT/'metadata/SOURCE_MANIFEST.json',{'source_digest':state,'source_digest_algorithm':'SHA256(sorted path + NUL + per-file SHA256 + LF)','excluded_derived':['SOURCE_MANIFEST.json','PACKAGE_MANIFEST.json','ACCEPTANCE.json','build/','dist/','evidence/'],'files':{p.relative_to(ROOT).as_posix():{'sha256':sha(p),'bytes':p.stat().st_size} for p in files}})
 files += [ROOT/'metadata/SOURCE_MANIFEST.json',ROOT/'metadata/ACCEPTANCE.json']
 files += sorted(p for p in (ROOT/'evidence').rglob('*') if p.is_file())
 save(ROOT/'metadata/PACKAGE_MANIFEST.json',{'files':{p.relative_to(ROOT).as_posix():{'sha256':sha(p),'bytes':p.stat().st_size} for p in files}})
 files += [ROOT/'metadata/PACKAGE_MANIFEST.json']
 out=ROOT/'dist/preloader_l_BF02_FIX31_APPLICATION_SOURCE_NOT_RELEASE.zip';out.parent.mkdir(exist_ok=True)
 pending=out.with_suffix('.pending')
 with zipfile.ZipFile(pending,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
  for p in sorted(files):
   info=zipfile.ZipInfo('preloader_l/'+p.relative_to(ROOT).as_posix(),(2026,10,27,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED;info.external_attr=0o644<<16;archive.writestr(info,p.read_bytes())
 pending.replace(out)
 errors=inspect(out)
 if errors:print(errors);return 1
 digest=sha(out);out.with_suffix('.zip.sha256').write_text(digest+'  '+out.name+'\n')
 save(ROOT/'build/reports/package_seal.json',{'status':'PASS','sha256':digest,'source_digest':state,'members':len(files),'scope':'cumulative application source; NON_RELEASE_WORK'})
 print('PASS complete application source ZIP: '+str(out));return 0
if __name__=='__main__':sys.exit(main())
