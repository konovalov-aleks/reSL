/* The original ShortLine game uses an embedded VGA font.
   This utility is based on the code of the original game, which is responsible
   for searching for fonts in the memory of the video device.

   Use TurboC compiler to build this utility, and then run it in DOS.
*/

#include <dos.h>
#include <stdio.h>

/* 1b06:0228 */
char far* loadSmallFont(int fontSize)
{
    /*
        This algorithm scans video memory, trying to find character glyphs there.

        1. The font is monospace => each glyph has a fixed size: fontSize bytes.

        2. The encoding is cp437 => characters 0x00 and 0x20 are spaces - all
        pixels are transparent (the entire glyph is filled with zeros);
        the 0xDB symbol is a black box - it's entirely filled with black pixels
        (the glyph is filled with 0xFF).
    */
    char far* videoMem = MK_FP(0xC000, 0x0000);
    char far* ptr;
    int i, ok;

    for (; videoMem < MK_FP(0xC000, 0xFFFF); ++videoMem) {
        ok = 1;

        /* check the symbol 0x00 */
        for (i = 0; i < fontSize; ++i) {
            if (videoMem[i] != 0) {
                ok = 0;
                break;
            }
        }
        if (!ok)
            continue;

        /* check the symbol 0x20 */
        ptr = &videoMem[fontSize * 0x20];
        for (i = 0; i < fontSize; ++i) {
            if (ptr[i] != 0) {
                ok = 0;
                break;
            }
        }
        if (!ok)
            continue;

        /* check the symbol 0xDB - expected a black box */
        ptr = &videoMem[fontSize * 0xDB];
        for (i = 0; i < fontSize; ++i) {
            if (ptr[i] != '\xFF') {
                ok = 0;
                break;
            }
        }
        if (ok)
            return videoMem;
    }
    return NULL;
}

void setVideoMode(int mode)
{
    struct REGPACK regs;
    regs.r_ax = mode;
    intr(0x10, &regs);
}

int main()
{
    const int fontSize = 14;
    int i;
    char far* ptr;
    FILE* f;

    setVideoMode(0x10); /* 640x350 16 colors */
    ptr = loadSmallFont(fontSize);
    setVideoMode(0x03);
    f = fopen("small_font.cpp", "wb");
    if (!f) {
        perror("unable to open the file for writing\n");
        return 1;
    }
    fputs("//\n"
          "// This is a generated file, do not change it manually.\n"
          "// Use the \"extract_vga_font\" utility instead\n"
          "//\n"
          "#include \"small_font.h\"\n"
          "\n"
          "#include <cstdint>\n"
          "\n"
          "namespace resl {\n"
          "\n"
          "// extracted from the VGA video memory\n", f);
    fprintf(f, "/* C000:%04x : %d bytes */\n"
               "const std::uint8_t g_font%dData[] = {",
            FP_OFF(ptr), 256 * fontSize, fontSize);
    for (i = 0; i < 256 * fontSize; ++i) {
        if (i % 12 == 0)
            fputs("\n    ", f);
        fprintf(f, "0x%02X, ", (unsigned char)ptr[i]);
    }
    fputs("\n"
          "};\n"
          "\n"
          "} // namespace resl\n", f);
    fclose(f);
    return 0;
}
