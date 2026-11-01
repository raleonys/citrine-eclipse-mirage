#!/usr/bin/env python3
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]; e=[]
def need(v,n):
    if not v:e.append(n)
avatar=(r/'src/runtime/avatar_path.cpp').read_text()
hdr=(r/'include/preloader_l/runtime/avatar_path.hpp').read_text()
qi=(r/'src/handlers/image_route.cpp').read_text()
need('GetModuleFileNameW' in avatar and 'WideCharToMultiByte' in avatar,'Unicode module filename + UTF-8 conversion API')
need('snapshot_state()' in avatar and 'state.module_base' in avatar,'loaded DLL module policy')
need("kAsset[]={'s','u','r','g','e','.','p','n','g',0u}" in avatar,'wide surge asset name')
need('GetEnvironmentVariableW' not in avatar and 'LOCALAPPDATA' not in avatar,'no LOCALAPPDATA fallback')
helper=(r/'src/config/module_relative_filesystem.hpp').read_text()
need('#include "../config/module_relative_filesystem.hpp"' in avatar and 'path_detail::join_module_relative' in avatar and 'parent_path()' in helper,'DLL split via pinned filesystem')
need('kAsset,asset_units' in avatar and 'u16string()' in helper and 'result[prefix+i]' in helper,'DLL sibling asset construction via filesystem')
need('resolved beside the loaded preloader_l.dll' in hdr,'header path contract')
need('project fallback beside the loaded DLL: surge.png' in qi,'handler path contract')
if e:
    print('AVATAR_PATH: FAIL'); [print(' -',x) for x in e]; sys.exit(1)
print(f'AVATAR_PATH: PASS ({8-len(e)}/8)')
