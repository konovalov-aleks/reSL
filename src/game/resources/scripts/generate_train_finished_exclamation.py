from cpp_writer import CPPWriter

import struct

# 1d7d:864e - 77 bytes
dataLeft = b'\x03\x19\x0c\x00\x00\x30\x00\x00\xc0\x00\x00\x00\x00\x00\x00\x00\x7c\x00\x0f\x80\x01\xf0\x00\x3e\x00\x00\xc0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0\x00\x00\x3e\x00\x00\x01\xf0\x00\x00\x0f\x80\x00\x00\x7c\x00\x00\x00\xc0\x00\x00\x30\x00\x00\x0c\x00\x00' 

# 1d7d:869b - 77 bytes
dataRight = b'\x03\x19\x00\x00\x30\x00\x00\x0c\x00\x00\x03\x00\x00\x00\x3e\x00\x00\x01\xf0\x00\x00\x0f\x80\x00\x00\x7c\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x7c\x00\x0f\x80\x01\xf0\x00\x3e\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x0c\x00\x00\x30'

def writeGlyph(w, name, data):
    width, height = struct.unpack('<BB', data[:2])
    data = data[2:]
    w.writeln('static const GlyphData<%d, %d> %s = {' % (width, height, name))
    with w.indent():
        for c in data:
            w.writeWithWrap('0x%02X, ' % c)
    w.writeln('};')

with CPPWriter('train_finished_exclamation_glyph.cpp') as w:
    w.writeln('#include "train_finished_exclamation_glyph.h"')
    w.writeln()
    w.writeln('#include <graphics/glyph.h>')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()
    w.writeln('/* 1d7d:864e - 77 bytes */')
    writeGlyph(w, 'g_glyphDataTrainFinishedLeft', dataLeft)
    w.writeln()
    w.writeln('/* 1d7d:864e - 77 bytes */')
    writeGlyph(w, 'g_glyphDataTrainFinishedRight', dataRight)
    w.writeln()
    w.writeln('const Glyph& g_glyphTrainFinishedLeftEntrance =')
    w.writeln('    reinterpret_cast<const Glyph&>(g_glyphDataTrainFinishedLeft);')
    w.writeln('const Glyph& g_glyphTrainFinishedRightEntrance = ')
    w.writeln('    reinterpret_cast<const Glyph&>(g_glyphDataTrainFinishedRight);')
    w.writeln()
    w.writeln('} // namespace resl')
