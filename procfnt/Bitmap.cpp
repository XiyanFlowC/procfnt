#include "Bitmap.h"

Bitmap::Bitmap() : bit_count(4)
{
}

Bitmap::Bitmap(std::string path) : bit_count(4), Graphic(path)
{
}

Bitmap::Bitmap(int w, int h) : Graphic(w, h), bit_count(4)
{
}

int Bitmap::GetBitCount()
{
    return bit_count;
}

void Bitmap::SetBitCount(int value)
{
    int tmp = value;
    while (tmp)
    {
        tmp >>= 1;
        if (tmp & 1) break;
    }
    if (tmp) throw bad_parameter("value", "value must be a power of 2.");
    bit_count = value;
}

void Bitmap::SaveFile()
{
    Palette pl;
    pl.ExtractPalette(*this, 1 << bit_count);
    if (pl.Size() > (1 << bit_count)) throw bad_operation("number of colour kinds exceeded limitation! reduce colour usage or increase the bit count.");
    // if (bit_count != 4) throw bad_operation("for now, only bit_count == 4 is valid.");
    bmpheader hdr{
        {'B', 'M'},
        sizeof(bmpheader) + sizeof(bmpinfo) + (sizeof(word) * (1 << bit_count)) +
          ((bit_count * Size() + 7) >> 3),
        0, 0,
        sizeof(bmpheader) + sizeof(bmpinfo) + (sizeof(word) * (1 << bit_count))
    };
    bmpinfo inf{
        40,
        width,
        height,
        0x1,
        bit_count,
        0,
        (bit_count * Size() + 7) >> 3,
        1000, 1000, // Not really used in anywhere?
        1 << bit_count,
        1 << bit_count
    };

    FILE* file = fopen(fileName.c_str(), "wb");
    if (file == NULL)
    {
        throw bad_operation(std::string("can not open file to write, errno: ") + strerror(errno));
    }
    
    fwrite(&hdr, sizeof(hdr), 1, file);
    fwrite(&inf, sizeof(inf), 1, file);

    int pltsz = pl.Size();
    word* rgba = new word[pltsz];
    for (int i = 0; i < pltsz; ++i)
    {
        try
        {
            rgba[i] = pl.GetColor(i).PackWord();
        }
        catch (bad_parameter&)
        {
            rgba[i] = 0;
        }
    }
    fwrite(rgba, sizeof(word), pl.Size(), file);
    delete[] rgba;

    size_t bytesz = ((static_cast<size_t>(Size()) * bit_count) + 7) >> 3;
    byte* seq = new byte[bytesz];
    
    if (bit_count <= 8)
    {
        int ppb = 8 / bit_count;
        int line = height - 1, col = 0;
        for (int i = 0; i < bytesz; ++i)
        {
            assert(line >= 0);
            byte tmp = 0;
            if (ppb == 8)
            {
                for (int j = 0; j < 8; ++j)
                {
                    tmp |= pl.GetPaletteIndex(pixels[line * width + col + j]) << (7 - j);
                }
                col += 8;
            }
            else if (ppb == 4)
            {
                for (int j = 0; j < 4; ++j)
                {
                    tmp |= pl.GetPaletteIndex(pixels[line * width + col + j]) << (6 - (j << 1));
                }
                col += 4;
            }
            else if (ppb == 2)
            {
                tmp = (pl.GetPaletteIndex(pixels[line * width + col]) << 4) | (pl.GetPaletteIndex(pixels[line * width + col + 1]));
                col += 2;
            }
            else if (ppb == 1)
            {
                tmp = pl.GetPaletteIndex(pixels[line * width + col++]);
            }
            seq[i] = tmp;
            if (col >= width) col = 0, line--;
        }
    }
    else
    {
        throw bad_operation("not supported bpp. please wait for update.");
        int bpp = bit_count / 8;
        // UNDONE: implement it!
    }
    fwrite(seq, sizeof(byte), bytesz, file);
    delete[] seq;

    fclose(file);
}

void Bitmap::LoadFile()
{
    bmpheader hdr;
    bmpinfo inf;
    FILE* file = fopen(fileName.c_str(), "rb");

    if (file == NULL)
    {
        throw bad_operation(std::string("can not open specified file: ") + fileName + ", " + strerror(errno));
    }

    fread(&hdr, sizeof(hdr), 1, file);
    if (hdr.ident[0] != 'B' || hdr.ident[1] != 'M')
    {
        throw bad_operation("not a valid bmp file to read.");
    }
    fread(&inf, sizeof(inf), 1, file);
    if (inf.bit_count != 4)
    {
        throw bad_format(fileName, ftell(file), "bit_count of bmp must be 4 (16 colours bitmap).");
    }
    width = inf.width;
    height = inf.height;
    bit_count = inf.bit_count;

    if (inf.bit_count > 32) throw bad_format(fileName, ftell(file), "bit_count too large.");
    word* qrgb = new word[static_cast<i64_t>(1) << inf.bit_count];
    Palette pl(1 << inf.bit_count);
    fread(qrgb, sizeof(word), static_cast<size_t>(1) << inf.bit_count, file);
    for (int i = 0; i < 1 << inf.bit_count; ++i)
    {
        pl.SetPixel(i, Pixel(qrgb[i]));
    }
    delete[] qrgb;

    byte* data = new byte[height * ((inf.bit_count * width + 7) >> 3)];
    fseek(file, hdr.offset, SEEK_SET);
    fread(data, sizeof(byte), static_cast<size_t>(hdr.size) - hdr.offset, file);
    if (pixels != nullptr) delete[] pixels; // drop old data
    pixels = new Pixel[Size()];
    int line = height - 1, col = 0;
    for (int i = 0; i < hdr.size - hdr.offset; ++i)
    {
        if (inf.bit_count == 4) // TODO: add more support for bmp file
        {
            SetPixel(line, col++, pl.GetColor(data[i] >> 4)); // FIXME: caused out of range if col is odd
            SetPixel(line, col++, pl.GetColor(data[i] & 0xF));
            if (col >= width) col = 0, line--; // bmp stores from bottom to top
            if (line < 0) throw bad_format(fileName, ftell(file), "image out of declaired size.");
        }
    }
    delete[] data;

    fclose(file);
}

//void Bitmap::LoadFile(const Palette& subpl) // FIXME: a copy of LoadFile(), merge them into one!
//{
//    bmpheader hdr;
//    bmpinfo inf;
//    FILE* file = fopen(fileName.c_str(), "rb");
//
//    if (file == NULL)
//    {
//        throw bad_operation(std::string("can not open specified file: ") + fileName + ", " + strerror(errno));
//    }
//
//    fread(&hdr, sizeof(hdr), 1, file);
//    if (hdr.ident[0] != 'B' || hdr.ident[1] != 'M')
//    {
//        throw bad_operation("not a valid bmp file to read.");
//    }
//    fread(&inf, sizeof(inf), 1, file);
//    if (inf.bit_count != 4)
//    {
//        throw bad_format(fileName, ftell(file), "bit_count of bmp must be 4 (16 colours bitmap).");
//    }
//    width = inf.width;
//    height = inf.height;
//    bit_count = inf.bit_count;
//
//    if (inf.bit_count > 32) throw bad_format(fileName, ftell(file), "bit_count too large.");
//    word* qrgb = new word[static_cast<i64_t>(1) << inf.bit_count];
//
//    byte* data = new byte[height * ((inf.bit_count * width + 7) >> 3)];
//    fseek(file, hdr.offset, SEEK_SET);
//    fread(data, sizeof(byte), static_cast<size_t>(hdr.size) - hdr.offset, file);
//    if (pixels != nullptr) delete[] pixels; // drop old data
//    pixels = new Pixel[Size()];
//    int line = height - 1, col = 0;
//    for (int i = 0; i < hdr.size - hdr.offset; ++i)
//    {
//        if (inf.bit_count == 4) // TODO: add more support for bmp file
//        {
//            SetPixel(line, col++, subpl.GetColor(data[i] >> 4)); // FIXME: caused out of range if col is odd
//            SetPixel(line, col++, subpl.GetColor(data[i] & 0xF));
//            if (col >= width) col = 0, line--; // bmp stores from bottom to top
//            if (line < 0) throw bad_format(fileName, ftell(file), "image out of declaired size.");
//        }
//    }
//    delete[] data;
//
//    fclose(file);
//}

void Bitmap::MapColor(const Palette& c2i, const Palette& i2c)
{
    int sz = Size();
    for (int i = 0; i < sz; ++i)
    {
        SetPixel(i, i2c.GetColor(c2i.GetPaletteIndex(GetPixel(i))));
    }
}
