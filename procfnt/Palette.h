#pragma once
#include "Graphic.h"
#include <set>
#include <stdio.h>

class Palette : public Graphic
{
public:
	Palette();
	Palette(Palette& m);
	Palette(int w);
	Palette(std::string path);

	//void AddColor32(word data);
	bool IsInPalette(const Pixel& color) const;
	int GetPaletteIndex(const Pixel& color) const;
	Pixel GetColor(int index) const;
	void ExtractPalette(Graphic& g, int size);
	void SaveFile();
	void LoadFile();

	Palette& operator= (const Palette& b);
};

