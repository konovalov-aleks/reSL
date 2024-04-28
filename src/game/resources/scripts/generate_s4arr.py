from cpp_writer import CPPWriter

import struct

# 1d60:0000 - 48 bytes
data = b'\x00\x00\x01\x00\x01\x00\x07\x00\x00\x00\x04\x00\x00\x01\x0a\x00\x00\x00\x02\x00\x01\x01\x09\x00\x01\x00\x05\x00\x00\x01\x00\x00\x00\x00\x03\x00\x01\x01\x08\x00\x01\x00\x06\x00\x00\x01\x0b\x00'

with CPPWriter('s4arr.cpp') as w:
    w.writeln('#include "s4arr.h"')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()
    w.writeln('/* 1d60:0000 - 48 bytes */')
    w.writeln('const s4 s4arr[6][2] = {')
    with w.indent():
        for i in range(6):
            # B uint8_t
            # b int8_t
            values = struct.unpack('<bbBBbbBB', data[:8])
            data = data[8:]
            w.writeln('{ { %d, %d, %d, %d }, { %d, %d, %d, %d } },' % values)
    w.writeln('};')
    w.writeln()
    w.writeln('} // namespace resl')
