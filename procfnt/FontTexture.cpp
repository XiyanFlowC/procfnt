#include "FontTexture.h"

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

const font_texture_t& FontTexture::GetInfo()
{
	return info;
}

byte* FontTexture::GetCompressedData(int& length)
{
	unsigned long destLeng = size * 2;
	byte* buffer = new byte[length];
	compress(buffer, &destLeng, data, size);
	length = destLeng;
	return buffer;
}

char toASCIIArt(int num)
{
	return " 1OEM%$#~~~~~~~~"[num];
}

void FontTexture::GetTexture(Graphic& g, Pallet& p)
{
	std::vector<Pixel> tmp;
	for (int i = 0; i < size; ++i) // FIXME: assumed all texture is 4bpp, enhance it!
	{
		tmp.push_back(p.GetColor(data[i] >> 4));
		tmp.push_back(p.GetColor(data[i] & 0xf));
	}
	g.SetPixels(info.canv_w << 1, info.canv_h << 1, tmp.data());
}

word FontTexture::GetCodePoint()
{
	return info.codepoint;
}

void FontTexture::SetCodePoint(word codepoint)
{
	info.codepoint = codepoint;
}

std::string FontTexture::GetConsoleDemo() // for 4bpp preview only, comment this out once others prepared!
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
