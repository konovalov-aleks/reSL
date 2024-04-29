from cpp_writer import CPPWriter

# 1d7d:888b - 64 bytes
data = b'\x00\x00\x80\x07\x00\x07\x1e\x08\x1e\x08\x08\x00\x08\x00\x00\x00\x00\x3c\x00\x38\xf0\x00\xe0\x04\x00\x04\x04\x0c\x04\x00\x0c\x00\x00\x00\xe0\x01\xe0\x00\x20\x78\x20\x78\x00\x10\x00\x10\x00\x00\x3c\x00\x1c\x00\x00\x0f\x10\x07\x10\x00\x30\x08\x00\x08\x00\x18'

with CPPWriter('impasse_glyph.cpp') as w:
    w.writeln('#include "impasse_glyph.h"')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()

    w.writeln('/* 1d7d:888b - 64 bytes */')
    w.writeln('const ImpasseGlyph g_impasseGlyphs[2] = {')
    with w.indent():
        for i in range(2):
            w.writeln('{')
            with w.indent():
                w.writeln('{')
                with w.indent():
                    for c in data[:16]:
                        w.writeWithWrap('0x%02X, ' % c)
                w.writeln('},')
                w.writeln('{')
                with w.indent():
                    for c in data[16:32]:
                        w.writeWithWrap('0x%02X, ' % c)
                w.writeln('}')
                data = data[32:]
            w.writeln('},')
    w.writeln('};')

    w.writeln()
    w.writeln('} // namespace resl')
