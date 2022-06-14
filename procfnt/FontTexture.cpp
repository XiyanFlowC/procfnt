#include "FontTexture.h"

FontTexture::FontTexture()
{
	memset(&info, 0, sizeof(font_texture_t));
	info.drw_h = 0x10;
	info.drw_w = 0x10;
	info.ukn = 0x10;
}

//FontTexture::FontTexture(Graphic& tex)
//{
//	memset(&info, 0, sizeof(font_texture_t));
//	info.drw_h = 0x10;
//	info.drw_w = 0x10;
//	info.ukn = 0x10;
//	UpdateTexture(tex);
//}

FontTexture::FontTexture(const Graphic& tex, const Palette& pl)
{
	memset(&info, 0, sizeof(font_texture_t));
	info.drw_h = 0x10;
	info.drw_w = 0x10;
	info.ukn = 0x10;
	UpdateTexture(tex, pl);
}

FontTexture::FontTexture(font_texture_t info, byte* compressedData, int bpp)
{
	this->info = info;
	unsigned long bufsz = bpp * (info.canv_w << 1) * (info.canv_h << 1);
	bufsz = (bufsz + 7) >> 3;
	byte* buffer = new byte[bufsz];
	int ret = uncompress(buffer, &bufsz, compressedData, info.size);
	if (ret != Z_OK)
	{
		data = nullptr;
		if (ret == Z_DATA_ERROR) throw bad_parameter("compressedData", "zlib error, invalid format.");
		else throw bad_operation("zlib error, error code: " + std::to_string(ret));
	}
	else
	{
		//if (this->data != nullptr) delete[] data;
		size = bufsz;
		data = new byte[bufsz];
		memcpy(this->data, buffer, bufsz);
	}
	delete[] buffer;
}

const font_texture_t& FontTexture::GetInfo() const
{
	return info;
}

byte* FontTexture::GetCompressedData(int& length) const
{
	unsigned long destLeng = size * 2;
	byte* buffer = new byte[destLeng];
	compress(buffer, &destLeng, data, size);
	// length = (destLeng + 0xF) & ~0xF;
	length = destLeng;
	return buffer;
}

void FontTexture::UpdateTexture(const Graphic& g, const Palette& p) // UNDONE: only handled the 4bpp situ.
{
	if (p.Size() != 16) throw bad_parameter("p", "palette must be 16 colours.");

	if (data != nullptr) delete[] data;
	size = g.Size() / 2;
	data = new byte[size];
	for (int i = 0; i < size; ++i)
	{
		byte tmp = 0;
		tmp |= p.GetPaletteIndex(g.GetPixel(i * 2)) << 4;
		tmp |= p.GetPaletteIndex(g.GetPixel(i * 2 + 1)) & 0xF;
		data[i] = tmp;
	}

	info.tex_w = g.Width();
	info.canv_h = g.Height() >> 1;
	info.canv_w = g.Width() >> 1;
}

//void FontTexture::UpdateTexture(const Graphic& g)
//{
//	Palette pl;
//	pl.ExtractPalette(g, 16);
//	UpdateTexture(g, pl);
//}

char toASCIIArt(int num)
{
	return " 1OEM%$#~~~~~~~~"[num];
}

void FontTexture::GetTexture(Graphic& g, Palette& p) const
{
	std::vector<Pixel> tmp;
	for (int i = 0; i < size; ++i) // FIXME: assumed all texture is 4bpp, enhance it!
	{
		tmp.push_back(p.GetColor(data[i] >> 4));
		tmp.push_back(p.GetColor(data[i] & 0xf));
	}
	g.SetPixels(info.canv_w << 1, info.canv_h << 1, tmp.data());
}

word FontTexture::GetCodePoint() const
{
	return info.codepoint;
}

void FontTexture::SetCodePoint(word codepoint)
{
	info.codepoint = codepoint;
}

std::string FontTexture::GetConsoleDemo() const // for 4bpp preview only, comment this out once others prepared!
{
	std::string ret;
	for (int i = 0; i < info.canv_h << 1; ++i)
	{
		for (int j = 0; j < info.canv_w; ++j)
		{
			ret += toASCIIArt((data[i * (info.canv_w) + j] & 0xF0) >> 4);
			ret += ' ';
			ret += toASCIIArt(data[i * (info.canv_w) + j] & 0xf);
			ret += ' ';
		}
		ret += '\n';
	}
	return ret;
}

FontTexture::~FontTexture()
{
	if (data != nullptr)
	{
		delete[] data;
	}
}
