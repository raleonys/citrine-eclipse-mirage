# SPDX-License-Identifier: GPL-3.0-only

"""Reject export-table mutations of the actual linked application DLL."""
import hashlib
from pathlib import Path
import struct
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
from check_dll_exports import inspect
from verify_corvexon_golden_trail import PE, PEError, u32


def main():
    original = Path(sys.argv[1])
    raw = original.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    assert inspect(original, ROOT)['result'] == 'PASS_PUBLIC_C_EXPORT_CONTRACT'
    pe = PE(original)
    er, _ = pe.directory(0)
    eo = pe.rva2off(er)
    funcs = pe.rva2off(u32(pe.b, eo+28))
    names = pe.rva2off(u32(pe.b, eo+32))
    ords = pe.rva2off(u32(pe.b, eo+36))
    surge_ordinal, surge_rva, _ = pe.exports()['SURGE']
    surge_slot = funcs + 4*(surge_ordinal-u32(pe.b, eo+16))
    owner_index = next(index for index, (_, va, span, _, _, _) in enumerate(pe.sections)
                       if va <= surge_rva < va+span)
    owner_flags = pe.opt+pe.optsz+owner_index*40+36
    mutations = {
        'wrong_ordinal_base': (eo+16, struct.pack('<I', 10)),
        'missing_function_body': (funcs, struct.pack('<I', 0)),
        'forwarded_function': (funcs, struct.pack('<I', er)),
        'hidden_unnamed_entry': (eo+20, struct.pack('<I', u32(pe.b, eo+20)+1)),
        'alias_ordinal_replaces_function': (ords, struct.pack('<H', 1)),
        'unexpected_name': (pe.rva2off(u32(pe.b, names)), b'UnknowN\0'),
        'wrong_machine': (pe.coff, struct.pack('<H', 0x14c)),
        'function_body_in_headers': (surge_slot, struct.pack('<I', 0x40)),
        'unmapped_function_body': (surge_slot, struct.pack('<I', 0xffffffff)),
        'non_executable_function_body': (owner_flags, struct.pack('<I', u32(pe.b, owner_flags) & ~0x20000000))
    }
    with tempfile.TemporaryDirectory() as folder:
        for name, (offset, replacement) in mutations.items():
            data = bytearray(raw)
            data[offset:offset+len(replacement)] = replacement
            path = Path(folder) / (name+'.dll')
            path.write_bytes(data)
            before = hashlib.sha256(data).hexdigest()
            try:
                assert inspect(path, ROOT)['result'] == 'FAIL', name+' was accepted'
            except (PEError, ValueError, KeyError):
                pass  # malformed PE rejection is also a failed contract
            assert hashlib.sha256(path.read_bytes()).hexdigest() == before
    assert hashlib.sha256(original.read_bytes()).hexdigest() == digest
    print(f'PASS: actual DLL baseline and {len(mutations)} export/machine mutations; all checks read-only')


if __name__ == '__main__':
    main()
