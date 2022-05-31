#include "FontFile.h"

FontFile::FontFile()
{
	Init();
}

void FontFile::Init()
{
	memset(&header, 0, sizeof(header));
	header.ukn1 = 0x10; // HACK: for all templates I recieved, these fields keeps 0x10(16), I'v no idea what they are
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
			delete* itr; // delete contents
		}
		delete textures[i]; // delete container
	}
}

void FontFile::SaveFile()
{
	header.palette_size = palette.Size();
	header.size = 0;
	for (int i = 0; i < 36; ++i)
	{
		header.size += textures[i]->size();
	}
	FILE* f = fopen(file_path.c_str(), "wb");

	// fwrite(&header, sizeof(header), 1, f);
	// size_t loc = ftell(f);
	size_t loc = sizeof(font_header_t);
	// size_t offset = 0;
	for (int i = 0; i < 36; ++i)
	{
		header.texset[i].offset = loc;
		header.texset[i].count = textures[i]->size();
		loc += textures[i]->size() * sizeof(font_texture_t);
	}

	fwrite(&header, sizeof(header), 1, f);
	size_t head_pos = ftell(f);

	for (int i = 0; i < 36; ++i)
	{
		for (auto itr = textures[i]->begin(); itr != textures[i]->end(); ++itr)
		{
			fseek(f, head_pos, SEEK_SET);
			font_texture_t inf = (*itr)->GetInfo();
			int leng;
			byte* data = (*itr)->GetCompressedData(leng);
			inf.offset = loc;
			inf.size = leng;
			fwrite(&inf, sizeof(font_texture_t), 1, f);
			head_pos += sizeof(font_texture_t);

			fseek(f, loc, SEEK_SET);
			fwrite(data, sizeof(byte), leng, f);
			loc += leng;
			delete[] data;
		}
	}

	int siz = palette.Size();
	word* pix = new word[siz];
	for (int i = 0; i < siz; ++i)
	{
		pix[i] = palette.GetColor(i).PackWord();
	}
	header.palette_offset = loc;
	fwrite(pix, sizeof(word), palette.Size(), f);
	fseek(f, 0, SEEK_SET);
	fwrite(&header, sizeof(header), 1, f);

	fclose(f);
}

static int bpp_calc(int palette_num) {
	int size = -1;
	while (palette_num) {
		palette_num >>= 1;
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
				tex = new FontTexture(texdef[j], data, bpp_calc(header.palette_size));
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

	fseek(file, header.palette_offset, SEEK_SET);
	word* pal = new word[header.palette_size];
	fread(pal, sizeof(word), header.palette_size, file);
	Pixel* pix = new Pixel[header.palette_size];
	for (int i = 0; i < header.palette_size; ++i)
	{
		pix[i].UnpackWord(pal[i]);
	}
	palette.SetPixels(header.palette_size, 1, pix);
	delete[] pix;
	delete[] pal;
}

const std::vector<FontTexture*> FontFile::GetTextureGroup(int group)
{
	return *textures[group];
}

void FontFile::DropGroup(int group)
{
	for (auto itr = textures[group]->begin(); itr != textures[group]->end(); ++itr)
	{
		delete* itr;
	}
	textures[group]->clear();
}

void FontFile::AddTexture(int group, FontTexture* texture)
{
	textures[group]->push_back(texture);
}

void FontFile::GimmGroup(int group, std::vector<FontTexture*>* texs)
{
	if (nullptr != textures[group]) DropGroup(group);
	textures[group] = texs;
}

void FontFile::SetFilePath(const std::string& path)
{
	file_path = path;
}

Palette& FontFile::GetPalette()
{
	return palette;
}

void FontFile::SubPalette(const Palette& palette)
{
	this->palette = palette;
}
