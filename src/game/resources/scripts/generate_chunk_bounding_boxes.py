import struct

from cpp_writer import CPPWriter

# 1d78:0000 : 48 bytes
DATA = b'\xe0\xff\xed\xff\x78\x00\x28\x00\x88\xff\xed\xff\x20\x00\x28\x00' \
       b'\xe0\xff\xed\xff\x4c\x00\x3d\x00\x88\xff\x02\x00\x78\x00\x32\x00' \
       b'\xb4\xff\xed\xff\x20\x00\x3d\x00\x88\xff\xf7\xff\x78\x00\x28\x00'

with CPPWriter('chunk_bounding_boxes.cpp') as w:
    w.writeln('#include "chunk_bounding_boxes.h"')
    w.writeln()
    w.writeln('#include <types/rectangle.h>')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()
    w.writeln('/* 1d78:0000 : 48 bytes */')
    w.writeln('const Rectangle g_chunkBoundingBoxes[6] = {')
    with w.indent():
        for i in range(6):
            x1, y1, x2, y2 = struct.unpack('<hhhh', DATA[:8])
            w.writeln('{ %d, %d, %d, %d },' % (x1, y1, x2, y2))
            DATA = DATA[8:]
    assert len(DATA) == 0
    w.writeln('};')
    w.writeln()
    w.writeln('} // namespace resl')
