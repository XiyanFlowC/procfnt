#pragma once
#include <string>
#include <vector>
#include <cstdio>
#include <iostream>
#include "fixedtype.h"
#include "FontTexture.h"
#include "Palette.h"
#include "bad_operation.h"
#include "bad_format.h"

#pragma pack(push,1)
struct font_header_t {
	half size;
	byte ukn1; // 0x10
	byte ukn2; // 0x10
	word ukn3; // 0x10
	word palette_offset;
	word palette_size; // 0x10 ?
	struct {
		word count;
		word offset;
	} texset[36];
};
#pragma pack(pop)

class FontFile
{
	void Init();
protected:
	std::string file_path;
	font_header_t header;
	std::vector<FontTexture*>* textures[36];
	Palette palette;
public:
	/// <summary>
	/// 创建新的字体文件对象
	/// </summary>
	FontFile();

	/// <summary>
	/// 创建新的字体文件对象，从指定路径读取并初始化
	/// </summary>
	/// <param name="path">字体文件路径</param>
	FontFile(std::string path);
	
	~FontFile();

	void SaveFile();

	void LoadFile();

	const std::vector<FontTexture*> GetTextureGroup(int group);

	void AddTexture(int group, word codePoint, FontTexture* texture);

	Palette& GetPalette();

	void SubPalette(const Palette& palette);
};

