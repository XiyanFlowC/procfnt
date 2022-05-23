#include "FontFile.h"

FontFile::FontFile()
{
	Init();
}

void FontFile::Init()
{
	memset(&header, 0, sizeof(header));
	header.ukn1 = 0x10;
	header.ukn2 = 0x10;
	header.ukn3 = 0x10;

	for (int i = 0; i < 36; ++i)
	{
		textures[i] = new std::vector<FontTexture*>();
	}
}

FontFile::FontFile(std::string path)
{
	Init();
	file_path = path;
	LoadFile();
}

FontFile::~FontFile()
{
	for (int i = 0; i < 36; ++i)
	{
		for (std::vector<FontTexture*>::iterator itr = textures[i]->begin(); itr != textures[i]->end(); ++itr)
		{
			delete* itr;
		}
		delete textures[i];
	}
}

static int bpp_calc(int pallet_num) {
	int size = -1;
	while (pallet_num) {
		pallet_num >>= 1;
		size++;
	}
	return size;
}

void FontFile::LoadFile()
{
	FILE* file = fopen(file_path.c_str(), "rb");
	if (file == NULL)
	{
		throw bad_operation("can't open specified file.");
	}

	fread(&header, sizeof(header), 1, file);
	int remain = header.size;
	for (int i = 0; i < 36; ++i)
	{
		if (header.texset[i].count == 0) continue;
		remain -= header.texset[i].count;
		
		fseek(file, header.texset[i].offset, SEEK_SET);
		font_texture_t* texdef = new font_texture_t[header.texset[i].count];
		if(header.texset[i].count != fread(texdef, sizeof(font_texture_t), header.texset[i].count, file)) throw bad_format(file_path, ftell(file), "invalid offset or brocken file.");
		for (unsigned int j = 0; j < header.texset[i].count; ++j)
		{
			fseek(file, texdef[j].offset, SEEK_SET);
			byte* data = new byte[texdef[j].size];
			if (!fread(data, texdef[j].size, 1, file)) throw bad_format(file_path, ftell(file), "invalid offset or brocken file.");

			FontTexture* tex = nullptr;
			try
			{
				tex = new FontTexture(texdef[j], data, bpp_calc(header.pallet_size));
			}
			catch (bad_parameter& e)
			{
				std::cerr << "format failure on group " << i << ", texture " << j << std::endl << e.to_string() << std::endl;
			}
			catch (bad_operation& e)
			{
				std::cerr << "unknown error, on group " << i << ", texture " << j << std::endl << e.to_string() << std::endl;
			}
			textures[i]->push_back(tex);
			delete[] data;
		}
		delete[] texdef;
	}

	fseek(file, header.pallet_offset, SEEK_SET);
	word* pal = new word[header.pallet_size];
	fread(pal, sizeof(word), header.pallet_size, file);
	Pixel* pix = new Pixel[header.pallet_size];
	for (int i = 0; i < header.pallet_size; ++i)
	{
		pix[i].UnpackWord(pal[i]);
	}
	pallet.SetPixels(header.pallet_size, 1, pix);
	delete[] pix;
	delete[] pal;
}

const std::vector<FontTexture*> FontFile::GetTextureGroup(int group)
{
	return *textures[group];
}

Pallet& FontFile::GetPallet()
{
	return pallet;
}

void FontFile::SubPallet(const Pallet& pallet)
{
	this->pallet = pallet;
}
