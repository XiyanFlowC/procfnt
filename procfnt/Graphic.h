#pragma once

#include "fixedtype.h"
#include <string>
#include <vector>
#include "Pixel.h"
#include "bad_parameter.h"

class Graphic
{
protected:
	std::string fileName;
	int height, width;
	Pixel* pixels;

public:
	Graphic();
	Graphic(std::string fileName);
	Graphic(int w, int h);
	Graphic(int w, int h, const Pixel* pixels);

	virtual void LoadFile() = 0;
	virtual void SaveFile() = 0;
	void LoadFile(std::string fileName);

	void SetFilePath(std::string filename);

	Pixel GetPixel(int index);
	Pixel GetPixel(int u, int v);
	void SetPixel(int index, Pixel data);
	void SetPixel(int u, int v, Pixel data);
	void SetPixels(int w, int h, const Pixel* pixels);
	const Pixel* const GetPixels(int& o_w, int& o_h);

	void Paste(int u, int v, Graphic& g);
	Graphic* Extract(int u, int v, int h, int w);

	int Size();
	void Clean();

	~Graphic();
};

