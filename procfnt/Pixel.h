#pragma once

#include "fixedtype.h"

struct Pixel
{
	short red;
	short green;
	short blue;
	short alpha;

	Pixel(word rgba);
	Pixel(short p_red, short p_green, short p_blue, short p_alpha);
	Pixel();

	word PackWord();
	void UnpackWord(word rgba);

	bool operator== (const Pixel& b);
	Pixel& operator= (const Pixel& b);
	friend bool operator< (const Pixel& a, const Pixel& b);
};

