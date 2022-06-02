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
	Graphic(Graphic& g);
	Graphic(std::string fileName);
	Graphic(int w, int h);
	Graphic(int w, int h, const Pixel* pixels);

	virtual void LoadFile();
	virtual void SaveFile();
	void LoadFile(std::string fileName);

	void SetFilePath(std::string filename);

	Pixel GetPixel(int index) const;
	Pixel GetPixel(int u, int v) const;
	void SetPixel(int index, Pixel data);
	void SetPixel(int u, int v, Pixel data);
	void SetPixels(int w, int h, const Pixel* pixels);
	const Pixel* const GetPixels(int& o_w, int& o_h) const;

	void Paste(int u, int v, Graphic& g);
	Graphic* Extract(int u, int v, int w, int h) const;
	void Resize(int new_w, int new_h);

	int Height() const;
	int Width() const;

	int Size() const;
	void Clean();

	~Graphic();
};

