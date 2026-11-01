# SPDX-License-Identifier: GPL-3.0-only

"""Bounded PE resource traversal adapted from Infrastructure MUSTAVEX's walker."""
import struct
from verify_corvexon_golden_trail import PEError, u16, u32


def resource_leaves(pe):
    rva, extent = pe.directory(2)
    if not rva:
        return []
    base = pe.rva2off(rva)
    if extent < 16 or base + extent > len(pe.b):
        raise PEError('Resource directory extent exceeds mapped file')
    if pe.rva2off(rva + extent - 1) != base + extent - 1:
        raise PEError('Resource directory crosses noncontiguous storage')
    leaves, visited = [], set()
    budget = {'entries': 0, 'directories': 0}
    def span(rel, count):
        if rel < 0 or count < 0 or rel > extent or count > extent - rel:
            raise PEError('Resource field outside declared resource range')
        return base + rel
    def name(value):
        if value & 0x80000000:
            rel = value & 0x7fffffff
            size = u16(pe.b, span(rel, 2)) * 2
            pos = span(rel + 2, size)
            return pe.b[pos:pos+size].decode('utf-16le', 'strict')
        return value & 0xffff
    def walk(rel, path, depth=0):
        if depth > 8 or rel in visited or budget['directories'] >= 256:
            raise PEError('Resource cycle/depth/directory budget exceeded')
        pos = span(rel, 16)
        count = u16(pe.b, pos + 12) + u16(pe.b, pos + 14)
        if count > 1024 - budget['entries']:
            raise PEError('Resource entry budget exceeded')
        span(rel + 16, count * 8)
        budget['entries'] += count
        budget['directories'] += 1
        visited.add(rel)
        try:
            for index in range(count):
                pos = base + rel + 16 + index * 8
                ident, value = u32(pe.b, pos), u32(pe.b, pos + 4)
                child = path + [name(ident)]
                if value & 0x80000000:
                    walk(value & 0x7fffffff, child, depth+1)
                else:
                    pos = span(value, 16)
                    data_rva, size, codepage, _ = struct.unpack_from('<IIII', pe.b, pos)
                    data_pos = pe.rva2off(data_rva)
                    if data_pos + size > len(pe.b):
                        raise PEError('Resource payload exceeds EOF')
                    if size and pe.rva2off(data_rva + size - 1) != data_pos + size - 1:
                        raise PEError('Resource payload crosses noncontiguous storage')
                    leaves.append((child, pe.b[data_pos:data_pos+size], codepage))
        finally:
            visited.remove(rel)
    walk(0, [])
    return leaves
