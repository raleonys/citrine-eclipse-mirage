#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
# Current runtime/source and canonical behavioral entrypoint must use release-independent
# contract terminology. Historical release/codename lineage belongs in CHANGELOG,
# release/governance metadata, migration ledgers, or explicit provenance fields.
scopes=[root/'src',root/'include',root/'tests'/'native',root/'scripts'/'test_regressions.py']
forbidden=('3.2.3','MELOTRIX','3.2.1','CERVRAX')
errors=[]
checked=0
for base in scopes:
    files=[base] if base.is_file() else sorted(p for p in base.rglob('*') if p.is_file())
    for p in files:
        if p.suffix.lower() not in {'.cpp','.hpp','.h','.c','.py'}: continue
        checked+=1
        text=p.read_text(errors='replace')
        for token in forbidden:
            if token in text:
                errors.append(f'{p.relative_to(root)}: stale current-source identity token {token!r}')
if errors:
    print('CURRENT_IDENTITY_HYGIENE: FAIL')
    for e in errors: print(' -',e)
    sys.exit(1)
print(f'CURRENT_IDENTITY_HYGIENE: PASS ({checked} active runtime/native/entrypoint files)')
