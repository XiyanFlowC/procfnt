#include "Palette.h"

Palette::Palette() : Graphic(0, 1)
{
}

Palette::Palette(Palette& m)
{
    width = m.width;
    height = m.height;

    pixels = new Pixel[Size()];
    memcpy(pixels, m.pixels, sizeof(Pixel) * Size());
}

Palette::Palette(int w) : Graphic(w, 1)
{
}

Palette::Palette(std::string path) : Graphic(path)
{
    LoadFile();
}

bool Palette::IsInPalette(const Pixel& color) const
{
    return GetPaletteIndex(color) != -1;
}

int Palette::GetPaletteIndex(const Pixel& color) const
{
    for (int i = 0; i < width * height; ++i)
    {
        if (pixels[i] == color) return i;
    }
    return -1;
}

Pixel Palette::GetColor(int index) const
{
    return GetPixel(index);
}

void Palette::ExtractPalette(const Graphic& g, int size)
{
    std::set<Pixel> colours;
    for (int i = 0; i < g.Size(); ++i)
    {
        Pixel px = g.GetPixel(i);
        colours.insert(px);
    }
    Clean();
    if (size < colours.size())
        throw bad_parameter("size", "size is too small for this graphic!");
    pixels = new Pixel[size];
    width = size;
    height = 1;
    int c = 0;
    for (auto itr = colours.begin(); itr != colours.end(); ++itr)
    {
        pixels[c++] = *itr;
    }
    for (; c < size; ++c)
    {
        pixels[c++] = Pixel(0);
    }
}

void Palette::SaveFile()
{
    struct {
        byte ident[4];
        word size;
    } header{{'P', 'L', '0', '0'}, Size()};
    FILE* f = fopen((fileName).c_str(), "wb");
    fwrite(&header, sizeof(header), 1, f);
    fwrite(pixels, sizeof(Pixel), Size(), f);
    fclose(f);
}

void Palette::LoadFile()
{
    struct {
        byte ident[4];
        word size;
    } hdr;
    FILE* f = fopen((fileName).c_str(), "rb");
    if (f == NULL) {
        throw bad_operation(std::string("Open specified file [") + fileName + "] failed, " + strerror(errno));
    }
    fread(&hdr, sizeof(hdr), 1, f);
    if (pixels != nullptr) delete[] pixels;
    width = hdr.size;
    height = 1;
    pixels = new Pixel[hdr.size];
    fread(pixels, sizeof(Pixel), hdr.size, f);
    fclose(f);
}

Palette& Palette::operator=(const Palette& b)
{
    Clean();
    width = b.width;
    height = b.height;
    
    pixels = new Pixel[Size()];
    memcpy(pixels, b.pixels, sizeof(Pixel) * Size());

    return *this;
}
