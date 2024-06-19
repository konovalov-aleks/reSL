from cpp_writer import CPPWriter

import struct

data = b'\x01\x03\x05\x05\x03\x01\x03\x05\x05\x03\x01\x03\x05\x05\x03\x01\x03\x05\x05\x03\x01\x03\x05\x05\x03\x01\x03\x05\x05\x03\x01\x05\x05\x05\x05\x01\x03\x05\x05\x03\x01\x03\x05\x05\x03\x01\x05\x05\x05\x05\x01\x03\x05\x05\x03\x01\x03\x05\x05\x03\x01\x03\x05\x05\x03\x01\x03\x05\x05\x03\x01\x03\x03\x03\x03'

with CPPWriter('carriage_bias.cpp') as w:
    w.writeln('#include "carriage_bias.h"')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()

    w.writeln('/* 1d7d:2360 : 75 bytes */')
    w.writeln('const std::int8_t g_carriageYBiases[15][5] = {')
    with w.indent():
        for i in range(15):
            values = struct.unpack('<bbbbb', data[:5])
            w.writeln('{ %d, %d, %d, %d, %d }, ' % values)
            data = data[5:]
    w.writeln('};')

    w.writeln()
    w.writeln('} // namespace resl')
