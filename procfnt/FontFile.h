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

//#define FONT_TEX_GRP_NUMBER (36) // for ukn_645 // ooops, it's also 0x36, damm
#define FONT_TEX_GRP_NUMBER (0x36) // for ukn_060

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
	} texset[FONT_TEX_GRP_NUMBER];
};
#pragma pack(pop)

class FontFile
{
	void Init();
protected:
	std::string file_path;
	font_header_t header;
	std::vector<FontTexture*>* textures[FONT_TEX_GRP_NUMBER];
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

	void DropTexture(int group, word codePoint);
	void DropGroup(int group);
	void AddTexture(int group, FontTexture* texture);
	void GimmGroup(int group, std::vector<FontTexture*>* texs);

	void SetFilePath(const std::string& path);

	Palette& GetPalette();

	void SubPalette(const Palette& palette);
};

