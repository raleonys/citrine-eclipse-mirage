# SPDX-License-Identifier: GPL-3.0-only
"""Observe actual Win32 directory-leaf behavior inside one owned fixture root."""
import ctypes
from ctypes import wintypes
import json
import os
from pathlib import Path
import platform
import tempfile
import time

NAMES = ('ordinary', 'COM10', 'CON', 'con.txt', 'COM1',
         'CONIN$', 'conin$', 'CONIN$.txt', 'CONIN$ .txt',
         'CONOUT$', 'conout$', 'CONOUT$.txt', 'CONOUT$ .txt',
         'xCONIN$', 'CONIN$_', 'COM\u00b9', 'COM\u00b2', 'COM\u00b3',
         'COM\u2074', 'safe\u202ename')


def extended(path):
    text = str(path)
    if text.startswith('\\\\?\\'):
        return text
    return '\\\\?\\UNC\\' + text[2:] if text.startswith('\\\\') else '\\\\?\\' + text


def save(report):
    output = Path('build/win32-leaf-observations.json')
    if output.is_symlink() or (output.exists() and
            (not output.is_file() or output.stat().st_nlink != 1)):
        raise ValueError('Unsafe observation output')
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=output.parent, prefix='.leaf-', delete=False) as stream:
            temporary = Path(stream.name)
            stream.write((json.dumps(report, indent=2, ensure_ascii=True) + '\n').encode())
        os.replace(temporary, output)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


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
    report = {'schema': 'citrine.win32-leaf-observation.v1',
              'api': 'CreateDirectoryW/GetFileAttributesW/RemoveDirectoryW',
              'native_windows': True, 'windows_version': platform.version(),
              'commit': os.environ.get('GITHUB_SHA'), 'result': 'RUNNING', 'rows': []}
    Path('build').mkdir(exist_ok=True)
    save(report)
    try:
        with tempfile.TemporaryDirectory(prefix='win32-leaf-', dir='build') as directory:
            root = Path(directory).resolve()
            for index, name in enumerate(NAMES):
                parent = root / ('case-' + str(index))
                parent.mkdir()
                target = parent / name
                ctypes.set_last_error(0)
                created = bool(api.CreateDirectoryW(str(target), None))
                error = 0 if created else ctypes.get_last_error()
                ctypes.set_last_error(0)
                attributes = api.GetFileAttributesW(str(target))
                attribute_error = ctypes.get_last_error() if attributes == 0xffffffff else 0
                literal_attributes = api.GetFileAttributesW(extended(target))
                entries = list(os.scandir(parent))
                row = {'name': name, 'created': created, 'create_error': error,
                       'attributes': attributes, 'attribute_error': attribute_error,
                       'directory_attribute': attributes != 0xffffffff and bool(attributes & 0x10),
                       'literal_directory_attribute': literal_attributes != 0xffffffff and bool(literal_attributes & 0x10),
                       'physical_entries': [entry.name for entry in entries]}
                report['rows'].append(row)
                save(report)  # Do not lose the causal observation on cleanup failure.
                if created:
                    ctypes.set_last_error(0)
                    removed = bool(api.RemoveDirectoryW(str(target)))
                    row['ordinary_remove'] = removed
                    row['remove_error'] = 0 if removed else ctypes.get_last_error()
                    if not removed:
                        # Only literal children of our private per-case directory
                        # may be removed through the extended cleanup spelling.
                        for entry in entries:
                            if not entry.is_dir(follow_symlinks=False):
                                raise RuntimeError('Unexpected non-directory fixture child')
                            for attempt in range(10):
                                if api.RemoveDirectoryW(extended(parent / entry.name)):
                                    break
                                time.sleep(.01)
                            else:
                                raise OSError(ctypes.get_last_error(), 'literal owned-directory cleanup failed')
                        row['literal_cleanup'] = True
                save(report)
                print(json.dumps(row, ensure_ascii=True), flush=True)
        first = report['rows'][0]
        if not first['created'] or not first['directory_attribute']:
            raise RuntimeError('Ordinary owned-directory positive control failed')
        report['result'] = 'PASS_OBSERVATIONS'
        save(report)
    except BaseException as error:
        report['result'] = 'FAIL'
        report['failure_type'] = type(error).__name__
        save(report)
        raise


if __name__ == '__main__':
    run()
