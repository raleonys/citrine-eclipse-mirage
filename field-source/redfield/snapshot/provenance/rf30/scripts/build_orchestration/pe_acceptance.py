# SPDX-License-Identifier: GPL-3.0-only
"""Internal scoped PE/CRT acceptance; not a substitute for the five Golden gates."""
import re
import struct


def inspect(data, *, native=False):
    def fail(reason):
        raise ValueError('PE_ACCEPTANCE:' + reason)

    def unpack(fmt, offset):
        if offset < 0 or offset + struct.calcsize(fmt) > len(data):
            fail('TRUNCATED')
        return struct.unpack_from(fmt, data, offset)

    if len(data) < 64 or data[:2] != b'MZ':
        fail('DOS_HEADER')
    pe, = unpack('<I', 0x3c)
    if pe < 64 or data[pe:pe+4] != b'PE\0\0':
        fail('PE_SIGNATURE')
    machine, count = unpack('<HH', pe+4)
    optional_size, characteristics = unpack('<HH', pe+20)
    optional = pe+24
    if machine != 0x8664 or not 1 <= count <= 96 or not characteristics & 0x2000:
        fail('X64_DLL_REQUIRED')
    magic, = unpack('<H', optional)
    directory_count, = unpack('<I', optional+108)
    entry, = unpack('<I', optional+16)
    dll_flags, = unpack('<H', optional+70)
    if (magic != 0x20b or optional_size < 240 or directory_count < 2 or
            dll_flags & 0x160 != 0x160 or not entry):
        fail('OPTIONAL_HEADER_SECURITY_OR_ENTRY')
    sections = []
    for index in range(count):
        offset = optional+optional_size+index*40
        virtual_size, va, raw_size, raw = unpack('<IIII', offset+8)
        flags, = unpack('<I', offset+36)
        if raw_size and (raw < optional+optional_size+count*40 or raw+raw_size > len(data)):
            fail('SECTION_FILE_RANGE')
        sections.append((va, max(virtual_size, raw_size), raw, raw_size, flags))

    def mapped(rva, size=1):
        matches = [s for s in sections if s[0] <= rva and rva+size <= s[0]+s[3]]
        if len(matches) != 1 or size < 0:
            fail('UNMAPPED_OR_OVERLAPPING_RVA')
        s = matches[0]
        return s[2]+rva-s[0], s[2]+s[3]

    def string(rva):
        offset, limit = mapped(rva)
        end = data.find(b'\0', offset, min(limit, offset+512))
        if end < 0:
            fail('UNTERMINATED_NAME')
        try:
            return data[offset:end].decode('ascii')
        except UnicodeError:
            fail('NON_ASCII_NAME')

    if not any(va <= entry < va+span and flags & 0x20000000 for va,span,_,_,flags in sections):
        fail('ENTRY_NOT_EXECUTABLE')
    export_rva, export_size, import_rva, import_size = unpack('<IIII', optional+112)
    if export_size < 40:
        fail('EXPORT_DIRECTORY')
    export_offset, _ = mapped(export_rva, 40)
    base, function_count, name_count, functions, names, ordinals = unpack('<IIIIII', export_offset+16)
    wanted = {'DllMain':1, 'SURGE':2, 'DllShutdown':3}
    if native:
        wanted.update(RFTestStatus=100, RFTestSetFaults=101, RFTestTransport=102)
    if name_count != len(wanted) or not 1 <= function_count <= 1024:
        fail('EXPORT_COUNT')
    function_offset, _ = mapped(functions, function_count*4)
    name_offset, _ = mapped(names, name_count*4)
    ordinal_offset, _ = mapped(ordinals, name_count*2)
    exports = {}
    named_indexes = set()
    for index in range(name_count):
        name_rva, = unpack('<I', name_offset+index*4)
        name = string(name_rva)
        ordinal_index, = unpack('<H', ordinal_offset+index*2)
        if ordinal_index >= function_count or name in exports or ordinal_index in named_indexes:
            fail('EXPORT_INDEX_OR_DUPLICATE')
        function_rva, = unpack('<I', function_offset+ordinal_index*4)
        if not function_rva or export_rva <= function_rva < export_rva+export_size:
            fail('NULL_OR_FORWARDED_EXPORT')
        mapped(function_rva)
        exports[name] = base+ordinal_index
        named_indexes.add(ordinal_index)
    for index in range(function_count):
        value, = unpack('<I', function_offset+index*4)
        if value and index not in named_indexes:
            fail('UNNAMED_EXPORT')
    if exports != wanted:
        fail('PUBLIC_EXPORT_CONTRACT')
    imports = []
    if not import_rva or import_size < 20 or import_size > 1024*1024:
        fail('IMPORT_DIRECTORY')
    offset, _ = mapped(import_rva, import_size)
    terminated = False
    for index in range(import_size//20):
        descriptor = unpack('<IIIII', offset+index*20)
        if not any(descriptor):
            terminated = True
            break
        name = string(descriptor[3]).lower()
        if not name or name in imports:
            fail('IMPORT_NAME')
        imports.append(name)
        if re.match(r'(?:ucrtbase|ucrtbased|vcruntime.*|msvcp.*|msvcr.*|api-ms-win-crt-.*)\.dll$', name):
            fail('DYNAMIC_CRT_IMPORT:' + name)
    if not terminated:
        fail('UNTERMINATED_IMPORT_DIRECTORY')
    # Delay imports cannot be used to hide a dynamic CRT dependency.
    if directory_count > 13:
        delay_rva, delay_size = unpack('<II', optional+112+13*8)
        if delay_rva or delay_size:
            fail('DELAY_IMPORTS_NOT_APPROVED')
    return {'machine':'x64', 'exports':exports, 'imports':imports,
            'static_crt_import_check':'PASS', 'entry_rva':entry,
            'security_flags':dll_flags,
            'scope':'Public export table,nonzero executable entry,ASLR/NX/high entropy,no dynamic CRT imports;not full Golden or runtime coverage'}
