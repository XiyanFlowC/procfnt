#pragma once
#include "Graphic.h"
#include <set>
#include <stdio.h>

class Pallet : public Graphic
{
public:
	Pallet();
	Pallet(Pallet& m);
	Pallet(int w);
	Pallet(std::string path);

	//void AddColor32(word data);
	bool IsInPallet(Pixel& color);
	int GetPalletIndex(Pixel& color);
	Pixel GetColor(int index);
	void ExtractPallet(Graphic& g, int size);
	void SaveFile();
	void LoadFile();

	Pallet& operator= (const Pallet& b);
};

