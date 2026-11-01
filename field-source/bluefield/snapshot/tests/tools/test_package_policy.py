# SPDX-License-Identifier: GPL-3.0-only
import sys,tempfile,zipfile
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'scripts/qa'))
from check_package import inspect
def main():
 path=Path(sys.argv[1]);assert not inspect(path)
 with zipfile.ZipFile(path) as z:members={n:z.read(n) for n in z.namelist()}
 with tempfile.TemporaryDirectory(prefix='preloader_l_zip_negative_') as temp:
  target=Path(temp)/'bad.zip'
  def write(changed={},extra=None,drop=None):
   with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED) as z:
    for n,data in members.items():
     if n!=drop:z.writestr(n,changed.get(n,data))
    if extra:z.writestr(*extra)
  name='preloader_l/src/application/application.cpp';write({name:members[name]+b'\n// altered\n'});assert inspect(target)
  write({'preloader_l/metadata/ACCEPTANCE.json':b'{}'});assert inspect(target)
  write(extra=('preloader_l/../escaped.txt',b'x'));assert inspect(target)
  write(extra=('preloader_l/unlisted.txt',b'x'));assert inspect(target)
  write(drop='preloader_l/CMakeLists.txt');assert inspect(target)
  write(extra=('bluefield/README.md',b'x'));assert inspect(target)
 print('PASS package policy: 6 negative mutations');return 0
if __name__=='__main__':sys.exit(main())
