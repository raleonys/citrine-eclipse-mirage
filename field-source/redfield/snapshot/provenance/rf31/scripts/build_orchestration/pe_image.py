# SPDX-License-Identifier: GPL-3.0-only
"""Bounded immutable x64 PE view shared by local acceptance and Golden gates.

Adapted from the pinned HF19 Golden readers: explicit byte ownership replaces
path/global-policy coupling; every RVA read is backed by one bounded section.
"""
import re
import struct


class PEError(ValueError):
    pass


def unpack(data, fmt, offset, end=None):
    limit = len(data) if end is None else min(end, len(data))
    if offset < 0 or offset + struct.calcsize(fmt) > limit:
        raise PEError('PE_TRUNCATED_OR_OUT_OF_RANGE')
    return struct.unpack_from(fmt, data, offset)


def checksum(data, offset):
    unpack(data, '<I', offset)
    if offset % 2:
        raise PEError('CHECKSUM_OFFSET_UNALIGNED')
    total = 0
    for index in range(0, len(data), 2):
        word = 0 if offset <= index < offset+4 else data[index] | ((data[index+1] if index+1 < len(data) else 0) << 8)
        total += word
        total = (total & 0xffff) + (total >> 16)
    total = (total & 0xffff) + (total >> 16)
    return (total + len(data)) & 0xffffffff


class Image:
    def __init__(self, data):
        self.data = bytes(data)
        b = self.data
        if len(b) < 64 or b[:2] != b'MZ':
            raise PEError('DOS_HEADER')
        self.pe, = unpack(b, '<I', 0x3c)
        if self.pe < 64 or self.pe % 4 or b[self.pe:self.pe+4] != b'PE\0\0':
            raise PEError('PE_SIGNATURE_OR_ALIGNMENT')
        self.coff = self.pe+4
        self.machine, count = unpack(b, '<HH', self.coff)
        self.optional_size, self.characteristics = unpack(b, '<HH', self.coff+16)
        self.optional = self.coff+20
        if self.machine != 0x8664 or not 1 <= count <= 96 or not self.characteristics & 0x2000:
            raise PEError('X64_DLL_REQUIRED')
        if self.optional_size < 240:
            raise PEError('DECLARED_OPTIONAL_HEADER_TRUNCATED')
        unpack(b, '<B', self.optional+self.optional_size-1)
        self.magic = self.u16(0)
        if self.magic != 0x20b or self.u32(108) != 16:
            raise PEError('PE32_PLUS_16_DIRECTORIES_REQUIRED')
        self.section_table_end = self.optional+self.optional_size+count*40
        self.sections = []
        for index in range(count):
            offset = self.optional+self.optional_size+index*40
            unpack(b, '<40s', offset)
            try:
                name = b[offset:offset+8].split(b'\0', 1)[0].decode('ascii')
            except UnicodeError as error:
                raise PEError('NON_ASCII_SECTION_NAME') from error
            vs, va, raw_size, raw = unpack(b, '<IIII', offset+8)
            flags, = unpack(b, '<I', offset+36)
            if raw_size and (raw < self.section_table_end or raw+raw_size > len(b)):
                raise PEError('SECTION_FILE_RANGE')
            if va+max(vs, raw_size) > 0x100000000:
                raise PEError('SECTION_VIRTUAL_RANGE_OVERFLOW')
            self.sections.append(dict(name=name, va=va, vs=vs, raw=raw, size=raw_size, flags=flags))

    def optional_value(self, fmt, relative):
        return unpack(self.data, fmt, self.optional+relative, self.optional+self.optional_size)[0]

    def u16(self, relative):
        return self.optional_value('<H', relative)

    def u32(self, relative):
        return self.optional_value('<I', relative)

    def u64(self, relative):
        return self.optional_value('<Q', relative)

    def directory(self, index):
        if not 0 <= index < 16:
            raise PEError('DIRECTORY_INDEX')
        return unpack(self.data, '<II', self.optional+112+index*8, self.optional+self.optional_size)

    def mapped(self, rva, size=1):
        if rva < 0 or size <= 0 or rva+size > 0x100000000:
            raise PEError('INVALID_RVA_RANGE')
        matches = [s for s in self.sections if s['va'] <= rva and rva+size <= s['va']+s['size']]
        if len(matches) != 1:
            raise PEError('UNMAPPED_OR_OVERLAPPING_RVA')
        s = matches[0]
        return s['raw']+rva-s['va'], s['raw']+s['size']

    def read_rva(self, rva, fmt):
        offset, end = self.mapped(rva, struct.calcsize(fmt))
        return unpack(self.data, fmt, offset, end)

    def string(self, rva):
        offset, limit = self.mapped(rva)
        end = self.data.find(b'\0', offset, min(limit, offset+512))
        if end < 0:
            raise PEError('UNTERMINATED_NAME')
        try:
            return self.data[offset:end].decode('ascii')
        except UnicodeError as error:
            raise PEError('NON_ASCII_NAME') from error

    def section_for(self, rva, size=1):
        self.mapped(rva, size)
        return next(s for s in self.sections if s['va'] <= rva and rva+size <= s['va']+s['size'])

    def exports(self, native=False):
        rva, size = self.directory(0)
        if not rva or size < 40:
            raise PEError('EXPORT_DIRECTORY')
        offset, _ = self.mapped(rva, 40)
        name_rva, base, nf, nn, functions, names, ordinals = unpack(self.data, '<IIIIIII', offset+12)
        wanted = {'DllMain':1, 'SURGE':2, 'DllShutdown':3}
        if native:
            wanted.update(RFTestStatus=100, RFTestSetFaults=101, RFTestTransport=102)
        if nn != len(wanted) or not 1 <= nf <= 4096:
            raise PEError('EXPORT_COUNT')
        self.mapped(functions, nf*4); self.mapped(names, nn*4); self.mapped(ordinals, nn*2)
        result, indexes = {}, set()
        for index in range(nn):
            nr, = self.read_rva(names+index*4, '<I')
            name = self.string(nr)
            ordinal_index, = self.read_rva(ordinals+index*2, '<H')
            if ordinal_index >= nf or name in result or ordinal_index in indexes:
                raise PEError('EXPORT_INDEX_OR_DUPLICATE')
            function, = self.read_rva(functions+ordinal_index*4, '<I')
            if not function or rva <= function < rva+size:
                raise PEError('NULL_OR_FORWARDED_EXPORT')
            self.mapped(function)
            result[name] = (base+ordinal_index, function)
            indexes.add(ordinal_index)
        for index in range(nf):
            function, = self.read_rva(functions+index*4, '<I')
            if function and index not in indexes:
                raise PEError('UNNAMED_EXPORT')
        if {k:v[0] for k,v in result.items()} != wanted:
            raise PEError('PUBLIC_EXPORT_CONTRACT')
        return result, self.string(name_rva)

    def imports(self):
        rva, size = self.directory(1)
        if not rva or not 20 <= size <= 1024*1024:
            raise PEError('IMPORT_DIRECTORY')
        self.mapped(rva, size)
        result = {}
        for index in range(min(size//20, 4096)):
            row = self.read_rva(rva+index*20, '<IIIII')
            if not any(row):
                if not result:
                    raise PEError('EMPTY_IMPORT_DIRECTORY')
                return result
            name = self.string(row[3]).upper()
            if not re.fullmatch(r'[A-Z0-9_.-]+', name) or name in result:
                raise PEError('IMPORT_NAME_OR_DUPLICATE')
            table = row[0] or row[4]
            functions = []
            for slot in range(8192):
                value, = self.read_rva(table+slot*8, '<Q')
                if not value:
                    break
                if value & (1 << 63):
                    functions.append('#'+str(value & 0xffff))
                else:
                    self.mapped(value, 3)
                    functions.append(self.string(value+2))
            else:
                raise PEError('IMPORT_THUNK_BUDGET')
            result[name] = functions
        raise PEError('UNTERMINATED_IMPORT_DIRECTORY')

    def resources(self):
        rva, size = self.directory(2)
        if not rva or not 16 <= size <= 1024*1024:
            raise PEError('RESOURCE_DIRECTORY')
        base, _ = self.mapped(rva, size)
        visited, leaves = set(), []
        budget = [0, 0]
        def span(relative, length):
            if relative < 0 or length < 0 or relative+length > size:
                raise PEError('RESOURCE_RANGE')
            return base+relative
        def walk(relative, path):
            if len(path) > 8 or relative in visited or budget[0] >= 256:
                raise PEError('RESOURCE_CYCLE_OR_DEPTH_BUDGET')
            visited.add(relative); budget[0] += 1
            offset = span(relative, 16)
            named, ids = unpack(self.data, '<HH', offset+12)
            count = named+ids; budget[1] += count
            if budget[1] > 1024:
                raise PEError('RESOURCE_ENTRY_BUDGET')
            span(relative+16, count*8)
            for index in range(count):
                name, child = unpack(self.data, '<II', offset+16+index*8)
                if name & 0x80000000:
                    raise PEError('NAMED_RESOURCE_NOT_ALLOWLISTED')
                if child & 0x80000000:
                    walk(child & 0x7fffffff, path+[name])
                else:
                    data_rva, length, cp, reserved = unpack(self.data, '<IIII', span(child, 16))
                    if reserved or not length:
                        raise PEError('RESOURCE_DATA_ENTRY')
                    data_offset, _ = self.mapped(data_rva, length)
                    if not rva <= data_rva or data_rva+length > rva+size:
                        raise PEError('RESOURCE_PAYLOAD_OUTSIDE_DIRECTORY')
                    leaves.append((path+[name], self.data[data_offset:data_offset+length], cp))
            visited.remove(relative)
        walk(0, [])
        return leaves


def version_tree(data):
    budget = [0]
    def node(offset, limit, depth):
        if depth > 8 or budget[0] >= 256:
            raise PEError('VERSIONINFO_NODE_BUDGET')
        budget[0] += 1
        length, value_length, kind = unpack(data, '<HHH', offset, limit)
        end = offset+length
        if length < 8 or end > limit or kind not in {0, 1}:
            raise PEError('VERSIONINFO_NODE_RANGE')
        pos = offset+6; start = pos
        while True:
            word, = unpack(data, '<H', pos, end); pos += 2
            if word == 0:
                break
        key = data[start:pos-2].decode('utf-16le')
        pos = (pos+3) & ~3
        nbytes = value_length*(2 if kind == 1 else 1)
        if pos+nbytes > end:
            raise PEError('VERSIONINFO_VALUE_RANGE')
        value = data[pos:pos+nbytes]
        children = []
        pos = (pos+nbytes+3) & ~3
        while pos < end:
            if end-pos <= 3 and not any(data[pos:end]):
                break
            child, pos = node(pos, end, depth+1)
            children.append(child); pos = (pos+3) & ~3
        return dict(key=key, kind=kind, value=value, children=children), end
    root, end = node(0, len(data), 0)
    if len(data)-end > 3 or any(data[end:]):
        raise PEError('VERSIONINFO_TRAILING_DATA')
    return root
