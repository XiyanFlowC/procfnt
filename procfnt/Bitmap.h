#pragma once
#include <cassert>
#include <cerrno>
#include <cstring>
#include "Graphic.h"
#include "Pallet.h"
#include "bad_parameter.h"
#include "bad_operation.h"

#pragma pack(push,1)
struct bmpheader {
    byte ident[2];
    word size;
    half reserve1;
    half reserve2;
    word offset;
};

struct bmpinfo {
    word size;
    word width;
    word height;
    half plane;
    half bit_count;
    word compression;
    word size_img;
    word xppm;
    word yppm;
    word clr_used;
    word clr_imp;
};
#pragma pack(pop)

class Bitmap :
    public Graphic
{
protected:
    int bit_count;
public:
    Bitmap();
    Bitmap(std::string path);
    Bitmap(int w, int h);

    int GetBitCount();
    void SetBitCount(int value);
    void SaveFile();
    void LoadFile();
};

