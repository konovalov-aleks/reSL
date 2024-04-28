from cpp_writer import CPPWriter

import struct

# 1d63:0000 - 24 bytes
data = b'\xf7\xfd\x09\x03\x09\xfd\xf7\x03\xf7\xfd\xf7\x03\x09\xfd\xf7\xfd\x09\xfd\x09\x03\x09\x03\xf7\x03'

with CPPWriter('semaphore_glyph_bias.cpp') as w:
    w.writeln('#include "semaphore_glyph_bias.h"')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()
    w.writeln('/* 1d63:0000 - 24 bytes */')
    w.writeln('const SemaphoreGlyphBias g_semaphoreGlyphBias[6][2] = {')
    with w.indent():
        for i in range(6):
            # B uint8_t
            # b int8_t
            values = struct.unpack('<bbbb', data[:4])
            data = data[4:]
            w.writeln('{ { %d, %d }, { %d, %d } },' % values)
    w.writeln('};')
    w.writeln()
    w.writeln('} // namespace resl')
