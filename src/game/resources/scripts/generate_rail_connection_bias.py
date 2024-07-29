import struct

from cpp_writer import CPPWriter

# 1d60:0000 - 48 bytes
DATA = b'\x00\x00\x01\x00\x01\x00\x07\x00\x00\x00\x04\x00\x00\x01\x0a\x00\x00\x00\x02\x00\x01\x01\x09\x00\x01\x00\x05\x00\x00\x01\x00\x00\x00\x00\x03\x00\x01\x01\x08\x00\x01\x00\x06\x00\x00\x01\x0b\x00'

with CPPWriter('rail_connection_bias.cpp') as w:
    w.writeln('#include "rail_connection_bias.h"')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()
    w.writeln('/* 1d60:0000 - 48 bytes */')
    w.writeln('const RailConnectionBias g_railConnectionBiases[6][2] = {')
    with w.indent():
        for i in range(6):
            # B uint8_t
            # b int8_t
            x1, y1, u1, p1, x2, y2, u2, p2 = struct.unpack('<bbBBbbBB', DATA[:8])
            # padding
            assert p1 == 0
            assert p2 == 0
            DATA = DATA[8:]
            w.writeln(f'{{ {{ {x1}, {y1}, {u1} }}, {{ {x2}, {y2}, {u2} }} }},')
    w.writeln('};')
    w.writeln()
    w.writeln('} // namespace resl')
