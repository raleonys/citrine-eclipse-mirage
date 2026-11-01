#!/usr/bin/env python3
from pathlib import Path
import hashlib,sys
r=Path(__file__).resolve().parents[1]
pin="8af6b4acae5a9388fd742b56fa79ece89d96f823"
cm=(r/"CMakeLists.txt").read_text(encoding="utf-8",errors="replace")
base=r/"third_party/minhook/upstream"
req=["include/MinHook.h","src/buffer.c","src/buffer.h","src/hook.c","src/trampoline.c","src/trampoline.h","src/hde/hde64.c","src/hde/hde64.h","src/hde/pstdint.h","src/hde/table64.h","LICENSE.txt"]
errors=[]
for rel in req:
    if not (base/rel).is_file(): errors.append("missing vendored "+rel)
cf=r/"third_party/minhook/UPSTREAM_COMMIT.txt"
if not cf.is_file() or cf.read_text(encoding="ascii",errors="ignore").strip()!=pin: errors.append("commit marker mismatch")
for forbidden in ["FetchContent", "GIT_REPOSITORY", "PRELOADER_L_MINHOOK_SOURCE_DIR", "add_subdirectory(${_preloader_l_minhook_patched}"]:
    if forbidden in cm: errors.append("network/external/upstream-cmake path remains: "+forbidden)
for required in ["add_library(minhook STATIC", "PatchMinHookSafeFreeze.cmake", "third_party/minhook/upstream"]:
    if required not in cm: errors.append("vendored build token missing: "+required)
manifest=r/"third_party/minhook/UPSTREAM_MANIFEST.sha256"
if base.is_dir() and manifest.is_file():
    rows=[]
    for p in sorted(x for x in base.rglob('*') if x.is_file()):
        rel=p.relative_to(base).as_posix(); rows.append(hashlib.sha256(p.read_bytes()).hexdigest()+"  "+rel)
    expected=[x.strip() for x in manifest.read_text(encoding="ascii",errors="ignore").splitlines() if x.strip()]
    if rows!=expected: errors.append("vendored manifest mismatch")
else:
    errors.append("vendored manifest missing")
if errors:
    print("VENDORED_MINHOOK_POLICY: FAIL")
    for e in errors: print(" -",e)
    sys.exit(1)
print("VENDORED_MINHOOK_POLICY: PASS")
