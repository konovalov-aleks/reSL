from cpp_writer import CPPWriter

# 1d7d:86e8 - 128 bytes
data = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1f\x00\x1f\x00\x1b\x00\x15\x00\x0e\xc0\x71\x20\x84\x20\x80\xa0\xa4\xa0\xa0\xa0\xa4\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1f\x00\x1f\x00\x1b\x00\x15\x00\x0e\xc0\x7f\xe0\xff\xe0\xff\xe0\xff\xe0\xff\xe0\xff\x3f\x00\x21\x00\x21\x00\x21\x00\x3f\x00\x20\x1f\x70\x1f\x50\x1b\x50\x15\x50\x0e\x90\x71\x20\x84\xc0\x80\x80\xa4\x80\xa0\x80\xa4\x3f\x00\x3f\x00\x3f\x00\x3f\x00\x3f\x00\x20\x1f\x70\x1f\x70\x1b\x70\x15\x70\x0e\xf0\x7f\xe0\xff\xc0\xff\x80\xff\x80\xff\x80\xff'

with CPPWriter('dispatcher_glyph.cpp') as w:
    w.writeln('#include "dispatcher_glyph.h"')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()

    w.writeln('/* 1d7d:86e8 - 128 bytes */')
    w.writeln('const DispatcherGlyph g_dispatcherGlyphs[2] = {')
    with w.indent():
        for i in range(2):
            w.writeln('{')
            with w.indent():
                for j in range(2):
                    w.writeln('{')
                    with w.indent():
                        for c in data[:32]:
                            w.writeWithWrap('0x%02X, ' % c)
                        data = data[32:]
                    w.writeln('},' if j != 1 else '}')
            w.writeln('},' if i != 1 else '}')
    assert(not data)

    w.writeln('};')

    w.writeln()
    w.writeln('} // namespace resl')
