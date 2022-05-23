#include "Pixel.h"

Pixel::Pixel(word rgba)
{
	UnpackWord(rgba);
}

Pixel::Pixel(short p_red, short p_green, short p_blue, short p_alpha)
{
	red = p_red;
	green = p_green;
	blue = p_blue;
	alpha = p_alpha;
}

Pixel::Pixel()
{
	red = green = blue = alpha = 0;
}

word Pixel::PackWord()
{
	return (red & 0xff) | (green & 0xff) << 8 | (blue & 0xff) << 16 | (alpha & 0xff) << 24;
}

void Pixel::UnpackWord(word rgba)
{
	red = (rgba & 0xff);
	green = (rgba & 0xff00) >> 8;
	blue = (rgba & 0xff0000) >> 16;
	alpha = (rgba & 0xff000000) >> 24;
}

Pixel& Pixel::operator=(const Pixel& b)
{
	red = b.red;
	green = b.green;
	blue = b.blue;
	alpha = b.alpha;
	return *this;
}

bool Pixel::operator==(const Pixel& b)
{
	return red == b.red && green == b.green && blue == b.blue && alpha == b.alpha;
}

bool operator< (const Pixel& a, const Pixel& b)
{
	if (a.red == b.red)
	{
		if (a.green == b.green)
		{
			if (a.blue == b.blue)
			{
				return a.alpha < b.alpha;
			}
			return a.blue < b.blue;
		}
		return a.green < b.green;
	}
	return a.red < b.red;
}
