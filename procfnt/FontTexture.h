#pragma once
#ifndef FONT_TEXTURE_H
#define FONT_TEXTURE_H
#include <zlib.h>
#include <string>
#include "fixedtype.h"
#include "bad_parameter.h"
#include "bad_operation.h"
#include "Graphic.h"
#include "Pallet.h"

#pragma pack(push,1)
struct font_texture_t {
	word codepoint;
	half size;
	byte tex_w;
	byte drw_w; // 0x10
	byte canv_w; // := value >> 1
	byte drw_h; // 0x10
	byte canv_h; // := value >> 1
	byte ukn;   // 0x10
	word offset;
};
#pragma pack(pop)

class FontTexture
{
	struct font_texture_t info;
	byte* data;
	int size;
public:
	FontTexture(font_texture_t info, byte* compressedData, int bpp);

	const font_texture_t &GetInfo();

	byte* GetCompressedData(int& length);

	void UpdateTexture(const Graphic& g, const Pallet& p);

	void GetTexture(Graphic& g, Pallet& p);

	word GetCodePoint();

	void SetCodePoint(word codepoint);

	std::string GetConsoleDemo();

	~FontTexture();
};

#endif