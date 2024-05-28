from cpp_writer import CPPWriter

import struct

# 1d5d:0000 : 48 bytes
data = b'\x00\xff\x03\x01\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x04\x00\x00\x00\x01\x00\xff\x00\x03\x00\xff\x00\x05\x00\xff\x00\x00\x01\xff\xff\x04\x01\xff\xff\x02\x01\x00\xff\x01\x01\x00\xff\x05\x01'

SemType = {
    0: 'RightUp',
    1: 'RightDown',
    2: 'LeftUp',
    3: 'LeftDown',
    -1: 'None',
}

with CPPWriter('rail_type_meta.cpp') as w:
    w.writeln('#include "rail_type_meta.h"')
    w.writeln()
    w.writeln('#include <game/types/semaphore.h>')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()
    w.writeln('/* 1d5d:0000 : 48 bytes */')
    w.writeln('const RailTypeMeta g_railTypeMeta[12] = {')
    with w.indent():
        for i in range(12):
            # B uint8_t
            # b int8_t
            values = struct.unpack('<bbBB', data[:4])
            data = data[4:]
            w.writeln('{ %d, %d, %d, SemaphoreType::%s },' % (*values[:-1], SemType[values[-1]]))
    w.writeln('};')
    w.writeln()
    w.writeln('} // namespace resl')
