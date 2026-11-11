# SPDX-License-Identifier: GPL-3.0-only
"""Observe actual Win32 directory-leaf behavior inside one owned fixture root."""
import ctypes
from ctypes import wintypes
import json
from pathlib import Path
import platform
import tempfile

NAMES = ('ordinary', 'COM10', 'CON', 'con.txt', 'COM1',
         'CONIN$', 'conin$', 'CONIN$.txt', 'CONIN$ .txt',
         'CONOUT$', 'conout$', 'CONOUT$.txt', 'CONOUT$ .txt',
         'xCONIN$', 'CONIN$_', 'COM\u00b9', 'COM\u00b2', 'COM\u00b3',
         'COM\u2074', 'safe\u202ename')


def run():
    if platform.system() != 'Windows':
        raise RuntimeError('This diagnostic must execute on native Windows')
    api = ctypes.WinDLL('kernel32', use_last_error=True)
    api.CreateDirectoryW.argtypes = (wintypes.LPCWSTR, ctypes.c_void_p)
    api.CreateDirectoryW.restype = wintypes.BOOL
    api.RemoveDirectoryW.argtypes = (wintypes.LPCWSTR,)
    api.RemoveDirectoryW.restype = wintypes.BOOL
    api.GetFileAttributesW.argtypes = (wintypes.LPCWSTR,)
    api.GetFileAttributesW.restype = wintypes.DWORD
    rows = []
    Path('build').mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='win32-leaf-', dir='build') as directory:
        root = Path(directory).resolve()
        for name in NAMES:
            target = str(root / name)
            ctypes.set_last_error(0)
            created = bool(api.CreateDirectoryW(target, None))
            error = 0 if created else ctypes.get_last_error()
            attributes = api.GetFileAttributesW(target)
            is_directory = attributes != 0xffffffff and bool(attributes & 0x10)
            if created and not api.RemoveDirectoryW(target):
                raise OSError(ctypes.get_last_error(), 'owned directory cleanup failed')
            rows.append({'name': name, 'created': created, 'error': error,
                         'directory_attribute': is_directory})
    if not rows[0]['created']:
        raise RuntimeError('Ordinary owned-directory positive control failed')
    report = {'schema': 'citrine.win32-leaf-observation.v1',
              'api': 'CreateDirectoryW/GetFileAttributesW/RemoveDirectoryW',
              'native_windows': True, 'rows': rows}
    output = Path('build/win32-leaf-observations.json')
    output.write_text(json.dumps(report, indent=2, ensure_ascii=True) + '\n')
    print(json.dumps(report, indent=2, ensure_ascii=True))


if __name__ == '__main__':
    run()
