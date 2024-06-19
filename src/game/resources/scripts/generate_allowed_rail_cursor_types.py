from cpp_writer import CPPWriter

# 1d3b:0000 : 121 bytes
data = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x28\x15\x00\x00\x00\x00\x00\x00\x00\x00\x28\x3f\x3f\x15\x00\x00\x00\x00\x00\x00\x28\x3f\x3f\x3f\x3f\x15\x00\x00\x00\x00\x00\x16\x3f\x3f\x3f\x3f\x3f\x15\x00\x00\x00\x00\x00\x16\x3f\x3f\x3f\x3f\x3f\x15\x00\x00\x00\x00\x00\x16\x3f\x3f\x3f\x3f\x3f\x01\x00\x00\x00\x00\x00\x16\x3f\x3f\x3f\x2b\x00\x00\x00\x00\x00\x00\x00\x16\x3f\x2b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'

with CPPWriter('allowed_cursor_rail_types.cpp') as w:
    w.writeln('#include "allowed_cursor_rail_types.h"')
    w.writeln()
    w.writeln('#include <cstdint>')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()
    w.writeln('/* 1d3b:0000 : 121 bytes */')
    w.writeln('const std::uint8_t g_allowedRailCursorTypes[11][11] = {')
    with w.indent():
        for tileX in range(11):
            values = []
            for tileY in range(11):
                values.append('0x%02X' % data[tileY])
            w.writeln('{ %s },' % ', '.join(values))
            data = data[11:]
        assert(len(data) == 0)
    w.writeln('};')
    w.writeln()
    w.writeln('} // namespace resl')
