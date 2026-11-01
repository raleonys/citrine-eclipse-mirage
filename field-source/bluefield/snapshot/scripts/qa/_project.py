# SPDX-License-Identifier: GPL-3.0-only
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[2]
OMIT={'build','dist','evidence','__pycache__'}
DERIVED={'SOURCE_MANIFEST.json','PACKAGE_MANIFEST.json','ACCEPTANCE.json'}
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def source_files(root=ROOT):
 return sorted(p for p in root.rglob('*') if p.is_file() and not OMIT.intersection(p.relative_to(root).parts) and p.name not in DERIVED)
def source_digest(root=ROOT):
 h=hashlib.sha256()
 for p in source_files(root):h.update((p.relative_to(root).as_posix()+'\0'+sha(p)+'\n').encode())
 return h.hexdigest()
def save(path,data):path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(data,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
