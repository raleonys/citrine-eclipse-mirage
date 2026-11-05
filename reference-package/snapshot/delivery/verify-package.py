from pathlib import Path
import hashlib,json
root=Path(__file__).resolve().parents[1]
m=json.loads((root/'package-manifest.json').read_text(encoding='utf-8-sig'))
for row in m['files']:
 p=(root/row['path']).resolve();assert p.is_relative_to(root),row['path']
 assert p.stat().st_size==row['bytes'],row['path']
 with p.open('rb') as f:assert hashlib.file_digest(f,'sha256').hexdigest()==row['sha256'],row['path']
print('PASS:',len(m['files']),'manifested files; no target execution')
