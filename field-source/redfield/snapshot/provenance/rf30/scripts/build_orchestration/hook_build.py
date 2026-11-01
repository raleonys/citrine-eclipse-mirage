"""Internal source-bound MinHook materialization shared by build/verification."""
import hashlib
import json
from pathlib import Path
import re
import shutil

def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def verify_patch(root):
    record=json.loads((root/'metadata/MINHOOK_BUILD.json').read_text(encoding="utf-8"))
    for key in ['source','replacement','patch']:
        if sha(root/record[key])!=record[key+'_sha256']:
            raise ValueError('MINHOOK_PIN_MISMATCH:'+key)
    if (root/'third_party/minhook/UPSTREAM_COMMIT.txt').read_text(encoding="utf-8").strip()!=record['commit']:
        raise ValueError('MINHOOK_COMMIT_MISMATCH')
    # Evaluate only the exact existing patch's five marker substitutions and
    # two literal replacements. Refuse a changed instruction shape.
    script=(root/record['patch']).read_text(encoding="utf-8")
    replacements=re.findall(r'_replace_between\(\n("[^"\n]*")\n("[^"\n]*")\n\[=\[(.*?)\]=\]\)',script,re.S)
    if len(replacements)!=5: raise ValueError('MINHOOK_PATCH_SHAPE')
    # UTF-8 (not UTF-8-SIG) intentionally retains the upstream EF BB BF BOM.
    # Never let a Windows ANSI code page change pinned source bytes.
    text=(root/record['source']).read_text(encoding="utf-8")
    for start,end,body in replacements:
        start=json.loads(start);end=json.loads(end)
        if text.count(start)!=1 or text.count(end)!=1:raise ValueError('MINHOOK_PATCH_MARKER')
        i=text.index(start);j=text.index(end,i)+len(end);text=text[:i]+body+text[j:]
    text=text.replace('Unfreeze(&threads);','status = FinishFreeze(&threads, status);')
    old='static MH_STATUS EnableAllHooksLL(BOOL enable)\n{\n    MH_STATUS status = MH_OK;'
    new=old+'\n    if (!RetryRetainedThreadOwnership())\n        return MH_ERROR_MEMORY_PROTECT;'
    if text.count(old)!=1:raise ValueError('MINHOOK_PATCH_FINAL_MARKER')
    text=text.replace(old,new)
    if text.encode("utf-8")!=(root/record['replacement']).read_bytes():raise ValueError('MINHOOK_PATCH_REPLAY_MISMATCH')
    if 'NtGetNextThread' not in text or 'PRELOADER_L_MH_SAFE_FREEZE_V1' not in text:raise ValueError('MINHOOK_SAFE_FREEZE_MISSING')
    return record

def prepare(root,build):
    record=verify_patch(root)
    vendor=build/'minhook_safe_freeze'
    shutil.copytree(root/'third_party/minhook/upstream',vendor,dirs_exist_ok=True)
    shutil.copyfile(root/record['replacement'],vendor/'src/hook.c')
    return vendor
