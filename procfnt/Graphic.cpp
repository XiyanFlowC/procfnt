#include "Graphic.h"
#include "bad_operation.h"

Graphic::Graphic()
{
	pixels = nullptr;
	width = height = 0;
}

Graphic::Graphic(Graphic& g)
{
	fileName = g.fileName;
	width = g.width;
	height = g.height;
	pixels = new Pixel[Size()];
	memcpy(pixels, g.pixels, sizeof(Pixel) * Size());
}

Graphic::Graphic(std::string fileName)
{
	pixels = nullptr;
	width = height = 0;
	this->fileName = fileName;
	LoadFile(fileName);
}

Graphic::Graphic(int w, int h)
{
	width = w;
	height = h;
	pixels = new Pixel[Size()];
}

Graphic::Graphic(int w, int h, const Pixel* pixels)
{
	pixels = nullptr;
	SetPixels(w, h, pixels);
}

void Graphic::LoadFile()
{
	throw bad_operation("in memory graphic can not load!");
}

void Graphic::SaveFile()
{
	throw bad_operation("in memory graphic can not save!");
}

void Graphic::LoadFile(std::string fileName)
{
	this->fileName = fileName;
	LoadFile();
}

void Graphic::SetFilePath(std::string filename)
{
	fileName = filename;
}

Pixel Graphic::GetPixel(int index) const
{
	if (pixels == nullptr)
	{
		throw bad_operation("member pixels is not initialized.");
	}
	if (index >= Size()) throw bad_parameter("index", "index out of range.");
	return pixels[index];
}

Pixel Graphic::GetPixel(int u, int v) const
{
	return GetPixel(u * width + v);
}

void Graphic::SetPixel(int index, Pixel data)
{
	if (pixels == nullptr)
	{
		throw bad_operation("pixels is not initialized.");
	}
	if (index >= Size()) throw bad_parameter("index", "index out of range");
	pixels[index] = data;
}

void Graphic::SetPixel(int u, int v, Pixel data)
{
	SetPixel(u * width + v, data);
}

void Graphic::SetPixels(int w, int h, const Pixel* pixels)
{
	if (this->pixels != nullptr) delete[] this->pixels;
	width = w;
	height = h;
	this->pixels = new Pixel[sizeof(Pixel) * w * h];
	memcpy(this->pixels, pixels, sizeof(Pixel) * w * h);
}

const Pixel* const Graphic::GetPixels(int& o_w, int& o_h) const
{
	o_w = width;
	o_h = height;
	return pixels;
}

void Graphic::Paste(int u, int v, Graphic& g)
{
	for (int i = 0; i < g.height; ++i)
		for (int j = 0; j < g.width; ++j)
			SetPixel(u + i, v + j, g.GetPixel(i, j));
}

Graphic* Graphic::Extract(int u, int v, int w, int h) const
{
	Graphic* ret = new Graphic(u, v);
	for (int i = 0; i < h; ++i)
		for (int j = 0; j < w; ++j)
			ret->SetPixel(i, j, GetPixel(u + i, v + j));

	return ret;
}

int Graphic::Height() const
{
	return height;
}

int Graphic::Width() const
{
	return width;
}

int Graphic::Size() const
{
	return width * height;
}

void Graphic::Clean()
{
	if (pixels != nullptr)
	{
		delete[] pixels;
		pixels = nullptr;
	}
	width = height = 0;
}

Graphic::~Graphic()
{
	if (pixels != nullptr)
	{
		delete[] pixels;
	}
}
