import struct
from cpp_writer import CPPWriter

# 1d7d:24da - 168 bytes
data = b'\xfe\xf2\x07\x00\x20\x50\x8b\x87\x86\x84\x44\x28\x10\x10\x10\x10\x10\x10\x10\x20\x70\xfb\xff\xfe\xfc\x7c\x38\x30\x30\x30\x30\x30\x30\x10\x20\x00\xd0\x03\xe8\x1f\x50\x1f\xfe\xf2\x07\x05\x10\x28\x44\xbe\xbf\xb4\x88\x50\x10\x10\x10\x10\x10\x10\x10\x10\x38\x7c\xfe\xff\xfc\xf8\x70\x30\x30\x30\x30\x30\x30\x10\x40\x3f\xa0\x3f\x40\x07\x80\x01\xfb\xf1\xf1\x00\x04\x0a\xd1\xe1\x61\x21\x22\x14\x04\x04\x04\x04\x04\x04\x04\x04\x0e\xdf\xff\x7f\x3f\x3e\x1c\x0c\x0c\x0c\x0c\x0c\x0c\x04\x00\x06\xc0\x0d\xf8\x17\xf8\x0d\xfb\xf1\xf1\x05\x08\x14\x22\x7d\xfd\x2d\x11\x0a\x04\x04\x04\x04\x04\x04\x00\x08\x1c\x3e\x7f\xff\x3f\x1f\x0e\x0c\x0c\x0c\x0c\x0c\x0c\x08\xfe\x03\xfe\x05\x70\x03\x80\x02'

with CPPWriter('semaphore_glyph.cpp') as w:
    w.writeln('#include "semaphore_glyph.h"')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()
    w.writeln('/* 1d7d:24da : 168 bytes */')
    w.writeln('const SemaphoreGlyph g_semaphoreGlyphs[2][2] = {')
    with w.indent():
        for i in range(2):
            w.writeln('{')
            with w.indent():
                for j in range(2):
                    w.writeln('{ // [%d][%d]' % (i, j))
                    with w.indent():
                        values = struct.unpack('<bbbb', data[:4])
                        data = data[4:]
                        w.writeln('%d, %d, %d, %d,' % values)

                        w.writeln('{')
                        with w.indent():
                            for c in data[:15]:
                                w.writeWithWrap('0x%02X, ' % c)
                            data = data[15:]
                        w.writeln('},')

                        w.writeln('{')
                        with w.indent():
                            for c in data[:15]:
                                w.writeWithWrap('0x%02X, ' % c)
                            data = data[15:]
                        w.writeln('},')

                        w.writeln('{')
                        with w.indent():
                            for c in data[:8]:
                                w.writeWithWrap('0x%02X, ' % c)
                            data = data[8:]
                        w.writeln('}')
                    w.writeln('},' if j != 1 else '}')
            w.writeln('},' if i != 1 else '}')
    w.writeln('};')
    w.writeln()
    w.writeln('} // namespace resl')
