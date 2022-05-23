#include "Pallet.h"

Pallet::Pallet() : Graphic()
{
}

Pallet::Pallet(Pallet& m)
{
    width = m.width;
    height = m.height;

    pixels = new Pixel[Size()];
    memcpy(pixels, m.pixels, sizeof(Pixel) * Size());
}

Pallet::Pallet(int w) : Graphic(w, 1)
{
}

Pallet::Pallet(std::string path) : Graphic(path)
{
}

bool Pallet::IsInPallet(Pixel& color)
{
    return GetPalletIndex(color) != -1;
}

int Pallet::GetPalletIndex(Pixel& color)
{
    for (int i = 0; i < width * height; ++i)
    {
        if (pixels[i] == color) return i;
    }
    return -1;
}

Pixel Pallet::GetColor(int index)
{
    return GetPixel(index);
}

void Pallet::ExtractPallet(Graphic& g, int size)
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

void Pallet::SaveFile()
{
    struct {
        byte ident[4];
        word size;
    } header{{'P', 'L', '0', '0'}, Size()};
    FILE* f = fopen((fileName + ".plt").c_str(), "wb");
    fwrite(&header, sizeof(header), 1, f);
    fwrite(pixels, sizeof(Pixel), Size(), f);
    fclose(f);
}

void Pallet::LoadFile()
{
}

Pallet& Pallet::operator=(const Pallet& b)
{
    Clean();
    width = b.width;
    height = b.height;
    
    pixels = new Pixel[Size()];
    memcpy(pixels, b.pixels, sizeof(Pixel) * Size());

    return *this;
}
